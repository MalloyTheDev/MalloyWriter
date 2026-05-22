#include "workbench/MainWindow.hpp"

#include "base/AppInfo.hpp"
#include "base/PathUtils.hpp"
#include "platform/BuildSystem.hpp"
#include "base/Theme.hpp"
#include "editor/Document.hpp"
#include "languages/LspDiagnosticMapper.hpp"
#include "workbench/ActivityBar.hpp"
#include "workbench/CommandPalette.hpp"
#include "workbench/OpenEditorsList.hpp"
#include "workbench/OutputPanel.hpp"
#include "workbench/ProjectExplorer.hpp"
#include "workbench/Sidebar.hpp"
#include "workbench/StatusBar.hpp"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

namespace {

bool isCppPath(const QString &path)
{
    static const QStringList suffixes = {
        "cpp", "cc", "cxx", "c", "hpp", "hxx", "h", "inl", "ipp", "tpp",
    };
    return suffixes.contains(QFileInfo(path).suffix().toLower());
}

} // namespace

namespace MalloyWriter::Workbench {

namespace {

QStringList executableFilter()
{
#ifdef Q_OS_WIN
    return {"Executables (*.exe)", "All files (*.*)"};
#else
    return {"All files (*)"};
#endif
}

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_buildService(&m_projectService, this)
    , m_clangd(this)
{
    setupUi();
    registerCommands();
    buildMenus();

    connect(&m_settings, &Platform::SettingsService::recentFilesChanged, this, &MainWindow::updateRecentMenus);
    connect(&m_settings, &Platform::SettingsService::recentWorkspacesChanged, this, &MainWindow::updateRecentMenus);
    connect(&m_buildService, &Platform::BuildService::outputReceived, m_outputPanel, &OutputPanel::appendText);
    connect(&m_buildService, &Platform::BuildService::jobStarted, this, [this](const QString &label) {
        statusBar()->showMessage(tr("Running %1").arg(label));
    });
    connect(&m_buildService, &Platform::BuildService::diagnosticsChanged, this,
            [this](const QList<Platform::Diagnostic> &diagnostics) {
        m_buildDiagnostics = diagnostics;
        refreshDiagnostics();
    });
    connect(&m_buildService, &Platform::BuildService::jobFailedToStart, this, [this](const QString &label) {
        m_outputPanel->appendText(tr("Failed to start %1.\n").arg(label));
        statusBar()->showMessage(tr("Failed to start %1").arg(label), 5000);
    });
    connect(&m_buildService, &Platform::BuildService::jobFinished, this, [this](const QString &, int exitCode, QProcess::ExitStatus status) {
        const QString statusText = status == QProcess::NormalExit
            ? tr("Process finished with exit code %1").arg(exitCode)
            : tr("Process crashed");
        m_outputPanel->appendText(QString("\n%1\n").arg(statusText));
        statusBar()->showMessage(statusText, 6000);
    });
    connect(m_projectExplorer, &ProjectExplorer::fileActivated, this, &MainWindow::openFile);
    connect(m_editorArea, &Editor::EditorArea::fileOpened, this, [this](const QString &path) {
        m_settings.addRecentFile(path);
    });

    // ---- Language intelligence (clangd) + live status ----
    m_clangdChangeTimer = new QTimer(this);
    m_clangdChangeTimer->setSingleShot(true);
    m_clangdChangeTimer->setInterval(300);
    connect(m_clangdChangeTimer, &QTimer::timeout, this, &MainWindow::flushClangdChanges);

    connect(&m_clangd, &Languages::ClangdLanguageService::diagnosticsReceived, this,
            [this](const QString &uri, const QJsonArray &diagnostics) {
        const QString path = MalloyWriter::Base::normalizePath(QUrl(uri).toLocalFile());
        m_clangdDiagnostics.removeIf([&path](const Platform::Diagnostic &diagnostic) {
            return MalloyWriter::Base::normalizePath(diagnostic.filePath) == path;
        });
        m_clangdDiagnostics += Languages::diagnosticsFromLsp(uri, diagnostics);
        refreshDiagnostics();
    });
    connect(&m_clangd, &Languages::ClangdLanguageService::statusChanged, this,
            [this](const QString &status) { m_statusBar->setClangdStatus(status, true); });

    connect(&m_documentService, &Platform::DocumentService::documentOpened, this,
            [this](Editor::Document *document) {
        if (!document || !isCppPath(document->path())) {
            return;
        }
        m_clangdVersions.insert(document->path(), 1);
        m_clangd.didOpen(document->path(), QStringLiteral("cpp"), document->text());
        connect(document, &Editor::Document::textChanged, this, [this, document]() {
            if (isCppPath(document->path())) {
                m_pendingClangdChanges.insert(document->path());
                m_clangdChangeTimer->start();
            }
        });
    });
    connect(&m_documentService, &Platform::DocumentService::documentSaved, this,
            [this](Editor::Document *document) {
        if (document && isCppPath(document->path())) {
            m_clangd.didSave(document->path());
        }
    });

    connect(m_editorArea, &Editor::EditorArea::cursorMoved, this,
            [this](int line, int column) { m_statusBar->setCursorPosition(line, column); });
    connect(m_editorArea, &Editor::EditorArea::currentDocumentChanged, this,
            [this](Editor::Document *document) { updateLanguageMode(document); });

    restoreRecentWorkspace();
}

void MainWindow::setupUi()
{
    setWindowTitle(tr("MalloyWriter %1").arg(MalloyWriter::Base::appVersion()));
    resize(1280, 820);

    // Activity bar + primary sidebar.
    m_activityBar = new ActivityBar(this);
    m_sidebar = new Sidebar(this);
    m_editorArea = new Editor::EditorArea(this);
    m_outputPanel = new OutputPanel(this);
    m_projectExplorer = new ProjectExplorer(this);

    // Explorer view = "Open Editors" group above the workspace file tree.
    auto *explorerPanel = new QWidget(this);
    auto *explorerLayout = new QVBoxLayout(explorerPanel);
    explorerLayout->setContentsMargins(0, 0, 0, 0);
    explorerLayout->setSpacing(0);
    explorerLayout->addWidget(new OpenEditorsList(m_editorArea, explorerPanel));
    explorerLayout->addWidget(m_projectExplorer, 1);
    m_sidebar->setViewWidget(QStringLiteral("explorer"), explorerPanel);

    // Editor region: editor area over a collapsible bottom panel.
    auto *editorSplit = new QSplitter(Qt::Vertical, this);
    editorSplit->addWidget(m_editorArea);
    editorSplit->addWidget(m_outputPanel);
    editorSplit->setStretchFactor(0, 1);
    editorSplit->setStretchFactor(1, 0);
    editorSplit->setSizes({620, 200});

    auto *editorRegion = new QWidget(this);
    editorRegion->setObjectName(QStringLiteral("editorRegion"));
    auto *editorLayout = new QVBoxLayout(editorRegion);
    editorLayout->setContentsMargins(0, 0, 0, 0);
    editorLayout->setSpacing(0);
    editorLayout->addWidget(editorSplit);

    // Assemble the shell row: activity | sidebar | editor region.
    auto *central = new QWidget(this);
    auto *row = new QHBoxLayout(central);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(0);
    row->addWidget(m_activityBar);
    row->addWidget(m_sidebar);
    row->addWidget(editorRegion, 1);
    setCentralWidget(central);

    m_statusBar = new StatusBar(this);
    setStatusBar(m_statusBar);

    m_commandPalette = new CommandPalette(&m_commands, this);

    connect(m_activityBar, &ActivityBar::viewChanged, m_sidebar, &Sidebar::setView);
    connect(m_activityBar, &ActivityBar::settingsRequested, this, [this]() {
        statusBar()->showMessage(tr("Settings view arrives in a later phase"), 4000);
    });
    connect(m_statusBar, &StatusBar::commandPaletteRequested, this, [this]() {
        m_commandPalette->openPalette();
    });
    connect(m_statusBar, &StatusBar::panelToggleRequested, this, &MainWindow::toggleBottomPanel);

    statusBar()->showMessage(tr("Ready"), 3000);
}

void MainWindow::toggleBottomPanel()
{
    if (m_outputPanel) {
        m_outputPanel->setVisible(!m_outputPanel->isVisible());
    }
}

void MainWindow::refreshDiagnostics()
{
    QList<Platform::Diagnostic> merged = m_buildDiagnostics;
    merged += m_clangdDiagnostics;
    m_editorArea->setDiagnostics(merged);

    int errors = 0;
    int warnings = 0;
    for (const Platform::Diagnostic &diagnostic : merged) {
        if (diagnostic.severity == Platform::DiagnosticSeverity::Error) {
            ++errors;
        } else if (diagnostic.severity == Platform::DiagnosticSeverity::Warning) {
            ++warnings;
        }
    }
    m_statusBar->setProblemCounts(errors, warnings);
}

void MainWindow::flushClangdChanges()
{
    for (const QString &path : m_pendingClangdChanges) {
        if (auto *document = m_documentService.documentForPath(path)) {
            m_clangd.didChange(path, document->text(), ++m_clangdVersions[path]);
        }
    }
    m_pendingClangdChanges.clear();
}

void MainWindow::updateLanguageMode(Editor::Document *document)
{
    const auto &theme = MalloyWriter::Base::Theme::active();
    QString label = tr("Plain Text");
    QString icon = QStringLiteral("file");
    QColor color = theme.color(QStringLiteral("muted"));

    if (document) {
        const QString suffix = QFileInfo(document->fileName()).suffix().toLower();
        if (isCppPath(document->path())) {
            label = QStringLiteral("C++");
            icon = suffix.startsWith(QLatin1Char('h')) ? QStringLiteral("filehpp") : QStringLiteral("filecpp");
            color = theme.color(QStringLiteral("tk-type"));
        } else if (suffix == "cmake" || document->fileName().compare(QStringLiteral("CMakeLists.txt"), Qt::CaseInsensitive) == 0) {
            label = QStringLiteral("CMake");
            icon = QStringLiteral("filecmake");
            color = theme.color(QStringLiteral("mod"));
        } else if (suffix == "md") {
            label = QStringLiteral("Markdown");
            icon = QStringLiteral("filemd");
            color = theme.color(QStringLiteral("text-soft"));
        } else if (suffix == "json") {
            label = QStringLiteral("JSON");
            icon = QStringLiteral("filejson");
            color = theme.color(QStringLiteral("tk-num"));
        }
    }
    m_statusBar->setLanguageMode(label, icon, color);
}

void MainWindow::registerCommands()
{
    auto addCommand = [this](QString id, QString title, QString category, QKeySequence shortcut, std::function<void()> handler) {
        m_commands.registerCommand({std::move(id), std::move(title), std::move(category), std::move(shortcut), std::move(handler)});
    };

    addCommand(MalloyWriter::Base::Commands::OpenFolder, tr("Open Folder"), tr("File"), QKeySequence("Ctrl+K,Ctrl+O"), [this]() { openFolderDialog(); });
    addCommand(MalloyWriter::Base::Commands::OpenFile, tr("Open File"), tr("File"), QKeySequence::Open, [this]() { openFileDialog(); });
    addCommand(MalloyWriter::Base::Commands::Save, tr("Save"), tr("File"), QKeySequence::Save, [this]() {
        QString error;
        if (!m_documentService.saveDocument(m_editorArea->currentDocument(), &error)) {
            statusBar()->showMessage(error.isEmpty() ? tr("Nothing saved") : error, 5000);
        }
    });
    addCommand(MalloyWriter::Base::Commands::SaveAll, tr("Save All"), tr("File"), QKeySequence("Ctrl+K,S"), [this]() {
        statusBar()->showMessage(m_documentService.saveAll() ? tr("All files saved") : tr("Some files could not be saved"), 4000);
    });
    addCommand(MalloyWriter::Base::Commands::Exit, tr("Exit"), tr("File"), QKeySequence::Quit, []() { QApplication::quit(); });

    addCommand(MalloyWriter::Base::Commands::CommandPalette, tr("Command Palette"), tr("View"), QKeySequence("Ctrl+Shift+P"), [this]() {
        m_commandPalette->openPalette();
    });
    addCommand(MalloyWriter::Base::Commands::ClearOutput, tr("Clear Output"), tr("View"), {}, [this]() { m_outputPanel->clear(); });
    addCommand(MalloyWriter::Base::Commands::Find, tr("Find"), tr("Editor"), QKeySequence::Find, [this]() { findInCurrentDocument(); });
    addCommand(MalloyWriter::Base::Commands::Replace, tr("Replace"), tr("Editor"), QKeySequence::Replace, [this]() { replaceInCurrentDocument(); });
    addCommand(MalloyWriter::Base::Commands::GoToLine, tr("Go To Line"), tr("Editor"), QKeySequence("Ctrl+G"), [this]() { goToLineInCurrentDocument(); });

    addCommand(MalloyWriter::Base::Commands::Configure, tr("Configure CMake"), tr("Build"), QKeySequence("Ctrl+Shift+C"), [this]() { configureProject(); });
    addCommand(MalloyWriter::Base::Commands::Build, tr("Build Project"), tr("Build"), QKeySequence("Ctrl+Shift+B"), [this]() { buildProject(); });
    addCommand(MalloyWriter::Base::Commands::Test, tr("Run Tests"), tr("Build"), QKeySequence("Ctrl+Shift+T"), [this]() { testProject(); });
    addCommand(MalloyWriter::Base::Commands::RunExecutable, tr("Run Executable"), tr("Build"), QKeySequence("Ctrl+F5"), [this]() { runExecutable(); });
    addCommand(MalloyWriter::Base::Commands::Stop, tr("Stop Running Process"), tr("Build"), {}, [this]() { m_buildService.stop(); });
    addCommand(MalloyWriter::Base::Commands::RefreshProject, tr("Refresh CMake Model"), tr("Project"), {}, [this]() {
        QString error;
        if (!m_projectService.refreshCMakeModel(&error)) {
            statusBar()->showMessage(error, 5000);
        } else {
            statusBar()->showMessage(tr("Project model refreshed"), 4000);
        }
    });
}

void MainWindow::buildMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::OpenFolder));
    fileMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::OpenFile));
    m_recentWorkspacesMenu = fileMenu->addMenu(tr("Recent Workspaces"));
    m_recentFilesMenu = fileMenu->addMenu(tr("Recent Files"));
    fileMenu->addSeparator();
    fileMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::Save));
    fileMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::SaveAll));
    fileMenu->addSeparator();
    fileMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::Exit));

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::CommandPalette));
    viewMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::ClearOutput));

    QMenu *editorMenu = menuBar()->addMenu(tr("&Editor"));
    editorMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::Find));
    editorMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::Replace));
    editorMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::GoToLine));

    QMenu *buildMenu = menuBar()->addMenu(tr("&Build"));
    buildMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::Configure));
    buildMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::Build));
    buildMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::Test));
    buildMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::RunExecutable));
    buildMenu->addSeparator();
    buildMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::Stop));

    QMenu *projectMenu = menuBar()->addMenu(tr("&Project"));
    projectMenu->addAction(createActionForCommand(MalloyWriter::Base::Commands::RefreshProject));

    updateRecentMenus();
}

