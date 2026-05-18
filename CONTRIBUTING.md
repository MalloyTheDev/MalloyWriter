# Contributing To MalloyWriter

MalloyWriter is moving toward a production-grade native C++ IDE. Contributions should keep behavior testable through services before wiring UI.

## Development Loop

```powershell
.\build.ps1 -RunTests
```

Before a PR:

- Keep changes scoped to one subsystem.
- Add or update Qt Test coverage for service behavior.
- Smoke launch `build\MalloyWriter.exe`.
- Update docs or tracking files when workflows, dependencies, or release gates change.

## Architecture Rule

UI code should delegate durable behavior to services. New IDE features should prefer these boundaries:

- `ProjectService`
- `DocumentService`
- `BuildService`
- `LanguageService`
- `DebugService`
- `AssistantService`
