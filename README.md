# Ursa 贪吃蛇 Demo

Demo 只包含贪吃蛇场景及其 C# 规则、输入和编辑器工具。运行时使用引擎默认 Forward 管线。

## 资源结构

- `Assets/Snake/Snake.ursascene`：游戏相机、控制器、食物和预先保存的蛇身对象。
- `Assets/Snake/Board.ursaprefab`：底座和 16×16 个棋盘格，直接保存在 Prefab 中。
- `Assets/Scripts/Snake/`：游戏规则、固定步长计时和已有对象的显示更新。
- `Assets/Scripts/Editor/SnakeEditor.cs`：控制器 Inspector、保存菜单和操作说明窗口。

打开场景即可看到完整棋盘、初始蛇身和食物，无需进入 Run。场景预存 256 个蛇身对象，初始启用 3 个；成长、移动和重启只改变已有对象的位置及显隐。脚本不创建棋盘、网格、材质或游戏对象。棋盘外观可直接编辑 Prefab；蛇身和食物的外观保存在场景本地材质中，几何共享引擎内置 Cube。

场景、Prefab 及现有脚本的资产身份保持稳定。`Library/` 是可重建缓存，不属于源资源。

## 构建与运行

从工作区根目录执行：

```powershell
cmake -S Engine -B Engine/Build -DURSA_ENABLE_DOTNET=ON -DURSA_PROJECT_DIR=Demo
cmake --build Engine/Build --parallel --config Release --target UrsaEditor UrsaScriptPlayer UrsaAssetImport UrsaPak
Engine/Build/Editor/Release/UrsaEditor.exe --project Demo --scene Snake/Snake.ursascene
```

独立运行前显式导入：

```powershell
Engine/Build/Editor/Release/UrsaAssetImport.exe --project Demo --configuration Release
Engine/Build/Release/UrsaScriptPlayer.exe --project Demo --scene Snake/Snake.ursascene
```

方向键或 WASD 转向，`P` 暂停/继续，空格重新开始。Editor 的 Run/Stop 使用独立运行副本，停止后保留原始场景。

打包及运行：

```powershell
Engine/Build/Release/UrsaPak.exe --project-root Demo --configuration Release --output Demo/Library/Content.pak
Engine/Build/Release/UrsaScriptPlayer.exe --pak Demo/Library/Content.pak --scene Snake/Snake.ursascene
```

需要 Visual Studio 2022、CMake、Vulkan SDK、.NET 10 SDK/runtime 和已初始化的引擎依赖。普通 C++ 构建不执行项目资源导入。

## 验证

```powershell
ctest --test-dir Engine/Build -C Release --output-on-failure -R 'Ursa(SnakeManagedLogic|ProjectSnake|ProjectScenePrefabRoundTrip|ProjectPlaySession|ProjectEditorExtensions)'
```

材质、纹理、模型、脚本 API 和渲染管线展示资源由 Engine 的 `Tests/Fixtures/SampleAssets` 保存，仅在测试项目中叠加，不进入 Demo。
