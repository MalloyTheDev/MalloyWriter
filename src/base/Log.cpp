#include "base/Log.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QStandardPaths>
#include <QTextStream>
#include <QtGlobal>

namespace MalloyWriter::Base {
namespace {

QMutex &logMutex()
{
    static QMutex mutex;
    return mutex;
}

QString messageTypeName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return "debug";
    case QtInfoMsg:
        return "info";
    case QtWarningMsg:
        return "warning";
    case QtCriticalMsg:
        return "critical";
    case QtFatalMsg:
        return "fatal";
    }

    return "unknown";
}

void writeLogLine(const QString &level, const QString &message)
{
    const QMutexLocker locker(&logMutex());
    QFile file(logFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString(Qt::ISODate) << " ["
           << level << "] " << message << '\n';
}

void messageHandler(QtMsgType type, const QMessageLogContext &, const QString &message)
{
    writeLogLine(messageTypeName(type), message);
}

}

void installMessageHandler()
{
    const QFileInfo info(logFilePath());
    QDir().mkpath(info.absolutePath());
    qInstallMessageHandler(messageHandler);
}

QString logFilePath()
{
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return QDir(baseDir).filePath("MalloyWriter.log");
}

void logInfo(const QString &message)
{
    writeLogLine("info", message);
}

void logWarning(const QString &message)
{
    writeLogLine("warning", message);
}

}
