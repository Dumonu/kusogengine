# Kusogengine

A Fighting Game Engine written in C++ using all the lessons we learned working on
[Ham5andw1ch/AnimeFightGame](https://github.com/Ham5andw1ch/AnimeFightGame).

The name is a portmanteau of Kusoge and Engine.

## Building

The Makefile is multiplatform, supporting Linux, Mac OSX, and even Windows 64-bit. It is also
not recursive as that was becoming very difficult to maintain.

To build on Linux and Mac OSX, ensure SDL2 is installed and simply run `make`.

To build on Windows:
- Install MinGW-w64 based tools.
- Install a Windows distribution of `awk` (necessary for the Makefile)
- Download the mingw32 version of SDL2 from
[here](https://www.libsdl.org/download-2.0.php), place it in the top directory
of this project, and untar it.
- Run `mingw32-make SDL_VER=2.0.10`, replacing the version with the one you downloaded.
`SDL_VER` is `2.0.10` by default.
