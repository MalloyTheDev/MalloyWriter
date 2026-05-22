#include "workbench/OpenEditorsList.hpp"

#include "base/Theme.hpp"
#include "editor/Document.hpp"
#include "editor/EditorArea.hpp"
#include "workbench/Icon.hpp"

#include <QLabel>
#include <QLayoutItem>
#include <QPushButton>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

OpenEditorsList::OpenEditorsList(Editor::EditorArea *editorArea, QWidget *parent)
    : QWidget(parent)
    , m_editorArea(editorArea)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 6, 0, 6);
    layout->setSpacing(0);

    auto *header = new QLabel(tr("OPEN EDITORS"), this);
    header->setObjectName(QStringLiteral("groupHeader"));
    header->setContentsMargins(14, 0, 8, 4);
    layout->addWidget(header);

    auto *rowsContainer = new QWidget(this);
    m_rows = new QVBoxLayout(rowsContainer);
    m_rows->setContentsMargins(0, 0, 0, 0);
    m_rows->setSpacing(0);
    layout->addWidget(rowsContainer);

    if (m_editorArea) {
        connect(m_editorArea, &Editor::EditorArea::openDocumentsChanged, this, &OpenEditorsList::rebuild);
        connect(m_editorArea, &Editor::EditorArea::currentDocumentChanged, this, [this]() { rebuild(); });
    }
    rebuild();
}

void OpenEditorsList::rebuild()
{
    while (QLayoutItem *item = m_rows->takeAt(0)) {
        if (QWidget *w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }

    if (!m_editorArea) {
        return;
    }

    const QList<Editor::Document *> documents = m_editorArea->orderedDocuments();
    setVisible(!documents.isEmpty());

    Editor::Document *current = m_editorArea->currentDocument();
    for (Editor::Document *document : documents) {
        const FileType type = Icon::fileType(document->fileName());
        const QString name = document->isDirty() ? document->fileName() + QStringLiteral("  ●")
                                                  : document->fileName();
        auto *row = new QPushButton(Icon::icon(type.iconName, 14, type.color), name);
        row->setObjectName(QStringLiteral("openEditorRow"));
        row->setIconSize(QSize(14, 14));
        row->setCursor(Qt::PointingHandCursor);
        row->setToolTip(document->path());
        row->setProperty("active", document == current);
        connect(row, &QPushButton::clicked, this, [this, document]() {
            m_editorArea->activateDocument(document);
        });
        m_rows->addWidget(row);
    }
}

} // namespace MalloyWriter::Workbench
