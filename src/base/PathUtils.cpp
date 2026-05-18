#include "base/PathUtils.hpp"

#include <QDir>
#include <QFileInfo>

namespace MalloyWriter::Base {

QString normalizePath(const QString &path)
{
    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

QString displayNameForPath(const QString &path)
{
    const QFileInfo info(path);
    return info.fileName().isEmpty() ? info.absoluteFilePath() : info.fileName();
}

QString compactPath(const QString &path, int maxCharacters)
{
    const QString normalized = normalizePath(path);
    if (normalized.size() <= maxCharacters || maxCharacters < 12) {
        return normalized;
    }

    const int tailLength = maxCharacters - 4;
    return QString(".../") + normalized.right(tailLength);
}

bool isLikelyTextFile(const QString &path)
{
    static const QStringList textSuffixes = {
        "c", "cc", "cpp", "cxx", "h", "hh", "hpp", "hxx",
        "cmake", "txt", "md", "json", "xml", "yaml", "yml",
        "ini", "toml", "py", "js", "ts", "css", "html", "qss"
    };

    const QFileInfo info(path);
    const QString suffix = info.suffix().toLower();
    return textSuffixes.contains(suffix) || info.fileName() == "CMakeLists.txt";
}

}
