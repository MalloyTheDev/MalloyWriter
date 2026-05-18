# Security Policy

MalloyWriter is early-stage software. Treat every workspace and build command as untrusted input.

## Current Security Rules

- Build and run actions must remain explicit user commands.
- Assistant features are disabled by default.
- Future extension/plugin execution must be sandboxed or clearly permissioned before shipping.
- Do not add telemetry or network calls without explicit settings and documentation.

## Reporting

Use a private GitHub security advisory once the repository is published. Until then, report issues directly to the repository owner.
