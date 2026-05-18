#include "platform/CommandRegistry.hpp"

#include <algorithm>

namespace MalloyWriter::Platform {

CommandRegistry::CommandRegistry(QObject *parent)
    : QObject(parent)
{
}

bool CommandRegistry::registerCommand(Command command)
{
    if (command.id.trimmed().isEmpty() || command.title.trimmed().isEmpty()) {
        return false;
    }

    if (m_commands.contains(command.id)) {
        return false;
    }

    m_commands.insert(command.id, std::move(command));
    emit commandsChanged();
    return true;
}

bool CommandRegistry::contains(const QString &id) const
{
    return m_commands.contains(id);
}

bool CommandRegistry::execute(const QString &id) const
{
    const auto it = m_commands.constFind(id);
    if (it == m_commands.constEnd() || !it->handler) {
        return false;
    }

    it->handler();
    return true;
}

QList<Command> CommandRegistry::commands() const
{
    QList<Command> result = m_commands.values();
    std::sort(result.begin(), result.end(), [](const Command &left, const Command &right) {
        if (left.category == right.category) {
            return left.title.localeAwareCompare(right.title) < 0;
        }
        return left.category.localeAwareCompare(right.category) < 0;
    });
    return result;
}

Command CommandRegistry::command(const QString &id) const
{
    return m_commands.value(id);
}

}
