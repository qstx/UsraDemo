using Ursa.Editor;

[CustomInspector("demo.move-with-input")]
public sealed class MoveWithInputInspector : Inspector
{
	public override void OnInspectorGUI()
	{
		EditorGui.Text("Right Arrow moves this object.");
		var step = SerializedObject.FindProperty("/Fields/step");
		EditorGui.PropertyField(step, "Step");
		if (EditorGui.Button("Reset Step"))
		{
			step.SetValue(1.0);
		}
		SerializedObject.ApplyModifiedProperties();
	}
}

public static class DemoEditorMenus
{
	[MenuItem("Tools/Demo/Save Current Asset")]
	private static void SaveCurrentAsset()
	{
		if (EditorContext.Document.ValueKind != System.Text.Json.JsonValueKind.Null)
		{
			EditorContext.Save();
		}
	}
}

[EditorWindowMenu("Tools/Demo/Project Overview")]
public sealed class ProjectOverviewWindow : EditorWindow
{
	public override string Title => "Demo Project Overview";

	public override void OnGUI()
	{
		EditorGui.Text("Assets: " + EditorContext.Assets.GetArrayLength());
		foreach (var selected in EditorContext.Selection.EnumerateArray())
		{
			EditorGui.Text(selected.GetProperty("Name").GetString() ?? "");
		}
	}
}
