# UrsaDemo

UrsaDemo is a standalone triangle application built against the sibling UrsaEngine repository. It links to `Ursa::Engine` and uses only the engine's public API.

## Requirements

- Windows 10 or newer, Visual Studio 2022 with C++ desktop tools, CMake 3.20 or newer.
- Vulkan SDK 1.2 or newer with `glslc`, and a Vulkan 1.2 capable GPU and driver.
- An initialized UrsaEngine checkout at `../Engine`, or an explicit `URSA_ENGINE_DIR` path. Initialize the engine's GLFW and VMA submodules before configuring Demo.

## Build and run

```powershell
git -C ..\Engine submodule update --init --recursive
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
& .\build\Debug\UrsaTriangle.exe
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Use `-DURSA_ENGINE_DIR="C:/path/to/Engine"` at configure time if the engine checkout is elsewhere. The window smoke test renders 20 hidden frames and exercises resize, minimize, and restore. Press Escape or close the visible window to quit the application.

The engine and Demo are separate Git repositories. Demo does not record an engine commit; configure it with a compatible UrsaEngine checkout. Follow the engine repository's `.clang-format` rules when editing C++ code.
