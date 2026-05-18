#pragma once

#include <QWidget>

class QPlainTextEdit;

namespace MalloyWriter::Workbench {

class OutputPanel : public QWidget {
    Q_OBJECT

public:
    explicit OutputPanel(QWidget *parent = nullptr);

public slots:
    void appendText(const QString &text);
    void clear();

private:
    QPlainTextEdit *m_output = nullptr;
};

}
