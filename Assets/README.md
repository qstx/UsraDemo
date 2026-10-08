# Demo 资源

此目录是 Demo 项目的可写资源根，挂载为 `prj:`。Editor 创建的 Scene、Prefab、材质、纹理和导入资源都保存在这里。Engine 内置资源从相邻的 Engine 仓库以只读 `ursa:` 挂载访问。

`Snake/Snake.ursascene` 引用 `Snake/Board.ursaprefab`。Prefab 上的 `SnakeBoardBehaviour` 用 C# 创建棋盘底座和格子；Scene 上的 `SnakeController` 负责规则、输入、蛇身和食物。使用通用 `UrsaEditor --project Demo --scene Snake/Snake.ursascene` 编辑，或用 `UrsaScriptPlayer` 独立运行。

`MaterialShowcase.ursascene` 演示 Unlit 与 PBR 材质。项目运行脚本和 Editor 扩展位于 `Scripts/`；编译项目、导入缓存和生成文件放在 `Library/`，不要放进此资源目录。
