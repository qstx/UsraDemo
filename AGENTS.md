# Demo project guidance

- Treat this repository as a C# project and asset library. Runtime scripts and Editor extensions belong under `Assets/Scripts/` and `Assets/Scripts/Editor/`.
- Do not add project-owned C++ code, launchers, component registration callbacks, asset baker programs, or C++ test targets. Put reusable native infrastructure and project integration tests in Engine.
- Use the `Ursa.Runtime` and `Ursa.Editor` APIs for project behavior. Keep Vulkan and native RHI types inside Engine.
- Preserve `.meta` GUIDs and resource IDs when editing existing assets. Save Scene and Prefab files with the generic Editor serialization path.
- Keep script builds, generated projects, imported data, and test output outside `Assets/`, in `Library/` or Engine's build tree.
- Write project-facing documentation in Chinese.
