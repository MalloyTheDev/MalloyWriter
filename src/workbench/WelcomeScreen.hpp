#pragma once

#include <QStringList>
#include <QWidget>

class QVBoxLayout;

namespace MalloyWriter::Workbench {

// The Welcome screen shown in the main area when no file is open: brand hero,
// Start action cards, and a Recent list. Mirrors MalloyIDE/WelcomeScreen.jsx.
class WelcomeScreen : public QWidget {
    Q_OBJECT

public:
    explicit WelcomeScreen(QWidget *parent = nullptr);

    void setRecentWorkspaces(const QStringList &paths);

signals:
    void openFolderRequested();
    void openFileRequested();
    void commandPaletteRequested();
    void openRecentRequested(const QString &path);

private:
    QVBoxLayout *m_recentLayout = nullptr;
};

} // namespace MalloyWriter::Workbench
