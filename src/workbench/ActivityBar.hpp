#pragma once

#include <QString>
#include <QWidget>

class QButtonGroup;

namespace MalloyWriter::Workbench {

// The 48px vertical activity bar: switches the primary sidebar view and exposes
// the accounts/settings actions. Mirrors MalloyIDE/ActivityBar.jsx.
class ActivityBar : public QWidget {
    Q_OBJECT

public:
    explicit ActivityBar(QWidget *parent = nullptr);

    QString currentView() const { return m_currentView; }

public slots:
    void setCurrentView(const QString &id);

signals:
    void viewChanged(const QString &id);
    void settingsRequested();

private:
    QButtonGroup *m_group = nullptr;
    QString m_currentView;
};

} // namespace MalloyWriter::Workbench