QAction *MainWindow::createActionForCommand(const QString &commandId)
{
    const auto command = m_commands.command(commandId);
    auto *action = new QAction(command.title.isEmpty() ? commandId : command.title, this);
    action->setShortcut(command.shortcut);
    action->setStatusTip(command.id);
    connect(action, &QAction::triggered, this, [this, commandId]() {
        m_commands.execute(commandId);
    });
    return action;
}

void MainWindow::openFolderDialog()
{
    const QString startPath = m_projectService.hasWorkspace() ? m_projectService.workspaceRoot() : QDir::homePath();
    const QString path = QFileDialog::getExistingDirectory(this, tr("Open Folder"), startPath);
    if (!path.isEmpty()) {
        openWorkspace(path);
    }
}

void MainWindow::openFileDialog()
{
    const QString startPath = m_projectService.hasWorkspace() ? m_projectService.workspaceRoot() : QDir::homePath();
    const QString path = QFileDialog::getOpenFileName(this, tr("Open File"), startPath);
    if (!path.isEmpty()) {
        openFile(path);
    }
}

void MainWindow::openWorkspace(const QString &path)
{
    if (!m_projectService.openWorkspace(path)) {
        QMessageBox::warning(this, tr("Open Folder"), tr("Could not open folder %1").arg(path));
        return;
    }

    m_projectExplorer->setRootPath(m_projectService.workspaceRoot());
    m_settings.addRecentWorkspace(m_projectService.workspaceRoot());
    setWindowTitle(tr("MalloyWriter %1 - %2").arg(MalloyWriter::Base::appVersion(), MalloyWriter::Base::displayNameForPath(m_projectService.workspaceRoot())));
    statusBar()->showMessage(tr("Opened %1").arg(m_projectService.workspaceRoot()), 5000);

    // Restart clangd for the new workspace root (header-only until a configure
    // produces compile_commands.json).
    m_clangd.stop();
    m_clangdDiagnostics.clear();
    m_clangdVersions.clear();
    const Platform::ToolchainKit kit = m_projectService.activeKit();
    if (!kit.clangdProgram.isEmpty() && m_clangd.start(kit, m_projectService.workspaceRoot())) {
        m_statusBar->setClangdStatus(tr("clangd"), true);
    } else {
        m_statusBar->setClangdStatus(tr("clangd: off"), false);
    }
    refreshDiagnostics();
}

