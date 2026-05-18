#include "editor/EditorArea.hpp"

#include "base/PathUtils.hpp"
#include "editor/EditorWidget.hpp"

#include <QFileInfo>
#include <QMessageBox>

namespace MalloyWriter::Editor {

EditorArea::EditorArea(QWidget *parent)
    : QTabWidget(parent)
{
    setDocumentMode(true);
    setMovable(true);
    setTabsClosable(true);

    connect(this, &QTabWidget::tabCloseRequested, this, &EditorArea::closeEditorTab);
    connect(this, &QTabWidget::currentChanged, this, [this](int) {
        emit currentDocumentChanged(currentDocument());
    });
}

Document *EditorArea::currentDocument() const
{
    auto *editor = qobject_cast<EditorWidget *>(currentWidget());
    return editor ? editor->documentModel() : nullptr;
}

bool EditorArea::openFile(const QString &path)
{
    const QString normalizedPath = MalloyWriter::Base::normalizePath(path);
    if (auto *existing = m_openDocumentsByPath.value(normalizedPath, nullptr)) {
        if (auto *editor = widgetForDocument(existing)) {
            setCurrentWidget(editor);
            return true;
        }
    }

    auto *document = new Document(this);
    if (!document->load(normalizedPath)) {
        document->deleteLater();
        QMessageBox::warning(this, tr("Open File"), tr("Could not open %1").arg(normalizedPath));
        return false;
    }

    return openDocument(document);
}

bool EditorArea::openDocument(Document *document)
{
    if (!document) {
        return false;
    }

    const QString normalizedPath = MalloyWriter::Base::normalizePath(document->path());
    if (auto *existing = m_openDocumentsByPath.value(normalizedPath, nullptr)) {
        if (auto *editor = widgetForDocument(existing)) {
            setCurrentWidget(editor);
            return true;
        }
    }

    auto *editor = new EditorWidget(document, this);
    const int tabIndex = addTab(editor, document->fileName());
    setCurrentIndex(tabIndex);
    m_openDocumentsByPath.insert(normalizedPath, document);

    connect(document, &Document::dirtyChanged, this, [this, document]() {
        updateTabTitle(document);
    });
    connect(document, &Document::pathChanged, this, [this, document]() {
        updateTabTitle(document);
    });

    emit fileOpened(normalizedPath);
    emit currentDocumentChanged(document);
    return true;
}

bool EditorArea::saveCurrent()
{
    Document *document = currentDocument();
    return document ? document->save() : false;
}

bool EditorArea::saveAll()
{
    bool allSaved = true;
    for (int i = 0; i < count(); ++i) {
        auto *editor = qobject_cast<EditorWidget *>(widget(i));
        if (!editor || !editor->documentModel()) {
            continue;
        }
        allSaved = editor->documentModel()->save() && allSaved;
    }
    return allSaved;
}

void EditorArea::setDiagnostics(const QList<MalloyWriter::Platform::Diagnostic> &diagnostics)
{
    for (int i = 0; i < count(); ++i) {
        auto *editor = qobject_cast<EditorWidget *>(widget(i));
        if (!editor || !editor->documentModel()) {
            continue;
        }

        QList<MalloyWriter::Platform::Diagnostic> documentDiagnostics;
        const QString path = MalloyWriter::Base::normalizePath(editor->documentModel()->path());
        for (const auto &diagnostic : diagnostics) {
            if (MalloyWriter::Base::normalizePath(diagnostic.filePath) == path) {
                documentDiagnostics << diagnostic;
            }
        }
        editor->setDiagnostics(documentDiagnostics);
    }
}

bool EditorArea::findInCurrent(const QString &text, bool caseSensitive)
{
    auto *editor = currentEditor();
    return editor && editor->findNext(text, caseSensitive);
}

bool EditorArea::replaceInCurrent(const QString &findText, const QString &replaceText, bool caseSensitive)
{
    auto *editor = currentEditor();
    return editor && editor->replaceNext(findText, replaceText, caseSensitive);
}

void EditorArea::goToLineInCurrent(int line)
{
    if (auto *editor = currentEditor()) {
        editor->goToLine(line);
    }
}

void EditorArea::closeEditorTab(int index)
{
    auto *editor = qobject_cast<EditorWidget *>(widget(index));
    if (!editor || !editor->documentModel()) {
        removeTab(index);
        return;
    }

    Document *document = editor->documentModel();
    if (document->isDirty()) {
        const QMessageBox::StandardButton choice = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("Save changes to %1?").arg(document->fileName()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);

        if (choice == QMessageBox::Cancel) {
            return;
        }
        if (choice == QMessageBox::Save && !document->save()) {
            QMessageBox::warning(this, tr("Save File"), tr("Could not save %1").arg(document->path()));
            return;
        }
    }

    m_openDocumentsByPath.remove(MalloyWriter::Base::normalizePath(document->path()));
    removeTab(index);
    editor->deleteLater();
}

void EditorArea::updateTabTitle(Document *document)
{
    if (!document) {
        return;
    }

    if (auto *editor = widgetForDocument(document)) {
        const int index = indexOf(editor);
        const QString title = document->isDirty() ? QString("*%1").arg(document->fileName()) : document->fileName();
        setTabText(index, title);
        setTabToolTip(index, document->path());
    }
}

EditorWidget *EditorArea::widgetForDocument(Document *document) const
{
    for (int i = 0; i < count(); ++i) {
        auto *editor = qobject_cast<EditorWidget *>(widget(i));
        if (editor && editor->documentModel() == document) {
            return editor;
        }
    }
    return nullptr;
}

EditorWidget *EditorArea::currentEditor() const
{
    return qobject_cast<EditorWidget *>(currentWidget());
}

}
