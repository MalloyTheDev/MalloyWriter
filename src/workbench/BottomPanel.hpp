#pragma once

#include "platform/Diagnostic.hpp"

#include <QList>
#include <QString>
#include <QWidget>

class QButtonGroup;
class QLineEdit;
class QPlainTextEdit;
class QProcess;
class QPushButton;
class QStackedWidget;
class QTreeWidget;

namespace MalloyWriter::Workbench {

class OutputPanel;

// Problems list: diagnostics grouped by file. Activating a row asks the host to
// open that file at the diagnostic's line.
class ProblemsView : public QWidget {
    Q_OBJECT
public:
    explicit ProblemsView(QWidget *parent = nullptr);
    void setDiagnostics(const QList<MalloyWriter::Platform::Diagnostic> &diagnostics);

signals:
    void activated(const QString &path, int line, int column);

private:
    QTreeWidget *m_tree = nullptr;
};

// Basic command runner: each entered command runs via QProcess in the workspace
// directory and streams stdout/stderr. A full PTY/ANSI terminal is a later phase.
class TerminalView : public QWidget {
    Q_OBJECT
public:
    explicit TerminalView(QWidget *parent = nullptr);
    void setWorkingDirectory(const QString &dir);

private:
    void runCommand(const QString &command);

    QPlainTextEdit *m_output = nullptr;
    QLineEdit *m_input = nullptr;
    QProcess *m_process = nullptr;
    QString m_cwd;
};

// The tabbed bottom panel: Problems / Output / Debug Console / Terminal / Ports.
class BottomPanel : public QWidget {
    Q_OBJECT
public:
    explicit BottomPanel(QWidget *parent = nullptr);

    OutputPanel *outputPanel() const { return m_output; }
    void setProblems(const QList<MalloyWriter::Platform::Diagnostic> &diagnostics);
    void setWorkspaceRoot(const QString &root);
    void showProblems();

signals:
    void closeRequested();
    void problemActivated(const QString &path, int line, int column);

private:
    QButtonGroup *m_tabs = nullptr;
    QStackedWidget *m_stack = nullptr;
    ProblemsView *m_problems = nullptr;
    OutputPanel *m_output = nullptr;
    TerminalView *m_terminal = nullptr;
    QPushButton *m_problemsTab = nullptr;
};

} // namespace MalloyWriter::Workbench