void MainWindow::openFile(const QString &path)
{
    if (!MalloyWriter::Base::isLikelyTextFile(path)) {
        const auto choice = QMessageBox::question(
            this,
            tr("Open File"),
            tr("%1 may not be a text file. Open it anyway?").arg(MalloyWriter::Base::displayNameForPath(path)),
            QMessageBox::Open | QMessageBox::Cancel,
            QMessageBox::Cancel);
        if (choice != QMessageBox::Open) {
            return;
        }
    }

    auto *document = m_documentService.openDocument(path);
    if (!document || !m_editorArea->openDocument(document)) {
        QMessageBox::warning(this, tr("Open File"), tr("Could not open %1").arg(path));
    }
}

void MainWindow::configureProject()
{
    if (!m_projectService.hasWorkspace()) {
        openFolderDialog();
    }
    if (!m_projectService.hasWorkspace()) {
        return;
    }

    const QFileInfo cmakeLists(QDir(m_projectService.workspaceRoot()).filePath("CMakeLists.txt"));
    if (!cmakeLists.exists()) {
        QMessageBox::information(this, tr("Configure CMake"), tr("This workspace does not contain a CMakeLists.txt file."));
        return;
    }

    m_buildService.configure();
}

void MainWindow::buildProject()
{
    if (!m_projectService.hasWorkspace()) {
        openFolderDialog();
    }
    if (!m_projectService.hasWorkspace()) {
        return;
    }

    m_buildService.build();
}

