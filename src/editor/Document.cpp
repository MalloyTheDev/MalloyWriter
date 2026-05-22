#include "editor/Document.hpp"

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

namespace MalloyWriter::Editor {
namespace {

QString detectLineEnding(const QByteArray &bytes)
{
    const int crlf = bytes.count("\r\n");
    const int lf = bytes.count('\n') - crlf;
    return crlf > lf ? "\r\n" : "\n";
}

}

Document::Document(QObject *parent)
    : QObject(parent)
{
}

QString Document::path() const
{
    return m_path;
}

QString Document::fileName() const
{
    const QFileInfo info(m_path);
    return info.fileName().isEmpty() ? "Untitled" : info.fileName();
}

QString Document::text() const
{
    return m_text;
}

bool Document::isDirty() const
{
    return m_dirty;
}

bool Document::hasExternalModification() const
{
    if (m_path.isEmpty() || !m_lastSavedAtUtc.isValid()) {
        return false;
    }

    const QFileInfo info(m_path);
    return info.exists() && info.lastModified().toUTC() > m_lastSavedAtUtc.addMSecs(5);
}

QString Document::lineEnding() const
{
    return m_lineEnding;
}

QDateTime Document::lastSavedAtUtc() const
{
    return m_lastSavedAtUtc;
}

bool Document::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    const QString normalizedPath = QFileInfo(path).absoluteFilePath();
    const QByteArray bytes = file.readAll();
    m_lineEnding = detectLineEnding(bytes);
    // Store text with '\n' line endings internally (the original ending is
    // remembered in m_lineEnding and restored on save). This keeps the document
    // in sync with QPlainTextEdit, which always uses '\n'.
    m_text = QString::fromUtf8(bytes);
    m_text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    m_text.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    if (m_path != normalizedPath) {
        m_path = normalizedPath;
        emit pathChanged(m_path);
    }

    emit textChanged();
    m_lastSavedAtUtc = QFileInfo(m_path).lastModified().toUTC();
    setDirty(false);
    return true;
}

bool Document::save()
{
    if (m_path.trimmed().isEmpty()) {
        return false;
    }

    QSaveFile file(m_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    file.write(normalizeForStoredLineEnding(m_text).toUtf8());
    if (!file.commit()) {
        return false;
    }

    m_lastSavedAtUtc = QFileInfo(m_path).lastModified().toUTC();
    setDirty(false);
    return true;
}

bool Document::saveAs(const QString &path)
{
    const QString oldPath = m_path;
    m_path = QFileInfo(path).absoluteFilePath();
    const bool saved = save();
    if (!saved) {
        m_path = oldPath;
        return false;
    }

    emit pathChanged(m_path);
    return true;
}

void Document::setText(const QString &text)
{
    if (m_text == text) {
        return;
    }

    m_text = text;
    emit textChanged();
    setDirty(true);
}

void Document::setDirty(bool dirty)
{
    if (m_dirty == dirty) {
        return;
    }

    m_dirty = dirty;
    emit dirtyChanged(m_dirty);
}

QString Document::normalizeForStoredLineEnding(const QString &text) const
{
    QString normalized = text;
    normalized.replace("\r\n", "\n");
    normalized.replace('\r', '\n');
    if (m_lineEnding == "\r\n") {
        normalized.replace("\n", "\r\n");
    }
    return normalized;
}

}
