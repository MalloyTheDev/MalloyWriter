#include "workbench/OutputPanel.hpp"

#include <QPlainTextEdit>
#include <QTextCursor>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

OutputPanel::OutputPanel(QWidget *parent)
    : QWidget(parent)
    , m_output(new QPlainTextEdit(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_output);

    m_output->setReadOnly(true);
    m_output->setLineWrapMode(QPlainTextEdit::NoWrap);
}

void OutputPanel::appendText(const QString &text)
{
    m_output->moveCursor(QTextCursor::End);
    m_output->insertPlainText(text);
    m_output->moveCursor(QTextCursor::End);
}

void OutputPanel::clear()
{
    m_output->clear();
}

}
