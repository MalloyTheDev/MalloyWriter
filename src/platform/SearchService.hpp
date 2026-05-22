#pragma once

#include <QFutureWatcher>
#include <QList>
#include <QObject>
#include <QString>

namespace MalloyWriter::Platform {

struct SearchHit {
    int line = 0;
    int column = 0;
    QString lineText;
    int matchStart = 0;
    int matchLength = 0;
};

struct FileMatches {
    QString path;
    QString relativePath;
    QList<SearchHit> hits;
};

struct SearchQuery {
    QString text;
    bool caseSensitive = false;
    bool wholeWord = false;
    bool regex = false;
};

// Recursive workspace text search. Runs off the UI thread via QtConcurrent and
// reports results back on the main thread. Bounded (skips VCS/build dirs, large
// and binary files; caps total hits) so it stays responsive on big trees.
class SearchService : public QObject {
    Q_OBJECT

public:
    explicit SearchService(QObject *parent = nullptr);

    void search(const QString &root, const SearchQuery &query);
    bool isRunning() const;

    // Pure scan, exposed for testing.
    static QList<FileMatches> scan(const QString &root, const SearchQuery &query);

signals:
    void resultsReady(const QList<MalloyWriter::Platform::FileMatches> &results, int fileCount, int hitCount);

private:
    QFutureWatcher<QList<FileMatches>> m_watcher;
};

} // namespace MalloyWriter::Platform
