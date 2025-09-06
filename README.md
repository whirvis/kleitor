<div style="text-align: center;" align="center">
  <a href=""><img src="https://i.imgur.com/gFfmDYB.png" height="220"
    title="One of the Japanese logos for Sonic CD"/></a>
</div>

# The Project
This is the semester long project for a graduate class at my university.
The code remains unchanged since it's final submission, except for changes
to protect my privacy.
The project originally aimed to recreate at least one level from one of
the original [Sonic the Hedgehog](https://en.wikipedia.org/wiki/Sonic_CD)
games for the [SEGA Genesis](https://en.wikipedia.org/wiki/Sega_Genesis)
from the early to mid 1990s. But, due to limited time, I could only make
a title screen. However, I may come back to this project in the future.

*Sonic the Hedgehog* is owned by **SEGA Corporation.**

## 🕹️ Building the Program

**Windows, with Visual Studio:**
- This assumes you are building the project with
  [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/)
  or later.
  When installing Visual Studio, make sure you add the "Desktop development
  with C++" workload. This should allow you to compile and link C code and
  manage dependenies with `vcpkg`. Once everything is installed, you can
  open the project in the Visual Studio IDE. The solution file is located
  at `msvs/Kleitor.sln`. From there, you should be able to immediately run
  it with the Local Windows Debugger.

  If you have issues building or running due to missing headers or
  libraries, make sure the libraries are installed on your system. You can
  do this by opening a Developer PowerShell (or Developer Command Prompt)
  and running one of the following commands while in the `msvs` directory:
  ```sh
  vcpkg install --triplet x64-windows # for 64-bit systems
  vcpkg install --triplet x86-windows # for 32-bit systems
  ```
  After installing the libraries for the desired architecture, **make sure
  you set the Visual Studio IDE to match it!** Otherwise, it will not find
  the libraries even though they have been installed. Note that installing
  them for both architectures may not work (in my experience, `vcpkg` will
  remove the binaries for the other architecture).

  If the program crashes with `RBTK_ERROR_IO`, make sure the current working
  directory is correct. You can do this by selecting the "Program" project,
  and then going to "Project -> Properties" (located on the toolbar). Then,
  find the tab "Configuration Properties -> Debugging". There should be an
  option in this section called  "Working Directory". Make sure it is set to
  `$(ProjectDir)..\`.

**Linux, with CMake:**
- This assumes you are running an Arch or Debian based Linux distro.
  Building the project on Linux should be straightforward. You only need
  to install the project's dependencies and then run CMake in the project's
  root directory. After building the project, you should also be able to
  immediately run the program from the terminal. You can do this with the
  following commands:
  ```
  sudo pacman -S   cmake glew glfw  openal      # Arch
  sudo apt install cmake glew glfw3 openal-soft # Debian

  git clone git@github.com:whirvis/kleitor.git
  cd kleitor
  cmake -S . -B build
  cmake --build build

  build/src/kleitor
  ```

**Macintosh, with XCode:** Not supported.

If you still have any issues while or after following these steps,
something may have gone wrong with installation. Please open an issue
and I will be happy to assist.

## 📦 Building the Installer

ℹ️ **Note: This section applies to Windows only.**

The installer for this project can be built in Release mode for x64
architectures. I tried to setup the installer to add files conditionally
(i.e., for x86, x64, Debug, Release, etc.) but I had no luck with it.
If you would like to create an installer for different settings, you can
do so by modifying the installer project yourself. The files you will most
likely need to add or modify  are: `glew32.dll`, `glew32d.dll` (for debug
builds), `glfw3.dll`, and `OpenAL32.dll`.

The entries above are located in the `bin` directory of the Application
Folder. To modify a binary, you will need to delete it and then re-add it.
The source path cannot be modified from the properties window.
After building and running the installer, the program can be uninstalled
from the Control Panel. The program is named "Sonic CD (Kleitor)" (rather
than just "Sonic CD") to prevent confusion with the original that used to
be available on [Steam.](https://store.steampowered.com/app/200940/)

If you have an issue with the "Installer" project being incompatible,
make sure your installation of Visual Studio is fully up-to-date and
that you have installed the [Microsoft Visual Studio Installer Projects
2022](https://marketplace.visualstudio.com/items?itemName=VisualStudioClient.MicrosoftVisualStudio2022InstallerProjects)
extension. After doing both of those, right click on the project and
select "Reload Project". From there, the installer should have no more
issues.

## 🖨️ Generating the Docs
This project uses [Doxygen](https://www.doxygen.nl/index.html) for its
documentation. Before building the docs, you must download
[Doxygen Awesome](https://github.com/deepin-community/doxygen-awesome-css)
to the `docs` directory. After that, generating the documentation should
be as simple as running `doxygen` while in the `docs` directory. You can
download Doxygen Awesome in one of two ways:
```sh
git clone --recursive                   # download when cloning
git submodule update --init --recursive # download after cloning
```

## 🧰️ Technologies

| Category      | Tool or Library                                                                                                                                            |
| ------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Audio         | [MiniMP3](https://github.com/lieff/minimp3), [OpenAL](https://openal.org), and [STB Vorbis](https://github.com/nothings/stb)                               | 
| Build System  | [CMake](https://cmake.org) and [Visual Studio](https://visualstudio.microsoft.com)                                                                         |
| Debugging     | [GDB](https://sourceware.org/gdb/) and [RenderDoc](https://renderdoc.org)                                                                                  |
| Documentation | [Doxygen](https://doxygen.nl) and [Doxygen Awesome](https://github.com/jothepro/doxygen-awesome-css)                                                       |
| Graphics      | [GLFW](https://glfw.org), [OpenGL](https://opengl.org), [STB Image](https://github.com/nothings/stb), and [STB TrueType](https://github.com/nothings/stb)  |
| Mathematics   | [CGLM](https://github.com/recp/cglm)                                                                                                                       |

## ✨ Special Thanks

🐰 Special thanks to [Volt the Cabbit](https://www.youtube.com/@vtcabbit)
for allowing me to use [one of their songs](https://youtu.be/-p0w38ueDSg)
for this project.
