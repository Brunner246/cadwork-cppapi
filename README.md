# cadwork-cppapi – vcpkg Setup

This project uses **vcpkg manifest mode** to manage C/C++ dependencies.

The manifest file is:

- `vcpkg.json` – created with:
  - `C:\vcpkg\vcpkg.exe new --application`

## Prerequisites

- vcpkg is cloned/installed at `C:\vcpkg`.
- You can run vcpkg either by full path or by setting `VCPKG_ROOT` and updating `PATH`.

Example (PowerShell):

```powershell
$env:VCPKG_ROOT = 'C:\vcpkg'
$env:Path += ";$env:VCPKG_ROOT"
```

Then you can run:

```powershell
vcpkg version
```

or directly:

```powershell
& "C:\vcpkg\vcpkg.exe" version
```

## vcpkg manifest mode

When you run vcpkg commands in the project folder (where `vcpkg.json` lives), vcpkg automatically uses the manifest to know which libraries to install.

Typical commands from the project root (`cadwork-cppapi`):

```powershell
# Install all dependencies listed in vcpkg.json for the default triplet
& "C:\vcpkg\vcpkg.exe" install

# Or specify an explicit triplet, e.g. x64-windows
& "C:\vcpkg\vcpkg.exe" install --triplet x64-windows
```

You can add dependencies by editing `vcpkg.json` and then re-running `vcpkg install`.

## Updating dependencies

- Edit `vcpkg.json` to add/remove libraries.
- Run `vcpkg install` again from the project root.
- To see what vcpkg plans to do, you can run:

```powershell
& "C:\vcpkg\vcpkg.exe" install --dry-run
```

## Notes

- vcpkg does **not** build this project by itself; it only manages third‑party libraries.
- Use your normal build system (e.g. CMake, MSBuild, etc.) and point it at the vcpkg installed libraries/triplet as needed.

## CMake project layout

This repository contains a simple CMake project:

- [CMakeLists.txt](CMakeLists.txt) – root project file.
- [src/CMakeLists.txt](src/CMakeLists.txt) – builds a DLL target named `app`.
- [src/app.cpp](src/app.cpp) – minimal implementation with a sample function `app_hello`.

To configure and build with CMake (using vcpkg as a toolchain), from the repository root:

```powershell
mkdir build
cd build

cmake .. `
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DCMAKE_BUILD_TYPE=Release

cmake --build . --config Release
```

On success, the `app` DLL will be produced under the build tree (for example `build/Release/app.dll` on Windows).
