# Release Checklist

## Local Verification

- [ ] `.\build.ps1 -Clean -RunTests`
- [ ] GUI smoke launch from `build\MalloyWriter.exe`
- [ ] Open a CMake workspace
- [ ] Configure, build, test, and run a target
- [ ] Confirm compiler diagnostics appear in the output panel and editor gutter
- [ ] Confirm recent workspace/file persistence

## Packaging

- [ ] `.\scripts\package-windows.ps1 -Clean`
- [ ] Confirm `dist\MalloyWriter-win64.zip` exists
- [ ] Extract package outside the build tree
- [ ] Launch packaged `MalloyWriter.exe`
- [ ] Confirm Qt `platforms\qwindows.dll` is present

## GitHub

- [ ] CI passes on `main`
- [ ] Draft release includes changelog, known issues, and package artifact
- [ ] Milestone and QA tracker updated
