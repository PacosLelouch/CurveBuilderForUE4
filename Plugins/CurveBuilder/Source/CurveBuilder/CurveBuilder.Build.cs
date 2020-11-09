// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class CurveBuilder : ModuleRules
{
	public CurveBuilder(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		//string DoubleWorldDir = Path.Combine(ModuleDirectory, "../DoubleWorld/");
		string BaseDir = Path.Combine(Path.Combine(ModuleDirectory, "Source"), ModuleDirectory);
		string ThirdPartyDir = Path.Combine(ModuleDirectory, "ThirdParty");

		PublicIncludePaths.AddRange(
			new string[] {
				BaseDir,
				ThirdPartyDir,
				//DoubleWorldDir,
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				//"DoubleWorld",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		
		//AddEngineThirdPartyPrivateStaticDependencies(Target, 
		//	"Eigen"
		//	);
	}
}
