# Code style

- Follow `.clang-format` and `.editorconfig`, matching Engine's `CODING_STYLE.md`.
- Use clang-format 19. Run `Tools/Format.ps1 -Check` or build the `UrsaDemoFormatCheck` target after changing C++.
- Keep a blank line between method definitions. Keep opening braces on their own lines and braces around control-flow bodies.
- Use PascalCase for namespaces, types, enumeration values, and methods and camelCase for fields, parameters, and local variables, as specified in Engine's `CODING_STYLE.md`. Preserve serialized names and resource IDs when renaming C++ symbols.
- Do not format generated assets or build output.
