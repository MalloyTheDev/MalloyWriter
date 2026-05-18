# MalloyWriter Roadmap

## Phase 1: Production Foundation

- GitHub repository, CI, issue templates, release checklist.
- Service-oriented core for project, documents, build jobs, diagnostics, language, debug, and assistant boundaries.
- Broader Qt Test coverage.

## Phase 2: Project And Build

- CMake File API target discovery.
- Target picker and build/test/run actions per target.
- Kit settings UI for GCC, clangd, Ninja, CMake, and GDB.

## Phase 3: Editor

- Find/replace, go-to-line, line numbers, diagnostics gutter.
- Save conflict handling and reload prompts.
- Better large-file behavior and encoding/newline preservation.

## Phase 4: C++ Intelligence

- Full clangd lifecycle.
- Hover, completion, symbols, references, rename, formatting.
- Diagnostics mapped into the editor and problems panel.

## Phase 5: Debugging

- DAP session lifecycle.
- Breakpoints, call stack, variables, threads, debug console.
- GDB-backed launch path on Windows/MSYS2.

## Phase 6: Assistant

- Optional provider settings.
- Local codebase context and preview-only edits.
- Model evaluation before enabling default workflows.

## Phase 7: Release

- Windows package with Qt runtime.
- Clean-machine smoke test.
- GitHub release artifacts.
