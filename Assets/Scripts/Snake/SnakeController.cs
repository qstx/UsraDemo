using System;
using System.Collections.Generic;

namespace Ursa.Demo;
public sealed class SnakeController : Behaviour
{
	public int width = 16;
	public int height = 16;
	public uint seed = 1;
	public bool smokeGameplay;

	private readonly List<GameObject> segments = new();
	private readonly Dictionary<GameObject, Material> ownedMaterials = new();
	private SnakeGame? game;
	private SnakeFixedStepClock clock = new();
	private GameObject? foodObject;
	private bool paused;
	private ulong lastTitleFrame;
	private ulong smokeFrame;

	public override void Awake()
	{
		if (smokeGameplay)
		{
			width = 4;
			height = 4;
			game = new SnakeGame(width, height, seed,
				new[] { new SnakeCell(1, 1), new SnakeCell(0, 1) }, SnakeDirection.Right, new SnakeCell(2, 1));
		}
		else
		{
			game = new SnakeGame(width, height, seed);
		}
		SyncPresentation();
	}

	public override void FixedUpdate()
	{
		if (game == null || paused || smokeGameplay)
		{
			return;
		}
		int stepCount = clock.Advance(Time.FixedDeltaTime);
		for (int index = 0; index < stepCount; ++index)
		{
			game.Step();
		}
		SyncPresentation();
	}

	public override void Update()
	{
		if (game == null)
		{
			return;
		}
		if (smokeGameplay)
		{
			RunSmokeStep();
		}
		else
		{
			ReadDirectionKeys();
			if (Input.GetKeyDown(KeyCode.P))
			{
				paused = !paused;
			}
			if (Input.GetKeyDown(KeyCode.Space))
			{
				game.Reset();
				clock.Reset();
				paused = false;
			}
		}
		SyncPresentation();
		UpdateTitle();
	}

	private void ReadDirectionKeys()
	{
		if (Pressed(KeyCode.Up, KeyCode.W)) game!.QueueDirection(SnakeDirection.Up);
		else if (Pressed(KeyCode.Down, KeyCode.S)) game!.QueueDirection(SnakeDirection.Down);
		else if (Pressed(KeyCode.Left, KeyCode.A)) game!.QueueDirection(SnakeDirection.Left);
		else if (Pressed(KeyCode.Right, KeyCode.D)) game!.QueueDirection(SnakeDirection.Right);
	}

	private static bool Pressed(KeyCode primary, KeyCode secondary) =>
		Input.GetKeyDown(primary) || Input.GetKeyDown(secondary);

	private void SyncPresentation()
	{
		if (game == null)
		{
			return;
		}
		while (segments.Count > game.Body.Count)
		{
			ReleaseBox(segments[^1]);
			segments.RemoveAt(segments.Count - 1);
		}
		while (segments.Count < game.Body.Count)
		{
			segments.Add(CreateBox("Snake Segment", GameObject, default, new Vector3(0.84f, 0.84f, 0.84f),
				segments.Count == 0 ? new Color(0.35f, 0.95f, 0.5f) : new Color(0.12f, 0.65f, 0.42f)));
		}
		float xOffset = (game.Width - 1) * 0.5f;
		float zOffset = (game.Height - 1) * 0.5f;
		for (int index = 0; index < game.Body.Count; ++index)
		{
			SnakeCell cell = game.Body[index];
			GameObject segment = segments[index];
			segment.Transform.LocalPosition = new Vector3(cell.X - xOffset, 0.34f, cell.Z - zOffset);

		}
		if (!game.Food.HasValue)
		{
			if (foodObject != null) ReleaseBox(foodObject);
			foodObject = null;
			return;
		}
		if (foodObject == null)
		{
			foodObject = CreateBox("Food", GameObject, default, new Vector3(0.55f, 0.55f, 0.55f),
				new Color(1.0f, 0.45f, 0.12f));
		}
		SnakeCell food = game.Food.Value;
		foodObject.Transform.LocalPosition = new Vector3(food.X - xOffset, 0.28f, food.Z - zOffset);
	}

