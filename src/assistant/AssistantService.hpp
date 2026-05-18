#pragma once

#include <QObject>
#include <QString>

namespace MalloyWriter::Assistant {

enum class AssistantProvider {
    Disabled,
    HuggingFace,
    OpenAICompatible
};

struct AssistantSettings {
    AssistantProvider provider = AssistantProvider::Disabled;
    QString modelId;
    QString endpointUrl;
};

class AssistantService : public QObject {
    Q_OBJECT

public:
    explicit AssistantService(QObject *parent = nullptr);

    AssistantSettings settings() const;
    void setSettings(const AssistantSettings &settings);
    bool isEnabled() const;

signals:
    void settingsChanged();
    void statusChanged(const QString &status);

private:
    AssistantSettings m_settings;
};

}
