#pragma once

#include <QWidget>

class QHBoxLayout;

namespace MalloyWriter::Editor {

// The path breadcrumb row above the editor (MalloyIDE/CodeEditor.jsx Breadcrumbs).
// Shows the trailing folder segments + file with chevron separators. Symbol-level
// crumbs are deferred to a later phase.
class Breadcrumbs : public QWidget {
    Q_OBJECT

public:
    explicit Breadcrumbs(QWidget *parent = nullptr);

    void setFilePath(const QString &path);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QHBoxLayout *m_layout = nullptr;
};

} // namespace MalloyWriter::Editor
