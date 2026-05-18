#pragma once

#include <QString>

namespace MalloyWriter::Base {

void installMessageHandler();
QString logFilePath();
void logInfo(const QString &message);
void logWarning(const QString &message);

}
