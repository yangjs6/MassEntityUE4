// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class StateTreeEditorModule : ModuleRules
	{
		public StateTreeEditorModule(ReadOnlyTargetRules Target) : base(Target)
		{
			CppStandard = CppStandardVersion.Cpp17;
			PublicIncludePaths.AddRange(
			new string[] {
			}
			);

			PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"AssetTools",
				"EditorFramework",
				"UnrealEd",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"PropertyEditor",
				"AIModule",
				"StateTreeModule",
				"Projects",
				"BlueprintGraph",
				"PropertyAccessEditor",
				"StructUtils",
				"StructUtilsEditor",
			}
			);

			PrivateDependencyModuleNames.AddRange(
			new string[] {
				"RenderCore",
				"GraphEditor",
				"KismetWidgets",
				"PropertyPath",
				"PropertyEditor",
				"AIGraph",
				"ToolMenus",
			}
			);
		}

	}
}
