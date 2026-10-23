# Universal Rendering sample

This folder contains the saved Universal renderer/pipeline and Screen Tint feature configuration. Select `Rendering/Universal Pipeline Sample` in Project Graphics Settings to use the sample. The tint feature runs after transparent rendering for Game cameras, uses a viewport-sized temporary copy, and leaves Scene and Preview cameras unchanged. It creates a private material from the packaged `Ursa/Examples/ScreenTint` Shader and releases that runtime material when its pipeline is disposed.

To change the effect, edit `ScreenTintFeature` in the Inspector and adjust its tint. The feature is a normal `ScriptableRendererFeature` asset, restricts itself to Game cameras and uses the public `Ursa/Examples/ScreenTint` Shader.
