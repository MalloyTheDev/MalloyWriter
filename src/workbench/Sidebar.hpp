#pragma once

#include <QHash>
#include <QString>
#include <QWidget>

class QHBoxLayout;
class QLabel;
class QStackedWidget;

namespace MalloyWriter::Workbench {

// The primary sidebar: a 280px panel with an uppercase header (title + per-view
// action buttons) and a stacked body of views keyed off the activity bar.
// Mirrors MalloyIDE/Sidebar.jsx (rich view content arrives in Phase 3).
class Sidebar : public QWidget {
    Q_OBJECT

public:
    explicit Sidebar(QWidget *parent = nullptr);

    // Replace a view's placeholder with a real widget (e.g. the ProjectExplorer).
    void setViewWidget(const QString &id, QWidget *widget);

public slots:
    void setView(const QString &id);

signals:
    void settingsRequested();

private:
    void rebuildHeaderActions(const QString &id);

    QStackedWidget *m_stack = nullptr;
    QLabel *m_title = nullptr;
    QHBoxLayout *m_actions = nullptr;
    QHash<QString, int> m_pageIndex;
};

} // namespace MalloyWriter::Workbench
