#include "workbench/BottomPanel.hpp"

#include "base/Theme.hpp"
#include "workbench/Icon.hpp"
#include "workbench/OutputPanel.hpp"

#include <QButtonGroup>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

namespace {

using Base::Theme;

constexpr int kPathRole = Qt::UserRole + 1;
constexpr int kLineRole = Qt::UserRole + 2;
constexpr int kColumnRole = Qt::UserRole + 3;

QString severityIcon(Platform::DiagnosticSeverity severity)
{
    switch (severity) {
    case Platform::DiagnosticSeverity::Error:   return QStringLiteral("err");
    case Platform::DiagnosticSeverity::Warning: return QStringLiteral("warn");
    case Platform::DiagnosticSeverity::Note:    break;
    }
    return QStringLiteral("info");
}

QColor severityColor(Platform::DiagnosticSeverity severity)
{
    switch (severity) {
    case Platform::DiagnosticSeverity::Error:   return Theme::active().color(QStringLiteral("err"));
    case Platform::DiagnosticSeverity::Warning: return Theme::active().color(QStringLiteral("warn"));
    case Platform::DiagnosticSeverity::Note:    break;
    }
    return Theme::active().color(QStringLiteral("info"));
}

} // namespace

// ───────────────────────── Problems ─────────────────────────
ProblemsView::ProblemsView(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_tree = new QTreeWidget(this);
    m_tree->setHeaderHidden(true);
    m_tree->setIndentation(14);
    m_tree->setUniformRowHeights(true);
    layout->addWidget(m_tree);

    connect(m_tree, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem *item) {
        const QString path = item->data(0, kPathRole).toString();
        if (!path.isEmpty()) {
            emit activated(path, item->data(0, kLineRole).toInt(), item->data(0, kColumnRole).toInt());
        }
    });
}

void ProblemsView::setDiagnostics(const QList<Platform::Diagnostic> &diagnostics)
{
    m_tree->clear();

    // Group by file, preserving first-seen order.
    QList<QString> order;
    QHash<QString, QList<Platform::Diagnostic>> byFile;
    for (const Platform::Diagnostic &diagnostic : diagnostics) {
        if (!byFile.contains(diagnostic.filePath)) {
            order.append(diagnostic.filePath);
        }
        byFile[diagnostic.filePath].append(diagnostic);
    }

    const QColor messageColor = Theme::active().color(QStringLiteral("text-soft"));
    const QColor dimColor = Theme::active().color(QStringLiteral("muted"));

    for (const QString &filePath : order) {
        const QList<Platform::Diagnostic> &items = byFile.value(filePath);
        const FileType type = Icon::fileType(filePath);
        const int slash = qMax(filePath.lastIndexOf(QLatin1Char('/')), filePath.lastIndexOf(QLatin1Char('\\')));
        const QString name = slash >= 0 ? filePath.mid(slash + 1) : filePath;

        auto *fileItem = new QTreeWidgetItem(m_tree);
        fileItem->setIcon(0, Icon::icon(type.iconName, 14, type.color));
        fileItem->setText(0, QStringLiteral("%1  (%2)").arg(name).arg(items.size()));
        fileItem->setExpanded(true);

        for (const Platform::Diagnostic &diagnostic : items) {
            auto *row = new QTreeWidgetItem(fileItem);
            row->setIcon(0, Icon::icon(severityIcon(diagnostic.severity), 13, severityColor(diagnostic.severity)));
            const QString source = diagnostic.source.isEmpty() ? QString() : QStringLiteral(" [%1]").arg(diagnostic.source);
            row->setText(0, QStringLiteral("%1%2   [Ln %3, Col %4]")
                                .arg(diagnostic.message, source)
                                .arg(diagnostic.line).arg(diagnostic.column));
            row->setForeground(0, messageColor);
            row->setData(0, kPathRole, diagnostic.filePath);
            row->setData(0, kLineRole, diagnostic.line);
            row->setData(0, kColumnRole, diagnostic.column);
        }
    }
    if (order.isEmpty()) {
        auto *empty = new QTreeWidgetItem(m_tree);
        empty->setText(0, tr("No problems have been detected."));
        empty->setForeground(0, dimColor);
        empty->setFlags(Qt::ItemIsEnabled);
    }
}

// ───────────────────────── Terminal ─────────────────────────
TerminalView::TerminalView(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_output = new QPlainTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setFrameShape(QFrame::NoFrame);
    QFont mono;
    mono.setFamilies({"Geist Mono", "Cascadia Mono", "Consolas"});
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(10);
    m_output->setFont(mono);
    layout->addWidget(m_output, 1);

    m_input = new QLineEdit(this);
    m_input->setFont(mono);
    m_input->setPlaceholderText(tr("Run a command…"));
    layout->addWidget(m_input);

    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::readyRead, this, [this]() {
        m_output->appendPlainText(QString::fromLocal8Bit(m_process->readAll()).trimmed());
    });
    connect(m_process, &QProcess::finished, this, [this](int exitCode) {
        m_output->appendPlainText(tr("[exit %1]\n").arg(exitCode));
        m_input->setEnabled(true);
        m_input->setFocus();
    });
    connect(m_input, &QLineEdit::returnPressed, this, [this]() {
        const QString command = m_input->text().trimmed();
        m_input->clear();
        if (!command.isEmpty()) {
            runCommand(command);
        }
    });
}