void MainWindow::testProject()
{
    if (!m_projectService.hasWorkspace()) {
        openFolderDialog();
    }
    if (!m_projectService.hasWorkspace()) {
        return;
    }

    m_buildService.test();
}

void MainWindow::runExecutable()
{
    if (!m_projectService.hasWorkspace()) {
        openFolderDialog();
    }
    if (!m_projectService.hasWorkspace()) {
        return;
    }

    const Platform::BuildSettings settings = m_projectService.buildSettings();
    const QString startPath = Platform::CMakeBuildPlanner::resolvedBuildDirectory(m_projectService.workspaceRoot(), settings);
    const QString path = QFileDialog::getOpenFileName(this, tr("Run Executable"), startPath, executableFilter().join(";;"));
    if (path.isEmpty()) {
        return;
    }

    m_settings.setLastRunExecutable(path);
    m_buildService.runExecutable(path);
}

void MainWindow::findInCurrentDocument()
{
    bool ok = false;
    const QString text = QInputDialog::getText(this, tr("Find"), tr("Find:"), QLineEdit::Normal, {}, &ok);
    if (ok && !text.isEmpty() && !m_editorArea->findInCurrent(text)) {
        statusBar()->showMessage(tr("No match found"), 3000);
    }
}

void MainWindow::replaceInCurrentDocument()
{
    bool ok = false;
    const QString findText = QInputDialog::getText(this, tr("Replace"), tr("Find:"), QLineEdit::Normal, {}, &ok);
    if (!ok || findText.isEmpty()) {
        return;
    }

    const QString replaceText = QInputDialog::getText(this, tr("Replace"), tr("Replace with:"), QLineEdit::Normal, {}, &ok);
    if (ok && !m_editorArea->replaceInCurrent(findText, replaceText)) {
        statusBar()->showMessage(tr("No match found"), 3000);
    }
}

