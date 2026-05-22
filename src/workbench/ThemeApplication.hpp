#pragma once

namespace MalloyWriter::Base {
class Theme;
}

namespace MalloyWriter::Workbench {

// Apply a theme to the running QApplication: Fusion style (once), a QPalette
// derived from the tokens, and the generated global style sheet. Idempotent —
// safe to call repeatedly for live theme switching from Settings.
void applyTheme(const MalloyWriter::Base::Theme &theme);

} // namespace MalloyWriter::Workbench
