// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class MassCrowd: ModuleRules
	{
		public MassCrowd(ReadOnlyTargetRules Target) : base(Target)
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
					"AIModule",
					"MassEntity",
					"Core",
					"CoreUObject",
					"Engine",
					"MassActors",
					"MassCommon",
					"MassLOD",
					"MassMovement",
					"MassAIMovement",
					"MassReplication",
					"MassAIReplication",
					"MassSignals",
					"MassSimulation",
					"MassSpawner",
					"MassRepresentation",
					"MassAIBehavior",
					"NetCore",
					"StateTreeModule",
					"StructUtils",
					"ZoneGraph",
					"ZoneGraphAnnotations",
					"ZoneGraphDebug"
				}
			);
		}
	}
}