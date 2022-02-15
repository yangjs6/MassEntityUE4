// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class MassAIReplication : ModuleRules
	{
		public MassAIReplication(ReadOnlyTargetRules Target) : base(Target)
		{
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
					"Core",
					"CoreUObject",
					"Engine",
					"AIModule",
					"MassEntity",
					"MassCommon",
					"MassSpawner",
					"MassSimulation",
					"MassLOD",
					"NetCore",
					"StructUtils",
					"MassAIMovement",
					"MassReplication",
					"ZoneGraph"
				}
			);

			if (Target.bBuildEditor == true)
			{
				PrivateDependencyModuleNames.Add("EditorFramework");
				PrivateDependencyModuleNames.Add("UnrealEd");
			}
		}
	}
}
