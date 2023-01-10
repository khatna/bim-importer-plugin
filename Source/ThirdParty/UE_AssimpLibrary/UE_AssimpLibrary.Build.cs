// Fill out your copyright notice in the Description page of Project Settings.

using System;
using System.Diagnostics;
using System.Linq;
using System.IO;
using System.Text;
using UnrealBuildTool;

public class UE_AssimpLibrary : ModuleRules
{

    public string BinFolder(ReadOnlyTargetRules Target)
    {
        if (Target.Platform == UnrealTargetPlatform.Mac)
            return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/Mac/"));
        if (Target.Platform == UnrealTargetPlatform.IOS)
            return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/IOS/"));
        if (Target.Platform == UnrealTargetPlatform.Win64)
            return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/Win64/"));
        if (Target.Platform == UnrealTargetPlatform.Android)
            return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/Android/"));
        if (Target.Platform == UnrealTargetPlatform.Linux)
            return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/Linux/"));
        return "";
    }

    public UE_AssimpLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        string BinaryFolder = BinFolder(Target);
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "assimp", "include"));
        
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string AssimpDll = Path.Combine(ModuleDirectory, "assimp", "bin", "Release", "assimp.dll");
            // build if lib hasn't been built
            if (!File.Exists(AssimpDll))
            {
                Console.WriteLine("assimp dll not found");
                Process proc = new Process();
                proc.StartInfo.FileName = Path.Combine(ModuleDirectory, "build_windows.bat");
                proc.StartInfo.WorkingDirectory = ModuleDirectory;
                proc.StartInfo.UseShellExecute = false;
                proc.StartInfo.RedirectStandardOutput = false;
                proc.StartInfo.CreateNoWindow = true;
                proc.Start();
                proc.WaitForExit();
            }

            // Add the import library
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "assimp", "lib", "Release", "assimp.lib"));

            // Delay-load the DLL, so we can load it from the right place first
            PublicDelayLoadDLLs.Add(AssimpDll);
            
            // Copy to plugin bin folder
            Directory.CreateDirectory(BinaryFolder);
            string BinPath = Path.Combine(ModuleDirectory, BinaryFolder, "assimp.dll");
            CopyFile(AssimpDll, BinPath);

            // Ensure that the DLL is staged along with the executable
            RuntimeDependencies.Add("$(TargetOutputDir)/assimp.dll", BinPath);
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string AssimpDylib = Path.Combine(ModuleDirectory, "assimp", "bin", "libassimp.dylib");

            // build if lib hasn't been built
            if (!File.Exists(AssimpDylib))
            {
                var proc = new Process
                {
                    StartInfo = new ProcessStartInfo
                    {
                        FileName = "/bin/bash",
                        Arguments = "./build_mac.sh",
                        WorkingDirectory = ModuleDirectory,
                        UseShellExecute = false,
                        RedirectStandardOutput = false,
                        CreateNoWindow = true
                    }
                };
                proc.Start();
                proc.WaitForExit();
            }

            // Add the import library
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "assimp", "bin", "libassimp.dylib"));

            Directory.CreateDirectory(BinaryFolder);
            string BinPath = Path.Combine(ModuleDirectory, BinaryFolder, "libassimp.dylib");

            CopyFile(AssimpDylib, BinPath);

            // Stage shared library next to executable
            RuntimeDependencies.Add("$(TargetOutputDir)/libassimp.dylib", BinPath);
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string AssimpSo = Path.Combine(ModuleDirectory, "assimp", "bin", "libassimp.so");

            // build if lib hasn't been built
            if (!File.Exists(AssimpSo))
            {
                var proc = new Process
                {
                    StartInfo = new ProcessStartInfo
                    {
                        FileName = "/bin/bash",
                        Arguments = "./build_linux.sh",
                        WorkingDirectory = ModuleDirectory,
                        UseShellExecute = false,
                        RedirectStandardOutput = false,
                        CreateNoWindow = true
                    }
                };
                proc.Start();
                proc.WaitForExit();
            }

            // Add the import library
            PublicAdditionalLibraries.Add(AssimpSo);
            Directory.CreateDirectory(BinaryFolder);
            string BinPath = Path.Combine(ModuleDirectory, BinaryFolder, "libassimp.so");
            CopyFile(AssimpSo, BinPath);

            // Stage shared library next to executable
            RuntimeDependencies.Add("$(TargetOutputDir)/libassimp.so", BinPath);
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PublicAdditionalLibraries.Add(Path.Combine(BinaryFolder, "arm64-v8a", "libassimp.so"));
        }
    }
    public void CopyFile(string Source, string Dest)
    {
        System.Console.WriteLine("Copying {0} to {1}", Source, Dest);
        if (System.IO.File.Exists(Dest))
        {
            System.IO.File.SetAttributes(Dest, System.IO.File.GetAttributes(Dest) & ~System.IO.FileAttributes.ReadOnly);
        }
        try
        {
            //Make Folder

            System.IO.File.Copy(Source, Dest, true);
        }
        catch (System.Exception ex)
        {
            System.Console.WriteLine("Failed to copy file: {0}", ex.Message);
        }
    }


}