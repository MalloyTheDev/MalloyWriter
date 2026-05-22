#pragma once

#include <QDialog>

namespace MalloyWriter::Workbench {

// The MW Assist "Composer" — a centered modal overlay (Ask / Edit / Agent modes,
// prompt, context chips, quick prompts). Static until the AssistantService is
// wired; opened with Ctrl+K. Mirrors MalloyIDE/Composer.jsx.
class ComposerDialog : public QDialog {
    Q_OBJECT

public:
    explicit ComposerDialog(QWidget *parent = nullptr);
};

} // namespace MalloyWriter::Workbench
