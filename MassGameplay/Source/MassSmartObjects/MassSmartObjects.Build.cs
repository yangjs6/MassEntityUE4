// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class MassSmartObjects : ModuleRules
	{
		public MassSmartObjects(ReadOnlyTargetRules Target) : base(Target)
		{
			CppStandard = CppStandardVersion.Cpp17;
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicIncludePaths.AddRange(
				new string[] {
				"Runtime/AIModule/Public",
				ModuleDirectory + "/Public",
				}
			);

			PrivateIncludePaths.AddRange(
				new string[] {
				}
			);

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"MassEntity",
					"Core",
					"CoreUObject",
					"Engine",
					"GameplayTags",
					"MassCommon",
					"MassLOD",
					"MassMovement",
					"MassSignals",
					"MassSimulation",
					"MassSpawner",
					"SmartObjectsModule",
					"StructUtils",
					"EngineUtils",
					"ZoneGraph",
					"ZoneGraphAnnotations"
				}
			);
		}
	}
}
