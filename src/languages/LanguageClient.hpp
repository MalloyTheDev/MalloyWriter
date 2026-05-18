#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace MalloyWriter::Languages {

struct LanguageServerDefinition {
    QString id;
    QString displayName;
    QString executable;
    QStringList arguments;
};

class LanguageClient : public QObject {
    Q_OBJECT

public:
    explicit LanguageClient(QObject *parent = nullptr);

    virtual QString languageId() const = 0;
    virtual LanguageServerDefinition serverDefinition() const = 0;

signals:
    void diagnosticsChanged(const QString &uri);
    void serverStatusChanged(const QString &status);
};

}
