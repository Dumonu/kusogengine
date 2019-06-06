# Kusogengine

A Fighting Game Engine written in C++ using all the lessons we learned working on
[Ham5andw1ch/AnimeFightGame](https://github.com/Ham5andw1ch/AnimeFightGame).

The name is a portmanteau of Kusoge and Engine.

## Building

Right now, SDL2 has not been integrated into the system, but that should happen very soon.

The Makefile is multiplatform, supporting Linux, Mac OSX, and even Windows. It is also not
recursive as that was becoming very difficult to maintain.

To build on Linux and Mac OSX, simply run `make`.

To build on Windows:
- Install MinGW-w64 based tools.
- Install a Windows distribution of `awk` (necessary for the Makefile)
- Run `mingw32-make`