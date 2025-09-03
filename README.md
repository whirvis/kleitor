# The Project
This is the semester long project for a graduate class at my university.
The code remains unchanged since it's final submission, except for changes
to protect my privacy. The project originally aimed to recreate at least
one level from one of the original Sonic the Hedgehog games, created for
the SEGA Genesis from the early to mid 1990s. But, due to limited time, I
could only make a title screen. However, I may come back to this project
in the future.

*Sonic the Hedgehog* is owned by **SEGA Corporation.**

# Building the Program
This assumes you are building the project with the latest version Visual
Studio 2022 or higher. Your installation of Visual Studio should have the
tools to compile C code and `vcpkg` for dependencies. Once everything is
installed, you can open the project in the Visual Studio IDE. The solution
file is located at `msvs/Kleitor.sln`. From there, you should be able to
immediately run it with the debugger.

 - "Kleitor" is a combination of the two words "Kleine" and "motor".
   These two words are Dutch for "Tiny" and "engine".

If you have issues building due to missing headers or libraries, make sure
the libraries are installed on your system. You can do this by opening a
Developer PowerShell (or Developer Command Prompt) and running one of the
following commands while in the `msvs` directory:
```
vcpkg install --triplet x64-windows
vcpkg install --triplet x86-windows
```
After installing the libraries for the desired architecture, **make sure that
you set the Visual Studio IDE to match it!** Otherwise, it will not find the
libraries even though they have been installed. Note that installing them for
both architectures may not work (`vcpkg` will probably remove the binaries for
the other architecture).

If the program crashes with an `RBTK_ERROR_IO` error, make sure the program
is running in the correct working directory. You can do this by selecting the
"Program" project, and then going to "Project -> Properties" (located on the
toolbar). Then, find the tab "Configuration Properties -> Debugging". There
is an option in this section called  "Working Directory". Make sure it is set
to `$(ProjectDir)..\`.

If you still have issues, something may have gone wrong with installation.
Please open an issue and I will be happy to assist.

# Building the Installer
The installer for this project can be built in Release mode for the x64
architecture. I tried to setup the installer project to add files conditionally
(i.e., for x86, x64, Debug, Release, etc.) but I had no luck with it. If you
would like to create an installer for different settings, you can do so easily
by modifying the installer project yourself. If you decide to do so, the files
you will most likely need to add/modify are:
```
glew32.dll
glew32d.dll
glfw3.dll
OpenAL32.dll
```
They are located in the `bin` directory of the Application Folder. To change
the location of a binary, you will need to delete it and then re-add it.
The source path cannot be modified from the properties window. After building
and running the installer, the program can be uninstalled from the Control
Panel. The program is named "Sonic CD (Kleitor)" (rather than just "Sonic CD")
to prevent confusion with the original which is available on Steam.

# Generating the Docs
This project uses Doxygen for documentation. Before building the docs,
you must download `doxygen-awesome-css` to the root directory. After that,
it should be as simple as running `doxygen` while in the `docs` directory.
Assuming you downloaded the repository via `git clone`, you can use this
command to download `doxygen-awesome-css`:
```
git submodule update --recursive --init
```
