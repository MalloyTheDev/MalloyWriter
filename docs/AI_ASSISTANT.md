# AI Assistant Notes

MalloyWriter has an `AssistantService` boundary, but assistant features are disabled by default. The IDE should earn reliable core project, build, document, and LSP data before any model is allowed to suggest edits.

## Candidate Model Research

Hugging Face search results during this implementation pass showed these candidate coder families:

- `Qwen/Qwen2.5-Coder-7B-Instruct`
- `Qwen/Qwen3-Coder-30B-A3B-Instruct`
- `Qwen/Qwen2.5-Coder-14B-Instruct-AWQ`

## Integration Rules

- No assistant network calls without explicit settings.
- Model providers must be swappable behind `AssistantService`.
- Suggestions must be previewable before modifying files.
- Project indexing should use parsed project and document data, not broad blind filesystem scraping.