void TerminalView::setWorkingDirectory(const QString &dir)
{
    m_cwd = dir;
}

void TerminalView::runCommand(const QString &command)
{
    if (m_process->state() != QProcess::NotRunning) {
        return;
    }
    m_output->appendPlainText(QStringLiteral("> %1").arg(command));
    if (!m_cwd.isEmpty()) {
        m_process->setWorkingDirectory(m_cwd);
    }
    m_input->setEnabled(false);
#ifdef Q_OS_WIN
    m_process->start(QStringLiteral("cmd.exe"), {QStringLiteral("/c"), command});
#else
    m_process->start(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), command});
#endif
    if (!m_process->waitForStarted(2000)) {
        m_output->appendPlainText(tr("[failed to start command]\n"));
        m_input->setEnabled(true);
    }
}

// ───────────────────────── Bottom panel ─────────────────────────
BottomPanel::BottomPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("bottomPanel"));
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *tabBar = new QWidget(this);
    tabBar->setObjectName(QStringLiteral("panelTabBar"));
    auto *tabLayout = new QHBoxLayout(tabBar);
    tabLayout->setContentsMargins(4, 0, 6, 0);
    tabLayout->setSpacing(0);

    m_tabs = new QButtonGroup(this);
    m_tabs->setExclusive(true);
    m_stack = new QStackedWidget(this);

    auto addTab = [&](const QString &label) {
        auto *button = new QPushButton(label, tabBar);
        button->setObjectName(QStringLiteral("panelTab"));
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        const int index = m_tabs->buttons().size();
        m_tabs->addButton(button, index);
        tabLayout->addWidget(button);
        return button;
    };

    m_problemsTab = addTab(tr("Problems"));
    addTab(tr("Output"));
    addTab(tr("Debug Console"));
    addTab(tr("Terminal"));
    addTab(tr("Ports"));
    tabLayout->addStretch(1);

    auto *closeButton = new QPushButton(tabBar);
    closeButton->setObjectName(QStringLiteral("iconBtn"));
    closeButton->setFlat(true);
    closeButton->setIcon(Icon::icon(QStringLiteral("x"), 12, Theme::active().color(QStringLiteral("text-soft"))));
    closeButton->setToolTip(tr("Close Panel"));
    closeButton->setCursor(Qt::PointingHandCursor);
    tabLayout->addWidget(closeButton);
    connect(closeButton, &QPushButton::clicked, this, &BottomPanel::closeRequested);

    layout->addWidget(tabBar);

    // Pages (order matches the tabs).
    m_problems = new ProblemsView(m_stack);
    m_output = new OutputPanel(m_stack);
    auto *debugConsole = new QWidget(m_stack);
    {
        auto *debugLayout = new QVBoxLayout(debugConsole);
        debugLayout->setContentsMargins(12, 10, 12, 10);
        auto *label = new QLabel(tr("Debug session not started. Press F5 to start, or pick a launch "
                                    "configuration in Run and Debug."), debugConsole);
        label->setObjectName(QStringLiteral("placeholderText"));
        label->setWordWrap(true);
        debugLayout->addWidget(label);
        debugLayout->addStretch(1);
    }
    m_terminal = new TerminalView(m_stack);
    auto *ports = new QWidget(m_stack);
    {
        auto *portsLayout = new QVBoxLayout(ports);
        portsLayout->setContentsMargins(12, 10, 12, 10);
        auto *label = new QLabel(tr("No forwarded ports. Run a server to detect ports automatically."), ports);
        label->setObjectName(QStringLiteral("placeholderText"));
        label->setWordWrap(true);
        portsLayout->addWidget(label);
        portsLayout->addStretch(1);
    }

    m_stack->addWidget(m_problems);
    m_stack->addWidget(m_output);
    m_stack->addWidget(debugConsole);
    m_stack->addWidget(m_terminal);
    m_stack->addWidget(ports);
    layout->addWidget(m_stack, 1);

    connect(m_tabs, &QButtonGroup::idClicked, m_stack, &QStackedWidget::setCurrentIndex);
    connect(m_problems, &ProblemsView::activated, this, &BottomPanel::problemActivated);

    m_problemsTab->setChecked(true);
    m_stack->setCurrentIndex(0);
}

void BottomPanel::setProblems(const QList<Platform::Diagnostic> &diagnostics)
{
    m_problems->setDiagnostics(diagnostics);
    m_problemsTab->setText(diagnostics.isEmpty() ? tr("Problems")
                                                 : tr("Problems (%1)").arg(diagnostics.size()));
}

void BottomPanel::setWorkspaceRoot(const QString &root)
{
    m_terminal->setWorkingDirectory(root);
}

void BottomPanel::showProblems()
{
    m_problemsTab->setChecked(true);
    m_stack->setCurrentIndex(0);
}

} // namespace MalloyWriter::Workbench
