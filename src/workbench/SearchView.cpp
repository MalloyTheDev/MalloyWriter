#include "workbench/SearchView.hpp"

#include "base/Theme.hpp"
#include "workbench/Icon.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

namespace {

using Base::Theme;

QToolButton *makeToggle(const QString &iconName, const QString &tooltip, QWidget *parent)
{
    auto *button = new QToolButton(parent);
    button->setObjectName(QStringLiteral("iconBtn"));
    button->setCheckable(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setToolTip(tooltip);
    button->setIcon(Icon::icon(iconName, 12, Theme::active().color(QStringLiteral("text-soft"))));
    button->setIconSize(QSize(12, 12));
    button->setFixedSize(22, 22);
    return button;
}

constexpr int kPathRole = Qt::UserRole + 1;
constexpr int kLineRole = Qt::UserRole + 2;
constexpr int kColumnRole = Qt::UserRole + 3;

} // namespace

SearchView::SearchView(QWidget *parent)
    : QWidget(parent)
    , m_service(new Platform::SearchService(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 6, 6);
    layout->setSpacing(8);

    // Query row: input + match toggles.
    auto *queryRow = new QHBoxLayout;
    queryRow->setSpacing(4);
    m_query = new QLineEdit(this);
    m_query->setPlaceholderText(tr("Search"));
    m_query->setClearButtonEnabled(true);
    queryRow->addWidget(m_query, 1);
    m_caseButton = makeToggle(QStringLiteral("matchCase"), tr("Match Case"), this);
    m_wordButton = makeToggle(QStringLiteral("matchWord"), tr("Match Whole Word"), this);
    m_regexButton = makeToggle(QStringLiteral("matchRegex"), tr("Use Regular Expression"), this);
    queryRow->addWidget(m_caseButton);
    queryRow->addWidget(m_wordButton);
    queryRow->addWidget(m_regexButton);
    layout->addLayout(queryRow);

    m_summary = new QLabel(this);
    m_summary->setObjectName(QStringLiteral("placeholderText"));
    layout->addWidget(m_summary);

    m_results = new QTreeWidget(this);
    m_results->setHeaderHidden(true);
    m_results->setIndentation(12);
    m_results->setUniformRowHeights(true);
    layout->addWidget(m_results, 1);

    connect(m_query, &QLineEdit::returnPressed, this, &SearchView::runSearch);
    for (QToolButton *toggle : {m_caseButton, m_wordButton, m_regexButton}) {
        connect(toggle, &QToolButton::toggled, this, &SearchView::runSearch);
    }
    connect(m_service, &Platform::SearchService::resultsReady, this, &SearchView::showResults);
    connect(m_results, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem *item) {
        const QString path = item->data(0, kPathRole).toString();
        if (!path.isEmpty()) {
            emit openRequested(path, item->data(0, kLineRole).toInt(), item->data(0, kColumnRole).toInt());
        }
    });
}

void SearchView::setWorkspaceRoot(const QString &root)
{
    m_root = root;
}

void SearchView::runSearch()
{
    const QString text = m_query->text();
    m_results->clear();
    if (text.isEmpty() || m_root.isEmpty()) {
        m_summary->setText(text.isEmpty() ? QString() : tr("Open a folder to search."));
        return;
    }
    m_summary->setText(tr("Searching…"));
    Platform::SearchQuery query;
    query.text = text;
    query.caseSensitive = m_caseButton->isChecked();
    query.wholeWord = m_wordButton->isChecked();
    query.regex = m_regexButton->isChecked();
    m_service->search(m_root, query);
}

void SearchView::showResults(const QList<Platform::FileMatches> &results, int fileCount, int hitCount)
{
    m_results->clear();
    m_summary->setText(tr("%1 results in %2 files").arg(hitCount).arg(fileCount));

    const Theme &theme = Theme::active();
    const QColor pathColor = theme.color(QStringLiteral("muted"));

    for (const Platform::FileMatches &file : results) {
        const FileType type = Icon::fileType(file.path);
        auto *fileItem = new QTreeWidgetItem(m_results);
        fileItem->setIcon(0, Icon::icon(type.iconName, 14, type.color));
        const int slash = file.relativePath.lastIndexOf(QLatin1Char('/'));
        const QString name = slash >= 0 ? file.relativePath.mid(slash + 1) : file.relativePath;
        const QString dir = slash >= 0 ? file.relativePath.left(slash) : QString();
        fileItem->setText(0, dir.isEmpty() ? QStringLiteral("%1  (%2)").arg(name).arg(file.hits.size())
                                           : QStringLiteral("%1  %2  (%3)").arg(name, dir).arg(file.hits.size()));
        fileItem->setExpanded(true);

        for (const Platform::SearchHit &hit : file.hits) {
            auto *hitItem = new QTreeWidgetItem(fileItem);
            hitItem->setText(0, QStringLiteral("%1: %2").arg(hit.line).arg(hit.lineText.trimmed()));
            hitItem->setForeground(0, pathColor);
            hitItem->setData(0, kPathRole, file.path);
            hitItem->setData(0, kLineRole, hit.line);
            hitItem->setData(0, kColumnRole, hit.column);
        }
    }
}

} // namespace MalloyWriter::Workbench
