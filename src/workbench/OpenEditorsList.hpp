#pragma once

#include <QWidget>

class QVBoxLayout;

namespace MalloyWriter::Editor {
class EditorArea;
}

namespace MalloyWriter::Workbench {

// The "OPEN EDITORS" group shown above the file tree in the Explorer. Observes
// the EditorArea and lists open documents (file icon + name + modified dot);
// clicking a row activates that tab. Hidden when nothing is open.
class OpenEditorsList : public QWidget {
    Q_OBJECT

public:
    explicit OpenEditorsList(MalloyWriter::Editor::EditorArea *editorArea, QWidget *parent = nullptr);

private:
    void rebuild();

    MalloyWriter::Editor::EditorArea *m_editorArea = nullptr;
    QVBoxLayout *m_rows = nullptr;
};

} // namespace MalloyWriter::Workbench
