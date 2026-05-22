#include "editor/EditorArea.hpp"

#include "base/PathUtils.hpp"
#include "base/Theme.hpp"
#include "editor/CodeEditor.hpp"
#include "editor/EditorTabBar.hpp"
#include "editor/EditorWidget.hpp"
#include "workbench/Icon.hpp"

#include <QTextCursor>

#include <QFileInfo>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QToolButton>

namespace MalloyWriter::Editor {

EditorArea::EditorArea(QWidget *parent)
    : QTabWidget(parent)
{
    setTabBar(new EditorTabBar(this));
    setDocumentMode(true);
    setMovable(true);
    setTabsClosable(false); // the custom tab bar paints/handles its own close affordance

    // Trailing tab-strip actions (decorative for now): chat / split / more.
    auto *actions = new QWidget(this);
    auto *actionsLayout = new QHBoxLayout(actions);
    actionsLayout->setContentsMargins(4, 0, 6, 0);
    actionsLayout->setSpacing(2);
    const QColor iconColor = Base::Theme::active().color(QStringLiteral("text-soft"));
    const struct { const char *icon; const char *tip; } actionDefs[] = {
        {"ai", "Open Chat"}, {"split", "Split Editor"}, {"more", "More Actions"},
    };
    for (const auto &def : actionDefs) {
        auto *button = new QToolButton(actions);
        button->setObjectName(QStringLiteral("iconBtn"));
        button->setIcon(Workbench::Icon::icon(QString::fromLatin1(def.icon), 14, iconColor));
        button->setIconSize(QSize(14, 14));
        button->setToolTip(QString::fromLatin1(def.tip));
        button->setAutoRaise(true);
        button->setFixedSize(24, 24);
        actionsLayout->addWidget(button);
    }
    setCornerWidget(actions, Qt::TopRightCorner);

    connect(this, &QTabWidget::tabCloseRequested, this, &EditorArea::closeEditorTab);
    connect(this, &QTabWidget::currentChanged, this, [this](int) {
        emit currentDocumentChanged(currentDocument());
        if (auto *editor = currentEditor()) {
            const QTextCursor cursor = editor->textEdit()->textCursor();
            emit cursorMoved(cursor.blockNumber() + 1, cursor.columnNumber() + 1);
        }
    });
}

Document *EditorArea::currentDocument() const
{
    auto *editor = qobject_cast<EditorWidget *>(currentWidget());
    return editor ? editor->documentModel() : nullptr;
}

QList<Document *> EditorArea::orderedDocuments() const
{
    QList<Document *> documents;
    for (int i = 0; i < count(); ++i) {
        if (auto *editor = qobject_cast<EditorWidget *>(widget(i)); editor && editor->documentModel()) {
            documents.append(editor->documentModel());
        }
    }
    return documents;
}

void EditorArea::activateDocument(Document *document)
{
    if (auto *editor = widgetForDocument(document)) {
        setCurrentWidget(editor);
    }
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
    EditorTabBar::setTabMeta(tabBar(), tabIndex, document->fileName(), document->isDirty(), false);
    setCurrentIndex(tabIndex);
    m_openDocumentsByPath.insert(normalizedPath, document);

    connect(document, &Document::dirtyChanged, this, [this, document]() {
        updateTabTitle(document);
        emit openDocumentsChanged();
    });
    connect(document, &Document::pathChanged, this, [this, document]() {
        updateTabTitle(document);
        emit openDocumentsChanged();
    });
    connect(editor->textEdit(), &QPlainTextEdit::cursorPositionChanged, this, [this, editor]() {
        if (currentWidget() == editor) {
            const QTextCursor cursor = editor->textEdit()->textCursor();
            emit cursorMoved(cursor.blockNumber() + 1, cursor.columnNumber() + 1);
        }
    });

    emit fileOpened(normalizedPath);
    emit currentDocumentChanged(document);
    emit openDocumentsChanged();
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
    emit openDocumentsChanged();
    emit currentDocumentChanged(currentDocument());
}

void EditorArea::updateTabTitle(Document *document)
{
    if (!document) {
        return;
    }

    if (auto *editor = widgetForDocument(document)) {
        const int index = indexOf(editor);
        setTabText(index, document->fileName());
        setTabToolTip(index, document->path());
        EditorTabBar::setTabMeta(tabBar(), index, document->fileName(), document->isDirty(), false);
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
