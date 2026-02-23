// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

using UnrealBuildTool;

public class ModularGameplaySystems : ModuleRules
{
	public ModularGameplaySystems(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				// Public headers exposed to dependent modules
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				// Private headers
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"NetCore",
			}
		);
	}
}
