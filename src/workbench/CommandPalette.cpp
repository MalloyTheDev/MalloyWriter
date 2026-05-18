#include "workbench/CommandPalette.hpp"

#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

namespace {

QString commandSearchText(const MalloyWriter::Platform::Command &command)
{
    return QString("%1 %2 %3").arg(command.category, command.title, command.id);
}

}

CommandPalette::CommandPalette(MalloyWriter::Platform::CommandRegistry *registry, QWidget *parent)
    : QDialog(parent)
    , m_registry(registry)
    , m_filter(new QLineEdit(this))
    , m_list(new QListWidget(this))
{
    setWindowTitle(tr("Command Palette"));
    resize(620, 420);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_filter);
    layout->addWidget(m_list);

    m_filter->setPlaceholderText(tr("Type a command"));

    connect(m_filter, &QLineEdit::textChanged, this, &CommandPalette::refresh);
    connect(m_filter, &QLineEdit::returnPressed, this, &CommandPalette::executeCurrent);
    connect(m_list, &QListWidget::itemActivated, this, &CommandPalette::executeCurrent);

    if (m_registry) {
        connect(m_registry, &MalloyWriter::Platform::CommandRegistry::commandsChanged, this, &CommandPalette::refresh);
    }
}

void CommandPalette::openPalette()
{
    m_filter->clear();
    refresh();
    show();
    raise();
    activateWindow();
    m_filter->setFocus();
}

void CommandPalette::refresh()
{
    m_list->clear();
    if (!m_registry) {
        return;
    }

    const QString filter = m_filter->text().trimmed();
    for (const auto &command : m_registry->commands()) {
        if (!filter.isEmpty() && !commandSearchText(command).contains(filter, Qt::CaseInsensitive)) {
            continue;
        }

        auto *item = new QListWidgetItem(QString("%1: %2").arg(command.category, command.title), m_list);
        item->setData(Qt::UserRole, command.id);
    }

    if (m_list->count() > 0) {
        m_list->setCurrentRow(0);
    }
}

void CommandPalette::executeCurrent()
{
    if (!m_registry || !m_list->currentItem()) {
        return;
    }

    const QString commandId = m_list->currentItem()->data(Qt::UserRole).toString();
    accept();
    m_registry->execute(commandId);
}

}
