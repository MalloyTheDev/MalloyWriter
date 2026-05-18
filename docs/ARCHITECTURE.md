# MalloyWriter Architecture

MalloyWriter is a native C++/Qt IDE organized around testable services and a thin workbench shell.

## Layers

- `base`: app metadata, command IDs, logging, path helpers.
- `platform`: workspace/project state, toolchain kits, CMake File API, build jobs, diagnostics, documents, settings, process execution.
- `editor`: text editor widgets, document binding, line numbers, current-line highlighting, diagnostics gutter markers, find/replace/go-to-line.
- `languages`: JSON-RPC framing and `clangd` LSP transport.
- `debug`: DAP request/session boundary for future debugger panes.
- `assistant`: disabled-by-default AI provider boundary.
- `workbench`: Qt Widgets main window, docks, menus, command palette, explorer, output panel.

## Production Rules

- UI code must call services for durable behavior.
- Build/project state comes from CMake and CMake File API, not guessed paths.
- C++ intelligence comes from LSP/clangd.
- Debugging uses DAP-shaped requests even while the first backend is GDB-oriented.
- Assistant features stay optional and disabled until local project/index data is reliable.

## Current Service Contracts

- `ProjectService` owns the active workspace, kit, build directory, and parsed CMake model.
- `BuildService` owns configure/build/test/run jobs and compiler diagnostic parsing.
- `DocumentService` owns loaded document identity, save operations, and external modification checks.
- `JsonRpcProcessTransport` owns Content-Length framing for LSP/DAP-compatible processes.
- `AssistantService` stores provider settings without making network calls.

## Next Hardening Targets

- Replace remaining direct tab-close save paths with `DocumentService`.
- Add target selection UI from `ProjectService::cmakeModel`.
- Convert clangd diagnostics into `Platform::Diagnostic` and render them in the editor gutter.
- Add DAP panes for breakpoints, call stack, variables, and debug console.
