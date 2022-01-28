// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class EngineUtils : ModuleRules
	{
		public EngineUtils(ReadOnlyTargetRules Target) : base(Target)
		{
			PublicDefinitions.AddRange(
				new string[] {
					"UE_ENABLE_DEBUG_DRAWING"
				}
			);
			
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
