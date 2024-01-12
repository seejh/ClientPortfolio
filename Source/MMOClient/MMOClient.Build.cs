// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class MMOClient : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    public MMOClient(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG", "Sockets", "Networking", "Libprotobuf" });


        PublicIncludePaths.AddRange(new string[] {
			// ... add public include paths required here ...

			Path.Combine(ModuleDirectory, "proto")
        });

        System.Console.WriteLine("-------------" + ModuleDirectory);
    }
}
