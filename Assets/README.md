# 贪吃蛇资源

此目录仅保存贪吃蛇源资源，挂载为可写的 `prj:`。引擎内置 Cube 和 Shader 通过只读的 `ursa:` 引用。

`Snake/Snake.ursascene` 引用静态 `Snake/Board.ursaprefab`，场景内直接保存相机、初始蛇身、食物、蛇身对象池及材质。编辑状态即可观察与调整完整画面；进入游戏后脚本只更新已有对象的位置及显隐，不动态生成场景。

C# 源码位于 `Scripts/Snake/`，编辑器工具位于 `Scripts/Editor/`。生成程序集、导入产物和缓存均放在项目 `Library/`。
