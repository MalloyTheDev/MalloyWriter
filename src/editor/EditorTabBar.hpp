#pragma once

#include <QTabBar>

namespace MalloyWriter::Editor {

// Custom-painted editor tab bar matching MalloyIDE/TabBar.jsx: file-type icon +
// color, name, a modified dot (becomes a close "x" on hover), preview italic,
// and an active top-accent line. Per-tab metadata is stored in tab data as a
// QVariantMap with keys "file" (QString), "modified" (bool), "preview" (bool).
class EditorTabBar : public QTabBar {
    Q_OBJECT

public:
    explicit EditorTabBar(QWidget *parent = nullptr);

    static void setTabMeta(QTabBar *bar, int index, const QString &fileName, bool modified, bool preview);

protected:
    QSize tabSizeHint(int index) const override;
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QRect closeRect(const QRect &tabRect) const;

    int m_hoveredTab = -1;
};

} // namespace MalloyWriter::Editor
