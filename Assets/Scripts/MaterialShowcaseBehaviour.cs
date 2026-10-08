using Ursa;

[UrsaScriptType("demo.material-showcase", Version = 1)]
public sealed class MaterialShowcaseBehaviour : Behaviour
{
	public AssetReference checkerMaterial =
		new("66666666-6666-4666-8666-666666666666");
	public AssetReference ceramicMaterial =
		new("8b8e4e91-58a2-4a26-a2d5-0a64365d3401");
	public AssetReference goldMaterial =
		new("a3f83e80-98a7-4d9e-a606-1c4a572b376b");

	public override void Awake()
	{
		AddMaterialBox("Checker", -2.2f, MaterialAsset.Load(checkerMaterial), new Color(1, 1, 1));
		AddMaterialBox("Ceramic", 0, MaterialAsset.Load(ceramicMaterial), new Color(1, 1, 1));
		AddMaterialBox("Gold", 2.2f, MaterialAsset.Load(goldMaterial), new Color(1, 1, 1));
		GameObject floor = GameObject.Create("Showcase Floor");
		floor.Parent = GameObject;
		floor.Transform.LocalPosition = new Vector3(0, -1.0f, -0.5f);
		floor.Transform.LocalScale = new Vector3(8.0f, 0.2f, 3.0f);
		floor.AddComponent<BoxRenderComponent>().Color = new Color(0.13f, 0.17f, 0.22f);
	}

	private void AddMaterialBox(string name, float x, MaterialAsset material, Color color)
	{
		GameObject item = GameObject.Create(name);
		item.Parent = GameObject;
		item.Transform.LocalPosition = new Vector3(x, 0, 0);
		item.Transform.LocalScale = new Vector3(1.5f, 1.5f, 1.5f);
		BoxRenderComponent renderer = item.AddComponent<BoxRenderComponent>();
		renderer.Color = color;
		renderer.Material = material;
	}
}
