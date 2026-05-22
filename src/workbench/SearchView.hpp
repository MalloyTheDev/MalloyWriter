#pragma once

#include "platform/SearchService.hpp"

#include <QWidget>

class QLabel;
class QLineEdit;
class QToolButton;
class QTreeWidget;

namespace MalloyWriter::Workbench {

// The Search sidebar view: query input + case/word/regex toggles over a real
// recursive workspace search (Platform::SearchService), with results grouped by
// file. Activating a hit asks the host to open the file at that line.
class SearchView : public QWidget {
    Q_OBJECT

public:
    explicit SearchView(QWidget *parent = nullptr);

    void setWorkspaceRoot(const QString &root);

signals:
    void openRequested(const QString &path, int line, int column);

private:
    void runSearch();
    void showResults(const QList<MalloyWriter::Platform::FileMatches> &results, int fileCount, int hitCount);

    QString m_root;
    MalloyWriter::Platform::SearchService *m_service = nullptr;
    QLineEdit *m_query = nullptr;
    QToolButton *m_caseButton = nullptr;
    QToolButton *m_wordButton = nullptr;
    QToolButton *m_regexButton = nullptr;
    QLabel *m_summary = nullptr;
    QTreeWidget *m_results = nullptr;
};

} // namespace MalloyWriter::Workbench
