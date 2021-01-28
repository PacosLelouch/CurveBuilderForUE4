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
		string ComputeDir = Path.Combine(ModuleDirectory, "Compute");
		string RuntimeComponentDir = Path.Combine(ModuleDirectory, "RuntimeComponent");
		string ThirdPartyDir = Path.Combine(ModuleDirectory, "ThirdParty");

		PublicIncludePaths.AddRange(
			new string[] {
				BaseDir,
				ThirdPartyDir,
				ComputeDir,
				RuntimeComponentDir,
				//DoubleWorldDir,
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				BaseDir,
				ThirdPartyDir,
				ComputeDir,
				RuntimeComponentDir,
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
				"InputCore",
				"RHI",
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
