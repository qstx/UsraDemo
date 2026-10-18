using System;
using Ursa.Editor;
using Ursa.Demo;

[CustomEditor(typeof(SnakeController))]
public sealed class SnakeControllerInspector : Inspector
{
	public override void OnInspectorGUI()
	{
		SerializedObject.Update();
		EditorGui.Text("平面网格移动，3D 视角呈现。");
		EditorGui.PropertyField(SerializedObject.FindProperty("width"), "棋盘宽度");
		EditorGui.PropertyField(SerializedObject.FindProperty("height"), "棋盘高度");
		EditorGui.PropertyField(SerializedObject.FindProperty("seed"), "随机种子");
		EditorGui.PropertyField(SerializedObject.FindProperty("smokeGameplay"), "自动冒烟流程");
		SerializedObject.ApplyModifiedProperties();
	}
}

[CustomEditor(typeof(SnakeBoardBehaviour))]
public sealed class SnakeBoardInspector : Inspector
{
	public override void OnInspectorGUI()
	{
		SerializedObject.Update();
		EditorGui.Text("棋盘格由 C# Prefab 行为生成。");
		EditorGui.PropertyField(SerializedObject.FindProperty("width"), "棋盘宽度");
		EditorGui.PropertyField(SerializedObject.FindProperty("height"), "棋盘高度");
		SerializedObject.ApplyModifiedProperties();
	}
}

public static class SnakeEditorMenus
{
	[MenuItem("Tools/Snake/Save Current Scene")]
	private static void SaveCurrentScene()
	{
		if (EditorContext.Document != null)
		{
			EditorContext.Save();
		}
	}
}

[EditorWindowMenu("Tools/Snake/Controls")]
public sealed class SnakeControlsWindow : EditorWindow
{
	public override string Title => "Snake Controls";

	public override void OnGUI()
	{
		EditorGui.Text("方向键或 WASD：转向");
		EditorGui.Text("P：暂停 / 继续");
		EditorGui.Text("空格：重新开始");
	}
}