void MainWindow::goToLineInCurrentDocument()
{
    bool ok = false;
    const int line = QInputDialog::getInt(this, tr("Go To Line"), tr("Line:"), 1, 1, 1000000, 1, &ok);
    if (ok) {
        m_editorArea->goToLineInCurrent(line);
    }
}

void MainWindow::updateRecentMenus()
{
    auto rebuildMenu = [this](QMenu *menu, const QStringList &paths, auto opener) {
        menu->clear();
        if (paths.isEmpty()) {
            auto *emptyAction = menu->addAction(tr("No recent items"));
            emptyAction->setEnabled(false);
            return;
        }

        for (const QString &path : paths) {
            auto *action = menu->addAction(MalloyWriter::Base::compactPath(path));
            action->setToolTip(path);
            connect(action, &QAction::triggered, this, [path, opener]() { opener(path); });
        }
    };

    if (m_recentWorkspacesMenu) {
        rebuildMenu(m_recentWorkspacesMenu, m_settings.recentWorkspaces(), [this](const QString &path) { openWorkspace(path); });
    }
    if (m_recentFilesMenu) {
        rebuildMenu(m_recentFilesMenu, m_settings.recentFiles(), [this](const QString &path) { openFile(path); });
    }
}

void MainWindow::restoreRecentWorkspace()
{
    const QStringList recent = m_settings.recentWorkspaces();
    for (const QString &path : recent) {
        if (QFileInfo(path).isDir()) {
            openWorkspace(path);
            return;
        }
    }
}

}
