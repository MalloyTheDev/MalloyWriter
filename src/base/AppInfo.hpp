#pragma once

#include <QString>

namespace MalloyWriter::Base {

inline constexpr int AppVersionMajor = 0;
inline constexpr int AppVersionMinor = 2;
inline constexpr int AppVersionPatch = 0;

inline QString appVersion()
{
    return QString("%1.%2.%3").arg(AppVersionMajor).arg(AppVersionMinor).arg(AppVersionPatch);
}

namespace Commands {
inline const QString OpenFolder = "file.openFolder";
inline const QString OpenFile = "file.openFile";
inline const QString Save = "file.save";
inline const QString SaveAll = "file.saveAll";
inline const QString Exit = "file.exit";
inline const QString CommandPalette = "view.commandPalette";
inline const QString ClearOutput = "view.clearOutput";
inline const QString Find = "editor.find";
inline const QString Replace = "editor.replace";
inline const QString GoToLine = "editor.goToLine";
inline const QString Configure = "build.configure";
inline const QString Build = "build.build";
inline const QString Test = "build.test";
inline const QString RunExecutable = "build.runExecutable";
inline const QString Stop = "build.stop";
inline const QString RefreshProject = "project.refresh";
}

}
