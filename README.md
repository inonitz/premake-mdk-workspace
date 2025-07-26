[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![MIT][license-shield]][license-url]



<!-- PROJECT LOGO -->
<br />
<div align="center">
<h3 align="center">Multi-Project Workspace Template</h3>

  <p align="center">
    C/C++ Cross-Platform Multi-Project Template
    <br />
  </p>
</div>


<!-- ABOUT THE PROJECT -->
## About
This project is my best current & continuous effort at a portable C++ multi-project environment (without CMake!)   
This Project aims to work seamlessly across Linux & Windows  
Current Integration of tools:
  - [clangd](https://clangd.llvm.org/)
  - [ASan](https://github.com/google/sanitizers/wiki/addresssanitizer)
  - [UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
  - [clang-format](https://clang.llvm.org/docs/ClangFormat.html)
  - [clang-tidy](https://clang.llvm.org/extra/clang-tidy/)
  - [lldb](https://lldb.llvm.org/use/tutorial.html)
  - Various useful C++ Libraries
    * [ImGui](https://github.com/ocornut/imgui)
    * [GLFW](https://github.com/glfw/glfw/releases)
    * [glbinding](https://github.com/cginternals/glbinding/releases)
    * [Catch2](https://github.com/catchorg/Catch2)
    * [GoogleMock & GoogleTest](https://github.com/google/googletest)
    * [Google Benchmark](https://github.com/google/benchmark)

<!-- ABOUT THE PROJECT -->
### Considerations
The original motivation was mainly the continuous discomfort with using hand-made **[Makefiles](https://github.com/inonitz/makefile-library-template)** without recompilation targets, I had to recompile everything on every little change  
More recently, I needed integration with various libraries & tools on vscode  
I also needed **some** form of cross platform Support (Incoming [DLL Hell](https://stackoverflow.com/questions/1379287/i-keep-hearing-about-dll-hell-what-is-this)) (Also see [ABI Hell](https://stackoverflow.com/questions/2171177/what-is-an-application-binary-interface-abi)), so I eventually (unfortunately) decided I'll manage dependencies manually in a mono-repository (for now).

Considering the problem of build/meta-build systems, I examined multiple choices (found below) and eventually decided to use premake5 because of its relative simplicity to the competition:
* **[Make](https://www.gnu.org/software/make/)** - Will not go back to those, too cumbersome to manage manually
* **[CMake](https://cmake.org/)**           - Industry standard, everyone loves to hate it
* **[xmake](https://github.com/xmake-io)**  - Didn't need an alternative to cmake
* **[premake5](https://premake.github.io/)** - A meta build system with lua syntax (also like xmake, except more barebones)
* **[Bazel](https://bazel.build/)** - Seemed a little too much & too complex for what I needed
* **[Ninja](https://ninja-build.org/)** - I do not know of any particular examples of people writing ninja scripts manually
  
Moreover, With the complexity involved in managing updates across multiple projects, when each of which uses this specific Mono-Repository and constantly changes/updates it, I've decided to move each Library here to its separate 'premake5-packaged' repository, with the eventual integration of git submodules to the library-consuming project


### Project Structure
Each Project contains a ```premake5.lua``` file, describing everything about its compilation/linking  
**There are 5 sub-project lua files available as reference/guiding points if you don't understand the Explanation below**
#### To add a project to compilation/linking:
* Create a ```premake5.lua``` file in your project root folder (see examples)
* Add project path to ```PROJECT_LIST``` in ```premake5.lua```
* Specify a ```LinkMyLibraryName``` function in ```dir.lua``` (see ```LinkLibExampleLibrary()``` for more info)
* Use ```IncludeProjectHeaders(...)``` & ```LinkMyLibraryName``` in your other libraries/executables' (see ```sample/premake5.lua``` for more info)
#### To add a dependency (Header Only library, prebuilt shared/static library, etc...) to compilation/linking:
* Add the library to ```dependencies/```
* Specify 2 functions in ```dir.lua```:
  * ```LinkMyDependencyName```
  * ```IncludeDependencyNameHeaders```
* Use Them in your library/executables' premake5.lua 


### Built With
<br> [<img height="100px" src="https://raw.githubusercontent.com/cginternals/glbinding/master/glbinding-logo.svg?sanitize=true">][glbinding-url] </br>
<br>
[<img height="150px" src="https://avatars.githubusercontent.com/u/11135954?s=150&v=4">][Premake-url]
[<img height="150px" src="https://avatars.githubusercontent.com/u/3905364?s=150&v=4">][GLFW-url]
[<img height="150px" src="https://avatars.githubusercontent.com/u/8225057?s=150&v=4">][ImGui-url]
</br>

<!-- GETTING STARTED -->
## Getting Started

### Prerequisites
1. [premake5](https://premake.github.io/docs/)
2. Windows/Linux:
   - **Windows:**
   - [Msys2 Clang64](https://www.mingw-w64.org/getting-started/msys2-llvm)
   - [Clang LLVM](https://github.com/llvm/llvm-project/releases)
      * **[NOTE]:** There may be compilation errors with Clang LLVM due to <threads.h>
   - **Linux:**
   - [Installing a specific llvm version](https://askubuntu.com/questions/1508260/how-do-i-install-clang-18-on-ubuntu)
   - [Configure Symlinks](https://unix.stackexchange.com/questions/596226/how-to-change-clang-10-llvm-10-etc-to-clang-llvm-etc) - as clang-'version_number' will not be detected by premake5
3. Add your toolchain to the global PATH
4. **[NOTE]:** LLVM-Clang relies on Platform-Specific System Headers & Libraries
   - **Windows:** Standard Library implementation with System-Headers:
      * MSYS2 (use [clang-mingw64](https://packages.msys2.org/groups/mingw-w64-clang-x86_64-toolchain) instead of LLVM-Clang)
      * MinGW-w64
      * WinLibs
      * MSVC (libraries will be automatically detected)
   - **Linux:** Will very likely work out of the box
5. Bash Shell 
    - **Windows:**
    - [Git for Windows](https://gitforwindows.org/) 
    - MSYS2 Clang64 Shell (Using MSYS2 without the provided terminal causes program execution issues with DLL's)
    - **Linux:**
    * Use your favourite Bash Shell


### Installation
#### There are 4 branches available:
* **with-subprojects** - Includes ImGui, GLFW, glbinding, various homebrew Utility Libraries [awc2, util2], with a sample opengl compute program
* **barebones** - Executable-With-Library Sample, including reference premake files for: 
    * ImGui
    * GLFW
    * glbinding
    * awc2 & util2 (My own Utility Libraries)
* **GoogleBenchmark** - like barebones, except with [google-benchmark](https://github.com/google/benchmark) and a running example
* **GoogleTest-Mock** - like barebones, except with [google-test & google-mock](https://github.com/google/googletest) and a running example

```sh
# If you want everything
git clone -b with-subprojects https://github.com/inonitz/premake5-workspace-template.git
# If you prefer to configure on your own
git clone -b barebones https://github.com/inonitz/premake5-workspace-template.git
# Don't forget to add your own remote repo
git remote set-url origin your_github_username/premake5-workspace-template
git remote -v
```

<!-- USAGE EXAMPLES -->
## Usage

call ```premake5 --help``` in the root of the repository

### Common Commands:
```sh
    premake5 cleanproj --proj=program 
    premake5 cleanbuild
    premake5 cleancfg
    premake5 cleanclangd
    premake5 cleanall 
    premake5 export-compile-commands
    premake5 ecc (same as export-compile-commands)
    premake5 --os=windows --arch=x86_64 --cc=clang gmake
    premake5 --os=windows --arch=x86_64 --cc=clang ninja
    premake5 --os=windows --arch=x86_64 --cc=clang vs2022
    premake5 --os=linux --arch=x86_64 --cc=clang gmake
    premake5 --os=linux --arch=x86_64 --cc=gcc gmake
```



<!-- ROADMAP -->
## Roadmap
- Supporting VS2022 Project Solutions (they do not generate correctly)
- Generating a launch.json at Project-Generation Time
- Deleting files based on architecture (e.g ```cleanarch --arch='x'```)
- Generating Test-Unit Projects for each library (see prototype ```premake5_generate_unit_test_per_tu.lua```)
- Adding an action to update compile_commands.json based on target
- Integrating Cppcheck
- Integrating a cross-platform C++ profiler With Flame Graphs - [Tracy](https://github.com/wolfpld/tracy)(?)/[Optick](https://github.com/bombomby/optick)(?)/(?)
- Cross-platform package management - [Spack](github.com/spack/spack?tab=readme-ov-file)(?)
- Integration of [LLVM Machine-Code Analyzer](https://llvm.org/docs/CommandGuide/llvm-mca.html)
- Utilizing [Profile-Guided Optimization](https://clang.llvm.org/docs/UsersManual.html#profile-guided-optimization)
- Automating this whole thing with Dev-Containers & Docker (Several issues, one being very gpu-specific solutions from every vendor) 
- Optimization of Project-Generation Time:
  * ```with-subprojects``` branch
    * ```gmake```
      * ~3200ms [windows] 
      * ~2200ms [wsl2] 
    * ```ninja```
      * ~3100ms [windows]
      * ~2800ms [wsl2]
  * ```barebones``` branch
    * ```gmake```
      * ~120ms [windows]
      * ~TBD [wsl2]
    * ninja
      * ~120ms [windows]
      * ~TBD [wsl2]


<!-- CONTRIBUTING -->
## Contributing
If you have a suggestion, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".


<!-- LICENSE -->
## License
Distributed under the MIT License. See `LICENSE` file.


<!-- ACKNOWLEDGEMENTS -->
## Acknowledgements
* [Kumodatsu](https://github.com/Kumodatsu/template-cpp-premake5/tree/master) For the initial template repo
* [Jarod42](https://github.com/Jarod42/premake-export-compile-commands/tree/Improvements) For the Improvements branch of export-compile-commands
* [Premake Ninja](https://github.com/jimon/premake-ninja) - Ninja Build System integration with premake5
* [Best-README](https://github.com/othneildrew/Best-README-Template)


<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/inonitz/premake5-workspace-template?style=for-the-badge&color=blue
[contributors-url]: https://github.com/inonitz/premake5-workspace-template/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/inonitz/premake5-workspace-template?style=for-the-badge&color=blue
[forks-url]: https://github.com/inonitz/premake5-workspace-template/network/members
[stars-shield]: https://img.shields.io/github/stars/inonitz/premake5-workspace-template?style=for-the-badge&color=blue
[stars-url]: https://github.com/inonitz/premake5-workspace-template/stargazers
[issues-shield]: https://img.shields.io/github/issues/inonitz/premake5-workspace-template.svg?style=for-the-badge
[issues-url]: https://github.com/inonitz/premake5-workspace-template/issues
[license-shield]: https://img.shields.io/github/license/inonitz/premake5-workspace-template?style=for-the-badge
[license-url]: https://github.com/inonitz/premake5-workspace-template/blob/master/LICENSE
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/linkedin_username
[product-screenshot]: images/screenshot.png
[Next.js]: https://img.shields.io/badge/next.js-000000?style=for-the-badge&logo=nextdotjs&logoColor=white

[ImGui-url]: https://github.com/ocornut/imgui
[ImGui.js]: https://avatars.githubusercontent.com/u/8225057?v=4&size=150
[glbinding-url]: https://github.com/cginternals/glbinding/releases/tag/v3.3.0
[glbinding.js]: https://raw.githubusercontent.com/cginternals/glbinding/master/glbinding-logo.svg?sanitize=true
[GLFW-url]: https://github.com/glfw/glfw/releases/tag/3.4
[GLFW.js]: https://avatars.githubusercontent.com/u/3905364?s=200&v=4&size=150
[Premake-url]: https://github.com/premake/premake-core
[Premake.js]: https://avatars.githubusercontent.com/u/11135954?s=150&v=4
