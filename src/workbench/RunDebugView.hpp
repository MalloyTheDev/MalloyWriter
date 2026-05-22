#pragma once

#include <QStringList>
#include <QWidget>

class QComboBox;

namespace MalloyWriter::Workbench {

// The Run & Debug sidebar view. The launch target selector is populated from the
// real CMake model (ProjectService executable targets); the Variables / Watch /
// Call Stack / Breakpoints sections are static until the DAP debugging phase.
class RunDebugView : public QWidget {
    Q_OBJECT

public:
    explicit RunDebugView(QWidget *parent = nullptr);

    void setTargets(const QStringList &targets);

signals:
    void runRequested(const QString &target);

private:
    QComboBox *m_targets = nullptr;
};

} // namespace MalloyWriter::Workbench
