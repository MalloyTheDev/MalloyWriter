#include "platform/SearchService.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using MalloyWriter::Platform::SearchService;
using MalloyWriter::Platform::SearchQuery;

class SearchServiceTests : public QObject {
    Q_OBJECT

private:
    static void writeFile(const QString &dir, const QString &name, const QByteArray &content)
    {
        QFile file(QDir(dir).filePath(name));
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }
        file.write(content);
        file.close();
    }

    static int totalHits(const QList<MalloyWriter::Platform::FileMatches> &results)
    {
        int hits = 0;
        for (const auto &file : results) {
            hits += file.hits.size();
        }
        return hits;
    }

private slots:
    void findsCaseInsensitiveMatchesGroupedByFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        writeFile(dir.path(), "a.cpp", "int Setup();\nvoid setup() {}\n");
        writeFile(dir.path(), "b.txt", "nothing relevant here\n");

        SearchQuery query;
        query.text = "setup";
        const auto results = SearchService::scan(dir.path(), query);
        QCOMPARE(results.size(), 1);            // only a.cpp matches
        QCOMPARE(totalHits(results), 2);        // "Setup" and "setup"
    }

    void caseSensitiveNarrowsMatches()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        writeFile(dir.path(), "a.cpp", "Setup setup SETUP\n");

        SearchQuery query;
        query.text = "setup";
        query.caseSensitive = true;
        QCOMPARE(totalHits(SearchService::scan(dir.path(), query)), 1);
    }

    void wholeWordExcludesSubstrings()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        writeFile(dir.path(), "a.cpp", "set setup setting set\n");

        SearchQuery query;
        query.text = "set";
        query.wholeWord = true;
        QCOMPARE(totalHits(SearchService::scan(dir.path(), query)), 2); // two standalone "set"
    }

    void skipsBinaryFilesAndEmptyQuery()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        writeFile(dir.path(), "bin.dat", QByteArray("text\0more", 9));

        SearchQuery binaryQuery;
        binaryQuery.text = "text";
        QVERIFY(SearchService::scan(dir.path(), binaryQuery).isEmpty());

        SearchQuery emptyQuery;
        QVERIFY(SearchService::scan(dir.path(), emptyQuery).isEmpty());
    }
};

QTEST_MAIN(SearchServiceTests)

#include "search_service_tests.moc"
