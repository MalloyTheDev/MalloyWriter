#include "platform/SearchService.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QtConcurrent>

namespace MalloyWriter::Platform {

namespace {

constexpr int kMaxHits = 2000;
constexpr int kMaxFiles = 8000;
constexpr qint64 kMaxFileSize = 1024 * 1024; // 1 MiB

bool isWordChar(QChar c)
{
    return c.isLetterOrNumber() || c == QLatin1Char('_');
}

bool isWholeWordMatch(const QString &line, int start, int length)
{
    if (start > 0 && isWordChar(line.at(start - 1))) {
        return false;
    }
    const int end = start + length;
    if (end < line.size() && isWordChar(line.at(end))) {
        return false;
    }
    return true;
}

bool inSkippedDirectory(const QString &relativePath)
{
    static const QStringList skip = {
        ".git", "build", "build-release", "dist", "release", ".malloywriter",
        "node_modules", ".vs", ".cache", ".idea",
    };
    const QString guarded = QLatin1Char('/') + relativePath + QLatin1Char('/');
    for (const QString &dir : skip) {
        if (guarded.contains(QLatin1Char('/') + dir + QLatin1Char('/'))) {
            return true;
        }
    }
    return false;
}

} // namespace

SearchService::SearchService(QObject *parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFutureWatcher<QList<FileMatches>>::finished, this, [this]() {
        if (m_watcher.isCanceled()) {
            return;
        }
        const QList<FileMatches> results = m_watcher.result();
        int hitCount = 0;
        for (const FileMatches &file : results) {
            hitCount += file.hits.size();
        }
        emit resultsReady(results, static_cast<int>(results.size()), hitCount);
    });
}

bool SearchService::isRunning() const
{
    return m_watcher.isRunning();
}

void SearchService::search(const QString &root, const SearchQuery &query)
{
    m_watcher.setFuture(QtConcurrent::run(&SearchService::scan, root, query));
}

QList<FileMatches> SearchService::scan(const QString &root, const SearchQuery &query)
{
    QList<FileMatches> results;
    if (root.isEmpty() || query.text.isEmpty()) {
        return results;
    }

    QRegularExpression regex;
    if (query.regex) {
        QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
        if (!query.caseSensitive) {
            options |= QRegularExpression::CaseInsensitiveOption;
        }
        regex = QRegularExpression(query.text, options);
        if (!regex.isValid()) {
            return results;
        }
    }
    const Qt::CaseSensitivity caseSensitivity = query.caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

    const QDir rootDir(root);
    int hitCount = 0;
    int fileCount = 0;

    QDirIterator iterator(root, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (iterator.hasNext() && hitCount < kMaxHits) {
        const QString filePath = iterator.next();
        const QString relativePath = rootDir.relativeFilePath(filePath);
        if (inSkippedDirectory(relativePath)) {
            continue;
        }
        if (++fileCount > kMaxFiles) {
            break;
        }
        QFileInfo info(filePath);
        if (info.size() > kMaxFileSize) {
            continue;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        const QByteArray bytes = file.readAll();
        if (bytes.contains('\0')) {
            continue; // treat as binary
        }

        const QStringList lines = QString::fromUtf8(bytes).split(QLatin1Char('\n'));
        FileMatches fileMatches;
        fileMatches.path = filePath;
        fileMatches.relativePath = relativePath;

        for (int i = 0; i < lines.size() && hitCount < kMaxHits; ++i) {
            const QString &line = lines.at(i);
            if (query.regex) {
                auto matches = regex.globalMatch(line);
                while (matches.hasNext() && hitCount < kMaxHits) {
                    const auto match = matches.next();
                    if (match.capturedLength() == 0) {
                        break;
                    }
                    fileMatches.hits.append({i + 1, match.capturedStart() + 1, line,
                                             static_cast<int>(match.capturedStart()),
                                             static_cast<int>(match.capturedLength())});
                    ++hitCount;
                }
            } else {
                int from = 0;
                while (hitCount < kMaxHits) {
                    const int index = line.indexOf(query.text, from, caseSensitivity);
                    if (index < 0) {
                        break;
                    }
                    if (!query.wholeWord || isWholeWordMatch(line, index, query.text.size())) {
                        fileMatches.hits.append({i + 1, index + 1, line, index, query.text.size()});
                        ++hitCount;
                    }
                    from = index + query.text.size();
                }
            }
        }

        if (!fileMatches.hits.isEmpty()) {
            results.append(fileMatches);
        }
    }
    return results;
}

} // namespace MalloyWriter::Platform
