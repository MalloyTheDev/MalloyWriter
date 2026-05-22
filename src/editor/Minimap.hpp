#pragma once

#include <QVector>
#include <QWidget>

class QTimer;

namespace MalloyWriter::Editor {

class CodeEditor;

// A compact code overview (MalloyIDE/CodeEditor.jsx Minimap): one thin colored
// row per source line (width ∝ trimmed length, color by dominant token) with a
// viewport box reflecting the editor's scroll position. Per-line data is cached
// on text change; scrolling only repaints the viewport box.
class Minimap : public QWidget {
    Q_OBJECT

public:
    explicit Minimap(CodeEditor *editor, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    enum Category { Default, Comment, Keyword, String, Function };
    struct Row {
        Category category = Default;
        double widthFraction = 0.0;
    };

    void rebuild();

    CodeEditor *m_editor = nullptr;
    QTimer *m_rebuildTimer = nullptr;
    QVector<Row> m_rows;
};

} // namespace MalloyWriter::Editor
