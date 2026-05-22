#pragma once

#include <QStatusBar>

class QLabel;

namespace MalloyWriter::Workbench {

// The rich status bar: dense left/right item groups (branch, problems, clangd,
// cursor position, language, etc.). Mirrors MalloyIDE/StatusBar.jsx. Transient
// messages still work via QStatusBar::showMessage. The cursor/language/problem/
// clangd cells are live and driven by MainWindow; branch/sync/task are static.
class StatusBar : public QStatusBar {
    Q_OBJECT

public:
    explicit StatusBar(QWidget *parent = nullptr);

    void setCursorPosition(int line, int column);
    void setLanguageMode(const QString &id, const QString &iconName, const QColor &color);
    void setProblemCounts(int errors, int warnings);
    void setClangdStatus(const QString &text, bool ok);

signals:
    void commandPaletteRequested();
    void panelToggleRequested();

private:
    void buildItems();

    QLabel *m_cursorLabel = nullptr;
    QLabel *m_languageLabel = nullptr;
    QLabel *m_languageIcon = nullptr;
    QLabel *m_errorCountLabel = nullptr;
    QLabel *m_warningCountLabel = nullptr;
    QLabel *m_clangdLabel = nullptr;
    QLabel *m_clangdDot = nullptr;
};

} // namespace MalloyWriter::Workbench
