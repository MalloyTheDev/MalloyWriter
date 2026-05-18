#pragma once

#include "platform/BuildService.hpp"
#include "editor/EditorArea.hpp"
#include "platform/CommandRegistry.hpp"
#include "platform/DocumentService.hpp"
#include "platform/ProjectService.hpp"
#include "platform/SettingsService.hpp"

#include <QMainWindow>

class QAction;
class QMenu;

namespace MalloyWriter::Workbench {

class CommandPalette;
class OutputPanel;
class ProjectExplorer;

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

    MalloyWriter::Platform::CommandRegistry m_commands;
    MalloyWriter::Platform::SettingsService m_settings;
    MalloyWriter::Platform::ProjectService m_projectService;
    MalloyWriter::Platform::DocumentService m_documentService;
    MalloyWriter::Platform::BuildService m_buildService;

    MalloyWriter::Editor::EditorArea *m_editorArea = nullptr;
    ProjectExplorer *m_projectExplorer = nullptr;
    OutputPanel *m_outputPanel = nullptr;
    CommandPalette *m_commandPalette = nullptr;
    QMenu *m_recentFilesMenu = nullptr;
    QMenu *m_recentWorkspacesMenu = nullptr;
};

}
