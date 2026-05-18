# MalloyWriter

MalloyWriter is a native C++/Qt IDE project inspired by the architectural shape of VS Code: a layered core, a workbench shell, command-driven UI, and future protocol boundaries for LSP and DAP.

This first slice is intentionally practical: open a folder, browse files, edit and save documents, run CMake configure/build/test commands, stream process output, parse diagnostics, and use a command palette.

## Build

On this machine the intended toolchain is MSYS2 MinGW Qt 6:

```powershell
.\build.ps1 -RunTests
```

The equivalent manual configure command is:

```powershell
& 'C:\msys64\mingw64\bin\qt-cmake.bat' -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_MAKE_PROGRAM='C:\ProgramData\chocolatey\bin\ninja.exe' `
  -DCMAKE_C_COMPILER='C:\msys64\mingw64\bin\gcc.exe' `
  -DCMAKE_CXX_COMPILER='C:\msys64\mingw64\bin\g++.exe'
cmake --build build
ctest --test-dir build --output-on-failure
```

## Current Features

- Qt Widgets desktop shell with docked project explorer and output panel.
- Tabbed editor with line numbers, current-line highlight, find/replace, go-to-line, and diagnostic gutter markers.
- Dirty-state tracking, save conflict checks, save current, save all, recent files, and recent workspaces.
- Command registry and searchable command palette.
- CMake configure/build/test/run process execution with live output streaming.
- CMake File API target parsing.
- JSON-RPC transport for `clangd` LSP and DAP-shaped debugging boundaries.
- Disabled-by-default assistant provider boundary.

## Production Docs

- [Architecture](docs/ARCHITECTURE.md)
- [Roadmap](docs/ROADMAP.md)
- [Release checklist](docs/RELEASE_CHECKLIST.md)
- [AI assistant notes](docs/AI_ASSISTANT.md)

## Packaging

```powershell
.\scripts\package-windows.ps1 -Clean
```

This builds a release binary, runs tests, deploys Qt runtime files with `windeployqt6`, and writes `dist\MalloyWriter-win64.zip`.
