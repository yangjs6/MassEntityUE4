// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class EngineUtils : ModuleRules
	{
		public EngineUtils(ReadOnlyTargetRules Target) : base(Target)
		{
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
			}
			);

			PrivateDependencyModuleNames.AddRange(
			new string[] {
			}
			);
		}

	}
}
