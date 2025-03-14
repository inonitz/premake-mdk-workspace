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
## About The Project
I needed to manage multiple projects in a single cross-platform-workspace with easy integration to clangd  
I had multiple options:
* Makefile - Will not go back to those
* **[cmake](https://cmake.org/)**           - Industry standard, should've probably used that
* **[xmake](https://github.com/xmake-io)**  - I didn't need an alternative to CMAKE
* **[premake](https://premake.github.io/)** - A meta build system with lua syntax (also like xmake, except more barebones)

Equipped with a new tool, I started migrating my previous **[project](https://github.com/inonitz/makefile-library-template)** because recompilation targets were non-existent
<br>
    ***This project is the result!***
</br>
### Project Structure
Each Project contains a ```premake5.lua``` file, describing everything about its compilation/linking
**There are 5 sub-projects available as reference/guiding points if you don't understand the Explanation below**
<br>
<br>
* To add a project to compilation/linking:
    * Add the path at ```projects/lua```
    * Specify a LinkMyLibraryName function at the root ```premake5.lua``` file (see ```LinkImGuiLibrary()``` for more info)
    * Use ```IncludeProjectHeaders(...)``` & ```LinkMyLibraryName``` in your library/executable' premake5.lua 
* To add a dependency to compilation/linking:
    * Add your library to the folder ```dependencies/```
    * Specify 2 functions at the root ```premake5.lua``` file:
        * LinkMyLibraryName
        * IncludeProjectHeaders
    * Use The defined functions in your library/executable' premake5.lua 


### Built With
<br> [<img height="100px" src="https://raw.githubusercontent.com/cginternals/glbinding/master/glbinding-logo.svg?sanitize=true">][glbinding-url] </br>
<br> 
  [![GLFW v3.4][GLFW.js]][GLFW-url]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  [![ImGui][ImGui.js]][ImGui-url]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  [![Premake][Premake.js]][Premake-url]
</br>

<!-- GETTING STARTED -->
## Getting Started

### Prerequisites
* [premake](https://premake.github.io/docs/) 
* Working compiler toolchain, preferably clang
  * Windows: You should use [llvm](https://github.com/llvm/llvm-project/releases)
  * Linux:
      1. [installing-specific-llvm-version](https://askubuntu.com/questions/1508260/how-do-i-install-clang-18-on-ubuntu)
      2. [configuring-symlinks](https://unix.stackexchange.com/questions/596226/how-to-change-clang-10-llvm-10-etc-to-clang-llvm-etc)
  * Define these environment variables (in your PATH):
    * LLVMInstallDir
    * LLVMToolsVersion
* Powershell / Any Standard unix-shell


### Installation
#### There are 2 branches available:
* **with-subprojects** - Includes ImGui, GLFW, glbinding, awc2 and a sample program at *program/*
* **barebones** - Executable-With-Library Samples, including reference premake files for: 
    * ImGui
    * GLFW
    * glbinding
    * awc2
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

call ```premake5 help``` in the cloned repo directory ```(.vscode/..)```

### Common Commands:
```sh
    premake5 --proj=program cleanproj 
    premake cleanall 
    premake cleancfgs
    premake cleanclangd
    premake export-compile-commands
    premake --os=windows --arch=x86_64 --cc=clang gmake2
    premake --os=windows --arch=x86_64 --cc=clang vs2022
    premake --os=linux --arch=x86_64 --cc=clang gmake2
```



<!-- ROADMAP -->
## Roadmap
- Adding an option to delete files based on architecture (e.g ```cleanarch --arch='x'```)
- Premake should be able to generate vs2022 files. This premake project can't do that
- Optimization of execution time:
  * ```with-subprojects``` branch
    * ~7sec [windows] 
    * ~4sec [wsl2] 
  * ```barebones``` branch
    * ~242ms [windows]


<!-- CONTRIBUTING -->
## Contributing
If you have a suggestion, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".  


<!-- LICENSE -->
## License
Distributed under the MIT License. See `LICENSE` file for more information.


<!-- ACKNOWLEDGEMENTS -->
## Acknowledgements
* [Kumodatsu](https://github.com/Kumodatsu/template-cpp-premake5/tree/master) For the initial template repo
* [Jarod42](https://github.com/Jarod42/premake-export-compile-commands/tree/Improvements) For the Improvements branch of export-compile-commands
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
