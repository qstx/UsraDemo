# UrsaDemo

UrsaDemo contains a playable 3D Snake game and a material showcase. Both build against the sibling UrsaEngine repository, link to `Ursa::Engine`, and use its public API.

First-party C++ follows the repository `.clang-format` and `.editorconfig` rules, shared with Engine. Run `Tools/Format.ps1` to format Demo sources or `Tools/Format.ps1 -Check` to verify them; pass `-ClangFormatPath` if needed. The conventions and compatibility boundary are documented in Engine's `CODING_STYLE.md`.

From a configured Demo build, `cmake --build build --config Debug --target UrsaFormatCheck` checks the Engine and Demo C++ sources without changing them.

## Source layout

`Snake/` contains the game entry point, with rules and scene code in `Snake/Game/` and `Snake/Scene/`; `Material/` contains the material showcase entry point. Each header sits beside its source file. Tests are grouped by feature under `Tests/`.

`Assets/` is the writable project asset root for the Editor, and `--project` names the project directory that contains it. From the Demo repository, run `& .\build\Engine\Editor\Debug\UrsaEditor.exe --project .` to browse and save Demo resources while viewing Engine built-ins as read-only. For the playable Snake scene, run `& .\build\Debug\UrsaDemoEditor.exe --project . --scene Snake/Snake.ursascene`; this launcher registers Snake component codecs and opens the scene directly. Both launchers require `--project` at startup. Their docked workspace shows Hierarchy, a scene viewport, Inspector, and Project; use the top Run/Stop control when editing a scene.

CMake discovers source files within these production directories and checks for additions or removals during a build. New test executables still need a CMake target.

## Requirements

- Windows 10 or newer, Visual Studio 2022 with C++ desktop tools, CMake 3.20 or newer.
- Vulkan SDK 1.2 or newer with `glslc`, and a Vulkan 1.2 capable GPU and driver.
- An initialized UrsaEngine checkout at `../Engine`, or an explicit `URSA_ENGINE_DIR` path. Initialize the engine's GLFW, VMA, JSON, EASTL, EABase, and spdlog submodules before configuring Demo.

## Build and run

```powershell
git -C ..\Engine submodule update --init --recursive
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DFETCHCONTENT_FULLY_DISCONNECTED=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
& .\build\Debug\UrsaSnake.exe
& .\build\Debug\UrsaMaterialShowcase.exe
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Use `-DURSA_ENGINE_DIR="C:/path/to/Engine"` at configure time if the engine checkout is elsewhere. `UrsaSnakeWindowSmoke` renders 20 hidden frames, exercises a scripted food pickup, loss and restart, and resizes, minimizes and restores the window. `UrsaMaterialWindowSmoke` renders 20 hidden showcase frames. `UrsaSnakeTests` checks the game rules without a GPU.

## Logs

Rotating logs use `user:Logs/<executable>.log`, under `%LOCALAPPDATA%/<executable>/`
on Windows. Settings and save games use the writable `user:` mount in every mode.
Content mounts remain read only in runtime and packaged launches.

## Material showcase

Run `UrsaMaterialShowcase` to see three boxes: a programmatically generated Unlit checker, a blue rough PBR surface with a generated normal map, and a smoother metallic gold surface. A directional light, warm point light, and adjustable ambient fill are scene components. The floor uses the old box-color path, so the showcase exercises both paths together. The window closes normally; `--hidden --frames 20` runs a short smoke test. No image files are loaded. Material and lighting API details are in the Engine README.

## 3D Snake

`UrsaSnake` is a Player: it loads resources from `Assets/` under its working directory and does not accept `--project`. Set the working directory to the Demo repository, then run `& .\build\Debug\UrsaSnake.exe` without arguments. Visual Studio, the VS Code Snake launch configuration, and the three Snake window smoke tests use this working directory. A missing assets directory produces a resource loading error; the Player does not search other directories. Editor launchers still require `--project`.

CMake Tools quick debugging uses the Demo working directory with no arguments. To debug an Editor in VS Code, select `Debug UrsaEditor` or `Debug UrsaDemoEditor` from `launch.json`; those configurations supply the project path and, for the Demo Editor, the initial scene.

The snake automatically moves across a 16-by-16 board at a fixed simulation interval. Turn with the arrow keys or WASD. Direct reversal is ignored, and one turn is accepted per grid step. Eating orange food grows the snake and increases the score by one. Running into the board edge or the snake's body ends the round. Filling every cell wins. Press Space to restart at any time, or Escape to close the window.

The board is stored in `Assets/Snake/Board.ursaprefab`; `Assets/Snake/Snake.ursascene` references it and stores the camera and gameplay components. The normal `UrsaSnake` launch loads these files. Food and snake segments are generated as colored boxes by the presentation component and shown by the saved camera. The current score, result and controls appear in the window title. The game has no external model or texture images.

`SnakeApp` loads a linked Engine `Scene` during normal startup. A root GameObject owns the `SnakeController` and `SnakePresentation` behavior components. The board base and tiles sit under an instanced board Prefab; separate GameObjects hold the camera, food, and each snake segment. `SnakeController` reads the frame's input snapshot and advances the pure `SnakeGame` rules from fixed updates at the game's 0.18-second move interval. `SnakePresentation` runs after fixed updates, creates or removes segment and food GameObjects, updates their transforms, and refreshes the window title. The Engine gathers their active `BoxRenderComponent` instances and renders through the selected `CameraComponent`. `UrsaSnakeAssetTests` checks the committed scene and Prefab link; `UrsaSnakeAssetWindowSmoke` loads and renders them. The stress/gameplay smoke path still constructs a small procedural board.

## Serialized Snake components

`Snake/Scene/SnakeSerialization.cpp` registers `demo.SnakeController` and `demo.SnakePresentation` with stable version 1 names. Their saved fields include controller options and the presentation root's persistent GameObject ID. The live `SnakeGame&` is supplied again at load time and is never written to the scene file:

```cpp
#include "SnakeSerialization.hpp"
#include <Serialization/SceneResource.hpp>

