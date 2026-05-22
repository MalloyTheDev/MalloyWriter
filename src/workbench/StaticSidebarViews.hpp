#pragma once

#include <QWidget>

namespace MalloyWriter::Workbench {

// Prototype-faithful, statically-populated sidebar views for capabilities that
// have no backing service yet (a GitService, an extension host, and a live
// AssistantService arrive in later phases). They render the prototype's layout
// with representative data so the workbench feels complete.

class ScmView : public QWidget {
    Q_OBJECT
public:
    explicit ScmView(QWidget *parent = nullptr);
};

class ExtensionsView : public QWidget {
    Q_OBJECT
public:
    explicit ExtensionsView(QWidget *parent = nullptr);
};

class AssistView : public QWidget {
    Q_OBJECT
public:
    explicit AssistView(QWidget *parent = nullptr);
};

} // namespace MalloyWriter::Workbench