	private GameObject CreateBox(string name, GameObject parent, Vector3 position, Vector3 scale, Color color)
	{
		GameObject item = GameObject.CreatePrimitive();
		item.Name = name;
		item.Parent = parent;
		item.Transform.LocalPosition = position;
		item.Transform.LocalScale = scale;
		var material = item.GetComponent<MeshRendererComponent>()!.Material!;
		material.Color = color.ToLinear();
		ownedMaterials.Add(item, material);
		return item;
	}

	private void ReleaseBox(GameObject item)
	{
		item.Destroy();
		if (ownedMaterials.Remove(item, out var material) && material.IsValid) material.Destroy();
	}

	public override void OnDestroy()
	{
		foreach (var material in ownedMaterials.Values) if (material.IsValid) material.Destroy();
		ownedMaterials.Clear();
	}

	private void UpdateTitle()
	{
		if (game == null || lastTitleFrame == Time.FrameCount)
		{
			return;
		}
		lastTitleFrame = Time.FrameCount;
		string title = $"Ursa Snake | Score: {game.Score}";
		if (paused) title += " | Paused";
		else if (game.Status == SnakeStatus.Lost) title += " | Game Over";
		else if (game.Status == SnakeStatus.Won) title += " | You Win";
		title += " | Arrows/WASD: turn | P: pause | Space: restart";
		Application.SetWindowTitle(title);
	}

	private void RunSmokeStep()
	{
		ulong frame = ++smokeFrame;
		if (frame == 1)
		{
			game!.Step();
			if (frame == 1 && (game.Score != 1 || game.Body.Count != 3))
			{
				throw new InvalidOperationException("Snake smoke run did not eat and grow on the first step.");
			}
		}
		else if (frame == 2)
		{
			paused = true;
			if (game!.Body[0] != new SnakeCell(2, 1) || game.Score != 1)
			{
				throw new InvalidOperationException("Snake smoke run did not remain paused.");
			}
		}
		else if (frame == 3)
		{
			paused = false;
			game!.Step();
		}
		else if (frame == 4)
		{
			game!.Step();
			if (game.Status != SnakeStatus.Lost)
			{
				throw new InvalidOperationException("Snake smoke run did not detect the board collision.");
			}
		}
		else if (frame == 5)
		{
			game!.Reset();
			if (game.Status != SnakeStatus.Running || game.Score != 0)
			{
				throw new InvalidOperationException("Snake smoke run did not restart the game.");
			}
		}
	}
}
public sealed class SnakeBoardBehaviour : Behaviour
{
	private readonly List<Material> ownedMaterials = new();
	public int width = 16;
	public int height = 16;

	public override void Awake()
	{
		CreateBox("Board Base", new Vector3(0, -0.42f, 0),
			new Vector3(width + 0.35f, 0.3f, height + 0.35f), new Color(0.12f, 0.18f, 0.23f));
		float xOffset = (width - 1) * 0.5f;
		float zOffset = (height - 1) * 0.5f;
		for (int z = 0; z < height; ++z)
		{
			for (int x = 0; x < width; ++x)
			{
				float shade = (x + z) % 2 == 0 ? 0.25f : 0.31f;
				CreateBox($"Tile ({x}, {z})", new Vector3(x - xOffset, -0.22f, z - zOffset),
					new Vector3(0.96f, 0.08f, 0.96f), new Color(shade * 0.55f, shade, shade * 0.75f));
			}
		}
	}

	private void CreateBox(string name, Vector3 position, Vector3 scale, Color color)
	{
		GameObject item = GameObject.CreatePrimitive();
		item.Name = name;
		item.Parent = GameObject;
		item.Transform.LocalPosition = position;
		item.Transform.LocalScale = scale;
		var material = item.GetComponent<MeshRendererComponent>()!.Material!;
		material.Color = color.ToLinear();
		ownedMaterials.Add(material);
	}
	public override void OnDestroy()
	{
		foreach (var material in ownedMaterials) if (material.IsValid) material.Destroy();
		ownedMaterials.Clear();
	}

}
