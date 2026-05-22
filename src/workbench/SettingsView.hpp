#pragma once

#include <QWidget>

class QStackedWidget;

namespace MalloyWriter::Workbench {

// The Settings screen (shown in the main area). The Appearance section is
// functional — theme variant + accent switch the live Base::Theme and re-apply
// immediately. Editor / Keyboard / MW Assist sections are representative.
class SettingsView : public QWidget {
    Q_OBJECT

public:
    explicit SettingsView(QWidget *parent = nullptr);

private:
    QWidget *buildAppearancePage();
    QWidget *buildEditorPage();
    QWidget *buildKeyboardPage();
    QWidget *buildAssistPage();
    void applyThemeChange();

    QStackedWidget *m_content = nullptr;
};

} // namespace MalloyWriter::Workbench
