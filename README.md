[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![MIT][license-shield]][license-url]



<!-- PROJECT LOGO -->
<br />
<div align="center">
<h3 align="center">2D Incompressible Fluid Simulation</h3>

  <p align="center">
    Implementation of </br>
      <a href="https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-38-fast-fluid-dynamics-simulation-gpu">
        Chapter 38. Fast Fluid Dynamics Simulation on the GPU
    </a> 
  </p>
</div>


<!-- ABOUT THE PROJECT -->
## About The Project
The following code simulates a 2D Incompressible Fluid using an:
* Eulerian-Grid Scheme
* Semi-lagrangian Advection Method [Unconditionally Stable]
* System-of-Equations Jacobi Method Solver [Very easy to implement GPGPU]
* Central Difference for Approximating Calculus Operators [O(x^2) Error]

* **Essentially All methods here are references to the original work of** [Stam1999 - stable fluids](https://pages.cs.wisc.edu/~chaol/data/cs777/stam-stable_fluids.pdf)
**[Here](https://en.wikipedia.org/wiki/Projection_method_(fluid_dynamics)) is a wikipedia article describing the method of solution**

<br>

The currently most-updated revision ```23multigrid/``` features the following:
* Force/Dye/Force-Dye User-Simulation Interaction
* Vorticity Confinement with variable coefficient
* Variable Kinematic Viscosity, Delta Time & Poisson-Solver Iterations
* Real-Time CFL/Reynolds Number Calculations (they're there to check solver correctness/stability)
* Visualizations of Calculation Textures, in particular Velocity-Pressure/Dye/Curl/Absolute Curl/Velocity Error/Pressure Error/Velocity-X/Velocity-Y/Pressure/CFL
* Real-Time Measurements of internal solver components - CPU & GPU Side:
  * A GPU Timer (gl-Begin/End-Query) essentially acts as a fence,   
    waiting for all previous GPU commands to finish  
    **In Short: Using GPU Timers degrades Performance by a few milliseconds**

  * When GPU Timers are not used, the calculations will be deferred until glfwSwapbuffers(),
    which by then will be dispatched & computed, updating glMemoryBarrier

</br>

### Pretty Pictures
Tomorrow, Im Tired lol :)


</br>

  
### Project Structure
The Underlying Project Structure uses my [premake5-workspace-template](https://github.com/inonitz/premake5-workspace-template) repo,  
You can expect the [```program/```](https://github.com/inonitz/compute-shader-fluid-2d/tree/gpu-gems38/projects/program) folder to occupy all revisions of the fluid-solver,   
culminating eventually with [```29cleanup3/```](https://github.com/inonitz/compute-shader-fluid-2d/tree/gpu-gems38/projects/program/source/29cleanup3) as the currently best revision
<br>
<br>
<br>


### Built With
<br> [<img height="100px" src="https://raw.githubusercontent.com/cginternals/glbinding/master/glbinding-logo.svg?sanitize=true">][glbinding-url] </br>
<br> 
  [![GLFW v3.4][GLFW.js]][GLFW-url]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  [![ImGui][ImGui.js]][ImGui-url]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  [![Premake][Premake.js]][Premake-url]
</br>

<!-- GETTING STARTED -->
## Getting Started

### Prerequisites - Instructions below work too, but you should follow [premake5-workspace-template](https://github.com/inonitz/premake5-workspace-template)
* [premake](https://premake.github.io/docs/) 
* Working compiler toolchain, preferably clang
  * Windows: You should use [llvm](https://github.com/llvm/llvm-project/releases)
  * Linux:
      1. [installing-specific-llvm-version](https://askubuntu.com/questions/1508260/how-do-i-install-clang-18-on-ubuntu)
      2. [configuring-symlinks](https://unix.stackexchange.com/questions/596226/how-to-change-clang-10-llvm-10-etc-to-clang-llvm-etc)
      3. **You Don't have to use LLVM, gcc works too**
  * Define these environment variables (in your PATH):
    * LLVMInstallDir
    * LLVMToolsVersion
* Powershell / Any Standard unix-shell


### Installation
Just Clone the repo

<br>


<!-- USAGE EXAMPLES -->
## Usage
### Build Process
```sh
premake5 --os=windows --arch=x86_64 --cc=clang gmake2
premake5 --os=windows --arch=x86_64 --cc=clang vs2022
premake5 --os=linux   --arch=x86_64 --cc=gcc   gmake2
premake5 --os=linux   --arch=x86_64 --cc=clang gmake2
```
You can also use this command to perform the whole build process,  
if youre using gmake2 and a commandline:  
```sh
premake5 --os=windows/linux --arch=x86_64 buildallrel
```
**Dont forget to actually build using your favorite IDE/command-line utility**

<br>

### Execution
**Static Library Builds: (premake5 gmake2, config=releaselib_amd64)**
```sh
./build/bin/ReleaseLib_amd64_program/program
```  
**DLL/Shared Library Builds: (premake5 gmake2, config=releasedll_amd64)**
```sh
./build/bin/ReleaseDll_amd64/program
```
**Visual Studio 2022 - Just Run normally using ```Local Windows Debugger```**

<!-- ROADMAP -->
## Roadmap
- Replacing Jacobi Method with the Multigrid Method for better performance & faster fluid convergence
- Adding Arbitrary boundaries
- Extending to 3D
- MAC Staggered Grid
- Different Simulation Schemes (FVM, etc...) for better simulation accuracy
- Extending to compressible/Turbulent Models


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


<!-- References -->
## References
* [Fluid Mechanics 101](https://www.youtube.com/@fluidmechanics101/videos)
* [GPU Gems 38](https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-38-fast-fluid-dynamics-simulation-gpu)
* [Stable Fluids - Stam 1999](https://www.dgp.toronto.edu/public_user/stam/reality/Research/pdf/ns.pdf)
* [Colour Advice](https://www.kennethmoreland.com/color-advice/)
* [Computational Methods for Fluid Dynamics - Fourth Edition (Ferziger, Perić, L. Street)](https://www.amazon.com/Computational-Methods-Fluid-Dynamics-Ferziger/dp/3319996916)
* [Online PDF of the Above Book](https://elmoukrie.com/wp-content/uploads/2022/05/joel-h.-ferziger-milovan-peric-robert-l.-street-computational-methods-for-fluid-dynamics-springer-international-publishing-2020.pdf)




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