void LoadSnake(const Ursa::VirtualPath &file, Ursa::ResourceSession &resources,
               Ursa::Demo::SnakeGame &game) {
    Ursa::ComponentRegistry registry;
    if (!Ursa::RegisterBuiltinComponents(registry) ||
        !Ursa::Demo::RegisterSnakeComponents(registry))
        return;
    Ursa::ComponentLoadContext context(resources);
    context.Provide(game);
    auto scene = Ursa::LoadSceneResource(resources.Index().Filesystem(), file, registry, context);
}
```

Keep the game object alive for the loaded scene's lifetime. Register codecs before loading; an unknown component version fails with its file and node location. `UrsaSnakeSerializationTests` compiles this pattern, checks a scene round trip, advances the rebound game, and covers cleanup and missing-dependency errors.

Run `UrsaSnake --serialized` to explicitly reload the committed linked scene before play. With `--smoke-gameplay`, the demo saves a small generated scene to a private memory VFS and reloads it; `UrsaSnakeSerializedWindowSmoke` checks that path through a hidden Vulkan window. To regenerate the initial scene and board Prefab from the procedural setup, build `UrsaSnakeAssetBake` and run `& .\build\Debug\UrsaSnakeAssetBake.exe . --force`. This replaces the two files, including Editor changes, so use it only when intentionally rebaking the initial assets.

## Demo Editor components and Play mode

Use `UrsaDemoEditor` when opening scenes containing Snake components. Its launcher registers the Snake codecs and property editors, provides a long-lived `SnakeGame` for edit-time decoding, and creates a fresh `SnakeGame` for each Run. The controller inspector edits `stressWindow` and `smokeGameplay`; the presentation inspector edits `smokeGameplay` and its root-node reference. Save and reopen the scene to verify the fields. To add another project component, implement Serializable and register its factory in `SnakeSerialization.cpp`, register a field editor like `SnakeEditors.cpp`, and provide any load or play services in `Editor/Main.cpp`. A component with a codec but without a property editor still round-trips unchanged.

With a scene open, click **Run** in the top bar to simulate the current in-memory scene and focus the viewport image for game keys. Click **Stop** to return to the exact edit scene. Run does not save files; saving is disabled until Stop. A scene with no active game camera still edits with the editor camera, but the play viewport reports that it needs a game camera. `UrsaSnakeEditorTests` exercises Snake fields after save/reopen, and the Engine `UrsaPlaySessionTests` exercises isolated Run/Stop.

## Reusing a Prefab

A board tile or other subtree can be saved once as a PrefabAsset with `SavePrefabResource` and placed in several scenes with `AddPrefabInstance`. A placement keeps its source ID and its own instance ID. After loading a linked scene, this sequence overrides a tile's X position, saves it, and later removes the override so the next explicit reload inherits the current source:

```cpp
#include <Serialization/PrefabResource.hpp>

void moveTile(Ursa::LinkedSceneResource &level, Ursa::VirtualFileSystem &filesystem, const Ursa::VirtualPath &scenePath,
              Ursa::ComponentRegistry &registry, Ursa::ComponentLoadContext &context,
              Ursa::ResourceId instanceId, Ursa::ResourceId tileGameObjectId) {
    if (!Ursa::SetPrefabPropertyOverride(level, instanceId, tileGameObjectId,
                                          "/localTransform/position/0", 3.0f) ||
        !Ursa::SaveLinkedSceneResource(filesystem, scenePath, level, registry))
        return;
    if (!Ursa::ReloadLinkedSceneResource(filesystem, scenePath, registry, context, level))
        return;
    if (!Ursa::ClearPrefabPropertyOverride(level, instanceId, tileGameObjectId,
                                            "/localTransform/position/0") ||
        !Ursa::SaveLinkedSceneResource(filesystem, scenePath, level, registry))
        return;
    Ursa::ReloadLinkedSceneResource(filesystem, scenePath, registry, context, level);
}
```

The context's `ResourceSession` must include both Engine and Demo resource roots. For changed material files, create a refreshed session and load a candidate before replacing the running scene. Structural overrides use `SetPrefabStructureOverride`; see the Engine README and `UrsaPrefabTests` for add, delete, reparent, owner, and order examples.

The engine and Demo are separate Git repositories. Demo does not record an engine commit; configure it with a compatible UrsaEngine checkout. Follow the engine repository's `.clang-format` rules when editing C++ code.

## Loose and packaged content

From the workspace root:

```powershell
cmake --build Demo/build --config Debug --target UrsaDemoPackage
Demo/build/Debug/UrsaSnake.exe --project Demo
Demo/build/Debug/UrsaSnake.exe --pak Demo/build/package/Content.pak
Demo/build/Debug/UrsaDemoEditor.exe --project Demo --scene Snake/Snake.ursascene
Demo/build/Debug/UrsaDemoEditor.exe --project Demo --pak Demo/build/package/Content.pak --scene Snake/Snake.ursascene
```

The package contains both `ursa:` engine content and `prj:` project content, including
generated shaders. `--pak` mounts it read only, while logs, settings and saves remain
under `user:`. Package-backed Editor documents can be viewed and run; save fails with
a read-only diagnostic. Loose Editor projects mount `prj:` writable.
See Engine's `Docs/Resources.md` for the VFS API and provider boundaries.
