#pragma once

#include <QString>
#include <QStringList>

namespace MalloyWriter::Base {

QString normalizePath(const QString &path);
QString displayNameForPath(const QString &path);
QString compactPath(const QString &path, int maxCharacters = 72);
bool isLikelyTextFile(const QString &path);

}
