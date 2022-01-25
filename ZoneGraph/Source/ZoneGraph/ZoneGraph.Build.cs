// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class ZoneGraph : ModuleRules
	{
		public ZoneGraph(ReadOnlyTargetRules Target) : base(Target)
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
					"RHI",
					"RenderCore",
					"DeveloperSettings",
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"EngineUtils"
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
