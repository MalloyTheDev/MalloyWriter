#include "assistant/AssistantService.hpp"

namespace MalloyWriter::Assistant {

AssistantService::AssistantService(QObject *parent)
    : QObject(parent)
{
}

AssistantSettings AssistantService::settings() const
{
    return m_settings;
}

void AssistantService::setSettings(const AssistantSettings &settings)
{
    m_settings = settings;
    emit settingsChanged();
    emit statusChanged(isEnabled() ? "Assistant provider configured" : "Assistant disabled");
}

bool AssistantService::isEnabled() const
{
    return m_settings.provider != AssistantProvider::Disabled;
}

}
