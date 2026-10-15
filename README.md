# Ursa Demo 项目

Demo 是 UrsaEngine 的 C# 游戏项目，运行脚本、编辑器扩展和资源都放在项目目录中。引擎负责 C++ 运行时、Vulkan 渲染、通用 Editor、Script Player、资源导入和打包。

## 目录

- `Assets/Scripts/`：C# 游戏行为。
- `Assets/Scripts/Editor/`：自定义 Inspector、菜单和 Editor 窗口。
- `Assets/Snake/`：贪吃蛇 Scene 与可复用 Board Prefab。
- `Assets/MaterialShowcase.ursascene`：Unlit 与 PBR 材质展示场景。
- `Assets/Materials/`、`Assets/ScriptSample/`：材质、纹理和脚本示例资源。
- `Assets/Rendering/`、`Assets/Scripts/Rendering/`：项目 C# 管线资产、离屏/全屏合成、计算纹理、替换材质与多相机示例。
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

## 选择与编写渲染管线

Demo 默认使用框架 Forward。Editor 中打开 `Window > Project Graphics Settings`，选择 `Assets/Rendering/ShowcasePipeline.ursaasset` 并 Apply，Scene、Game、Material Preview 与 Player 将使用项目离屏合成管线。绿色边框及 `tint` 色调来自项目 Composite Shader。选择 `ComputeShowcasePipeline.ursaasset` 可以观察计算生成图案及 RendererList 替换材质；选择 None 并 Apply 恢复默认 Forward。

通过 Project 的 Create 菜单选择 `Rendering/Demo Showcase Pipeline` 创建新资产，Inspector 中为 `compositeMaterial` 选择 `Composite.ursamat`，编辑字段后 Save，再通过 Graphics 设置应用。配置保存在 `ProjectSettings/Graphics.json`，引用 `.meta` GUID 与资产局部身份；移动资产时应使用 Project 的移动操作。无效配置和工厂失败会给出诊断并保留上一有效选择。

项目继承 `RenderPipelineAsset`、实现 `CreatePipeline()`，再由 `RenderPipeline.Render(context, cameras)` 组织相机、剔除、RendererList、命令缓冲和提交。样例每个相机复用同一剔除结果，分阶段绘制带深度的离屏目标，最后通过项目材质全屏合成；资源在 Dispose 中回收。多相机、计算与字段说明见 [Rendering 示例](Assets/Rendering/README.md)，API、生命周期和支持范围见 [渲染指南](../Engine/Docs/ScriptableRendering.md)。

Graphics 选择和依赖会随显式导入与 Pak 发布。改变选择或资产后，先重新执行对应配置的 `UrsaAssetImport`，再启动 Player 或运行 `UrsaPak`。Editor Stop 状态可以重编译管线；Play 期间源码变化将在 Stop 后重载。MaterialShowcase 的运行方块由 Behaviour 创建，需 Play 后观察。

`Library/` 是可重建缓存。关闭使用该项目的 Editor/导入进程后，可移走或删除 Library，再按上面的命令完整导入；保留 `Assets/` 下的 `.meta` 和 `ProjectSettings/`，即可恢复原 GUID、管线选择、Shader/计算产物和 C# 程序集。普通 C++ 构建不会准备这些内容。

默认管线、合成方向、三种材质/地板、计算与 Pak 像素验证可运行：

```powershell
ctest --test-dir Engine/Build -C Release -R '^UrsaProjectPipelineSamplesTests$' --output-on-failure
```

## 网格组件

Demo 使用 `GameObject.CreatePrimitive(PrimitiveType.Cube)` 或 MeshFilter + MeshRenderer 组合。棋盘、蛇身和食物共享引擎内置 Cube 几何，颜色写入并缓存独立 Material，在对象移除或停止游戏时显式释放。原来 BoxRender 的展示颜色通过 `Color.ToLinear()` 保持显示效果；材质展示保留 Checker、Ceramic、Gold 的原资产 GUID。

迁移旧副本时先运行 Engine 的 `UrsaMeshMigrate --project <项目目录>`，再显式导入当前配置内容。导入产生的 Mesh、材质、托管程序集和缓存仍位于项目 Library，普通构建不触发资源导入。
