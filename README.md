# BIM Importer Plugin

**Supported version:** UE 4.27

**Dependencies:** [Runtime Mesh Component Plugin 4.1](https://runtimemesh.koderz.io/)

Using this plugin, you can import 3D BIM models into UE4 as Runtime Mesh Actors.
Features included:

- Should support 40+ 3D file formats. Tested on DXF
- Supports meshes and polyline models
- Custom material support

## Installation
1. Install the Runtime Mesh Component Plugin
2. Create the `Plugins` folder in the UE4 project folder if necessary, and open it in terminal
3. `git clone --recurse-submodules https://github.com/khatna/bim-importer-plugin`
4. Rebuild the project

## Installation for Android
Build script for Android (`/Source/ThirdParty/UE_AssimpLibrary/build_android.sh and Assimp_APL.xml`) use
environment variables that are machine-specific. Make sure to change these variables before building. 
Machine specific variables:

* `ANDROID_NDK`
* `ANDROID_PLATFORM`
* `ANDROID_ABI`
* `cmake` path
* Android target architecture in `Assimp_APL.xml`

## To-do
1. Look into garbage collection, possible memory leaks
2. Performance profiling

