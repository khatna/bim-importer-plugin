// Fill out your copyright notice in the Description page of Project Settings.

using System;
using System.Diagnostics;
using System.Linq;
using System.IO;
using System.Text;
using UnrealBuildTool;

public class UE_AssimpLibrary : ModuleRules
{
    public UE_AssimpLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
        
        // add include path
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "assimp", "include"));
        
        // Prepare cmake process
        Process proc = new Process();
        proc.StartInfo.WorkingDirectory = ModuleDirectory;
        proc.StartInfo.FileName = "/bin/bash";
        proc.StartInfo.UseShellExecute = false;
        proc.StartInfo.RedirectStandardOutput = false;
        proc.StartInfo.CreateNoWindow = true;
        
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string AssimpDll = Path.Combine(ModuleDirectory, "assimp", "bin", "Release", "assimp.dll");
            
            // build if lib hasn't been built
            if (!File.Exists(AssimpDll))
            {
                Console.WriteLine("assimp dll not found");
                proc.StartInfo.FileName = Path.Combine(ModuleDirectory, "build_windows.bat");
                proc.Start();
                proc.WaitForExit();
            }

            // Add the import library
            PublicAdditionalLibraries.Add(AssimpDll);

            // Delay-load the DLL, so we can load it from the right place first
            PublicDelayLoadDLLs.Add(AssimpDll);

            // Ensure that the DLL is staged along with the executable
            RuntimeDependencies.Add("$(TargetOutputDir)/assimp.dll", AssimpDll);
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string AssimpDylib = Path.Combine(ModuleDirectory, "assimp", "bin", "libassimp.dylib");

            if (!File.Exists(AssimpDylib))
            {
                proc.StartInfo.Arguments = "./build_mac.sh";
                proc.Start();
                proc.WaitForExit();
            }

            PublicAdditionalLibraries.Add(AssimpDylib);
            RuntimeDependencies.Add("$(BinaryOutputDir)/libassimp.dylib", AssimpDylib);
            RuntimeDependencies.Add("$(BinaryOutputDir)/libassimp.5.dylib", AssimpDylib);
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string AssimpSo = Path.Combine(ModuleDirectory, "assimp", "bin", "libassimp.so");

            if (!File.Exists(AssimpSo))
            {
                proc.StartInfo.Arguments = "./build_linux.sh";
                proc.Start();
                proc.WaitForExit();
            }

            PublicAdditionalLibraries.Add(AssimpSo);
            RuntimeDependencies.Add("$(BinaryOutputDir)/libassimp.so", AssimpSo);
            RuntimeDependencies.Add("$(BinaryOutputDir)/libassimp.so.5", AssimpSo);
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string AssimpAndroidSo = Path.Combine(ModuleDirectory, "assimp", "bin", "libassimp.so");
            
            if (!File.Exists(AssimpAndroidSo))
            {
                proc.StartInfo.Arguments = "./build_android.sh";
                proc.Start();
                proc.WaitForExit();
            }
            
            PublicAdditionalLibraries.Add(AssimpAndroidSo);
            RuntimeDependencies.Add("$(BinaryOutputDir)/libassimp.so", AssimpAndroidSo);
        }
    }
}