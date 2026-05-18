#pragma once

#include <QHash>
#include <QKeySequence>
#include <QList>
#include <QObject>
#include <QString>

#include <functional>

namespace MalloyWriter::Platform {

struct Command {
    QString id;
    QString title;
    QString category;
    QKeySequence shortcut;
    std::function<void()> handler;
};

class CommandRegistry : public QObject {
    Q_OBJECT

public:
    explicit CommandRegistry(QObject *parent = nullptr);

    bool registerCommand(Command command);
    bool contains(const QString &id) const;
    bool execute(const QString &id) const;
    QList<Command> commands() const;
    Command command(const QString &id) const;

signals:
    void commandsChanged();

private:
    QHash<QString, Command> m_commands;
};

}
