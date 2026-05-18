#pragma once

#include <QObject>
#include <QDateTime>
#include <QString>

namespace MalloyWriter::Editor {

class Document : public QObject {
    Q_OBJECT

public:
    explicit Document(QObject *parent = nullptr);

    QString path() const;
    QString fileName() const;
    QString text() const;
    bool isDirty() const;
    bool hasExternalModification() const;
    QString lineEnding() const;
    QDateTime lastSavedAtUtc() const;

    bool load(const QString &path);
    bool save();
    bool saveAs(const QString &path);
    void setText(const QString &text);

signals:
    void pathChanged(const QString &path);
    void textChanged();
    void dirtyChanged(bool dirty);

private:
    void setDirty(bool dirty);
    QString normalizeForStoredLineEnding(const QString &text) const;

    QString m_path;
    QString m_text;
    QString m_lineEnding = "\n";
    QDateTime m_lastSavedAtUtc;
    bool m_dirty = false;
};

}
