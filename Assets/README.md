# Demo assets

This directory is the writable project asset root for UrsaDemo, mounted as `prj:`. Editor-created `.ursatex`, `.ursamat`, `.ursascene`, and `.ursaprefab` files belong here. The Engine's built-in assets remain in the sibling Engine repository and are mounted read-only as `ursa:`; compiled shaders are generated into `../cache/Assets`, the mirror that `ursa:` overlays on the engine sources.

`Snake/Snake.ursascene` is the playable Snake scene. It references `Snake/Board.ursaprefab`, which owns the board base and 16-by-16 tile grid. The scene owns the game camera and Snake controller/presentation components. The presentation component creates the moving body and food from game state at runtime. Edit and save these resources with `UrsaDemoEditor`; board Prefab edits take effect the next time the scene is loaded.
