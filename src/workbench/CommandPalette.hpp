#pragma once

#include "platform/CommandRegistry.hpp"

#include <QDialog>

class QLineEdit;
class QListWidget;

namespace MalloyWriter::Workbench {

class CommandPalette : public QDialog {
    Q_OBJECT

public:
    explicit CommandPalette(MalloyWriter::Platform::CommandRegistry *registry, QWidget *parent = nullptr);

    void openPalette();

private:
    void refresh();
    void executeCurrent();

    MalloyWriter::Platform::CommandRegistry *m_registry = nullptr;
    QLineEdit *m_filter = nullptr;
    QListWidget *m_list = nullptr;
};

}
