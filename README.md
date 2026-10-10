# Ursa Demo 项目

Demo 是 UrsaEngine 的 C# 游戏项目，运行脚本、编辑器扩展和资源都放在项目目录中。引擎负责 C++ 运行时、Vulkan 渲染、通用 Editor、Script Player、资源导入和打包。

## 目录

- `Assets/Scripts/`：C# 游戏行为。
- `Assets/Scripts/Editor/`：自定义 Inspector、菜单和 Editor 窗口。
- `Assets/Snake/`：贪吃蛇 Scene 与可复用 Board Prefab。
- `Assets/MaterialShowcase.ursascene`：Unlit 与 PBR 材质展示场景。
- `Assets/Materials/`、`Assets/ScriptSample/`：材质、纹理和脚本示例资源。
- `Tests/`：仅 C# 逻辑测试；引擎集成测试由 Engine 的 `URSA_PROJECT_DIR` 配置驱动。

项目源码不需要 CMakeLists.txt，也不需要项目自有 C++ launcher、组件注册回调或资产烘焙程序。脚本生成的项目、程序集和导入缓存保存在 `Library/`，不放入 `Assets/`。

内置与项目资源统一由 Editor 或无界面导入工具准备，产物及 Shader 缓存均归属 Demo/Library。普通程序构建不准备资源。流程与恢复说明见 [项目资源导入管线](../Engine/Docs/AssetImportPipeline.md)。

## 构建与测试

从工作区根目录运行：

```powershell
cmake -S Engine -B Engine/Build -DURSA_ENABLE_DOTNET=ON -DURSA_PROJECT_DIR=Demo
cmake --build Engine/Build --parallel --config Debug --target UrsaEditor UrsaScriptPlayer UrsaAssetImport UrsaPak
ctest --test-dir Engine/Build -C Debug --output-on-failure -R 'Ursa(Project|Snake)'
```

需要 Visual Studio 2022、CMake、Vulkan SDK、`.NET 10` SDK/runtime，以及已初始化的 Engine 子模块。Release 构建将 `Debug` 换成 `Release`。

## 运行

通用 Editor 打开 Snake Scene：

```powershell
Engine/Build/Editor/Debug/UrsaEditor.exe --project Demo --scene Snake/Snake.ursascene
```

独立运行 Snake 或材质展示：

```powershell
Engine/Build/Debug/UrsaScriptPlayer.exe --project Demo --scene Snake/Snake.ursascene
Engine/Build/Debug/UrsaScriptPlayer.exe --project Demo --scene MaterialShowcase.ursascene
```

Snake 使用方向键或 WASD 转向，`P` 暂停/继续，空格重新开始。Script Player 也支持 `--pak <Content.pak>`；Engine 的 `UrsaPak` 会把项目 C# 程序集、资源和 Engine 内容打包到同一个内容包中。

所有项目路径都通过 `--project` 指定。运行 Editor 时，先在 Project 面板打开 Scene，点击 Run/Stop 可在临时运行场景与编辑场景之间切换。

启动 Player 前执行 `Engine/Build/Editor/Debug/UrsaAssetImport.exe --project Demo --configuration Debug`；Editor 的 `--project Demo` 会自动准备资源。内容构建使用 `UrsaPak --project-root Demo --configuration Debug --output Demo/Build/Content.pak`。
