// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class MassActors : ModuleRules
	{
		public MassActors(ReadOnlyTargetRules Target) : base(Target)
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
					"Core",
					"CoreUObject",
					"Engine",
					"MassEntity",
					"AIModule",
					"MassCommon",
					"MassMovement",
					"MassSpawner",
					"MassSimulation",
					"MassReplication",
					"MassSignals",
					"StructUtils",
					"NetCore",
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
