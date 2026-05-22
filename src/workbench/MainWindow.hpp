#pragma once

#include "platform/BuildService.hpp"
#include "editor/EditorArea.hpp"
#include "languages/ClangdLanguageService.hpp"
#include "platform/CommandRegistry.hpp"
#include "platform/Diagnostic.hpp"
#include "platform/DocumentService.hpp"
#include "platform/ProjectService.hpp"
#include "platform/SettingsService.hpp"

#include <QHash>
#include <QList>
#include <QMainWindow>
#include <QSet>

class QAction;
class QMenu;
class QTimer;

namespace MalloyWriter::Workbench {

class ActivityBar;
class BottomPanel;
class CommandPalette;
class OutputPanel;
class ProjectExplorer;
class Sidebar;
class StatusBar;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUi();
    void registerCommands();
    void buildMenus();
    QAction *createActionForCommand(const QString &commandId);

    void openFolderDialog();
    void openFileDialog();
    void openWorkspace(const QString &path);
    void openFile(const QString &path);

    void configureProject();
    void buildProject();
    void testProject();
    void runExecutable();
    void findInCurrentDocument();
    void replaceInCurrentDocument();
    void goToLineInCurrentDocument();
    void updateRecentMenus();
    void restoreRecentWorkspace();
    void toggleBottomPanel();

    void refreshDiagnostics();
    void flushClangdChanges();
    void updateLanguageMode(MalloyWriter::Editor::Document *document);

    MalloyWriter::Platform::CommandRegistry m_commands;
    MalloyWriter::Platform::SettingsService m_settings;
    MalloyWriter::Platform::ProjectService m_projectService;
    MalloyWriter::Platform::DocumentService m_documentService;
    MalloyWriter::Platform::BuildService m_buildService;
    MalloyWriter::Languages::ClangdLanguageService m_clangd;

    QList<MalloyWriter::Platform::Diagnostic> m_buildDiagnostics;
    QList<MalloyWriter::Platform::Diagnostic> m_clangdDiagnostics;
    QHash<QString, int> m_clangdVersions;
    QSet<QString> m_pendingClangdChanges;
    QTimer *m_clangdChangeTimer = nullptr;

    MalloyWriter::Editor::EditorArea *m_editorArea = nullptr;
    ActivityBar *m_activityBar = nullptr;
    Sidebar *m_sidebar = nullptr;
    StatusBar *m_statusBar = nullptr;
    ProjectExplorer *m_projectExplorer = nullptr;
    BottomPanel *m_bottomPanel = nullptr;
    OutputPanel *m_outputPanel = nullptr;
    CommandPalette *m_commandPalette = nullptr;
    QMenu *m_recentFilesMenu = nullptr;
    QMenu *m_recentWorkspacesMenu = nullptr;
};

}
