// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
	public class MassEntity : ModuleRules
	{
		public MassEntity(ReadOnlyTargetRules Target) : base(Target)
		{
			CppStandard = CppStandardVersion.Cpp17;
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicIncludePaths.AddRange(
				new string[] {
				ModuleDirectory + "/Public",
				}
			);

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"Engine",
					"StructUtils",
					"EngineUtils",
					"DeveloperSettings",
				}
			);

			if (Target.bBuildEditor == true)
			{
				PrivateDependencyModuleNames.Add("UnrealEd");
			}

			if (Target.bBuildDeveloperTools == true)
			{
				PrivateDependencyModuleNames.Add("MessageLog");
			}
		}
	}
}