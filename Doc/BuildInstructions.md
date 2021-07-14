# Build Instructions

Jump straight to the actual [Basic Build Instructions](#basic-build-instructions).


## General Project Information

- The C++ code is compiled with language version `c++17`
- Indent is two spaces. Not because I want it to (I vastly prefer tabs), but because `github`
  otherwise makes a mess of the source display, especially when tabs and spaces are mixed.

In this project:

- `VideoCore IV` is referred to as `vc4`
- `VideoCore VI` is referred to as `v3d`

This follows the naming convention as used in the linux kernel code and in the `Mesa` library.


### All development is done on **Raspbian 10 Buster**

In other words, the latest stable release is used. Upgrades are run regularly.

`V3DLib` should compile and run on Raspbian versions from **Jessie** onwards.
There are compatibility issues with previous versions (Wheezy and before), see the FAQ for known issues.

In general, don't bother with old distributions. Use the latest stable distro instead.


### Unit Testing 

Unit tests are run often and always before a push to the central repository on `github`
(although I may skip tests on some platforms if the fix is obvious).

All platforms mentioned below run `Raspbian Buster 32-bits`, unless otherwise specified.

The following platforms are used for unit testing:

- Raspberry Pi 4 Model B Rev 1.1, 32 as well as 64 bits
- Raspberry Pi 3 Model B Rev 1.2
- Raspberry Pi 2 - **TODO Set up**
- Raspberry Pi Model B Rev 2
- Raspberry Pi 4 Model B Rev 1.1,  aarch64 (ARM 64-bits)
- Debian Buster 64-bits on Intel i7  - with QPU=0, skips the hardware GPU tests (obviously)


### External Libraries

The following external libraries are used:

1. **[CmdParameter](https://github.com/wimrijnders/CmdParameter)** 

This is a library for handling command line parameters in a sane way, of my own making.
The installation is included in the makefile, so usually you do not need to think about it.

For explicit manual install, run script `scripts/install.sh` to clone its repo and to build it.
This script needs to be run before the initial build of `V3DLib`, and after that only
if the version number of `CmdParameter` changes.

The update is handled in the makefile, but you can rerun it manually if desired.
In that case, variable `expected_version` in script `scripts/install.sh` will probably also
need to be updated.


2. **[The Mesa 3D Graphics Library](https://gitlab.freedesktop.org/mesa/mesa)**

This is the go-to open source OpenGL library.

Only selected parts are used, notably the disassembly of `VideoCore` instructions.
The relevants part of the library have been cherry-picked and added to `V3DLib`.
An effort has been made to minimize the amount of code needed of this library, but it is still a lot.

The building of the `Mesa` code is part of the makefile.
This build step is the reason that the very first build takes a significantly longer time.


## Running with `sudo`

You need to run the example programs with sudo in the following situations:

- On Pis prior to `Pi 4`
- When using command line option `-pc`
- On a `Pi 4` when running on Raspbian 64-bits (`aarch64`). This can be avoided by running command:

    > sudo setfacl -m u:((your user name here)):rw /dev/dri/card*


## Version Numbering

The API is still a moving target and keeps on breaking.
Officially, I should be changing the major version continually, but with the project still in its infancy
and having exactly two users (Update 2021027: three!), I consider this overkill.
I up the minor version instead as a compromise.

I decided to go to `1.0.0` when the TODO list has been completely cleared.
Don't hold your breath, though, it is still very much a moving target.


## Basic Build Instructions

This demonstrates the build commands by example.
The flags `GPU` and `DEBUG` are explained below.

    > make help                     # Overview of specific build commands
    > make                          # same
    
    > make QPU=1 DEBUG=1 all        # Builds all examples in debug mode with GPU hardware support
    > sudo ./obj/qpu-debug/bin/GCD  # Run an example made with previous step. sudo usually required 
    
    > make QPU=1 GCD                # Builds example `GCD` in release mode with GPU hardware support.
    > sudo ./obj-qpu/bin/GCD        # Run the example made with previous step. sudo usually required
	
    > make QPU=1 DEBUG=1 test       # Run all tests, debug mode required
    
    > make QPU=1 DEBUG=1 make_test  # Same as previous, in two steps. Sometimes useful
    > ./obj/qpu-debug/bin/runTests
    
    > make clean                    # Remove all binaries and intermediate files.
                                    # This does *not* clean up external libraries.

The following scripts are also significant:

    > script/install.sh            # Clone, update and rebuild external libraries.
                                   # Run this on first build or when an external library changes
    
    > script/gen.sh                # Redo the file dependencies within the projectA.s
                                   # Run this when source files are added or removed during development


## Run Modes

There are three actual run modes and one convenience run mode for the example programs.
These can be selected with flag `-s=` on the command line.

The run modes are:

| Run Mode | Description |
| - | - |
| `interpreter` | interprets the source level code |
| `emulator`    | compiles to `vc4` code and runs this on a `vc4` emulator |
| `qpu`         | compiles to either `vc4` or `v3d` code, depending on which hardware you're running on, and runs on the GPU |
| `default`     | selects the most suitable platform to run on, depending on the build flags below and the hardware |

Note that there is no `v3d` emulator.

On a Pi-platform, `interpreter` and `emulator` are useful for asserting that the hardware output
is correct. You can expect, however, that these will run a *lot* slower than hardware.

A program that works in emulation mode but not on the physical GPU probably indicates a bug in `V3DLib`.
Please report this and hopefully a valid explanation can be found for the differences.


## Build flags

The makefile takes two flags:

- **DEBUG=1**  - enables debug information in the builds.
                 Values 0 (default) or 1
- **QPU=1**    - includes the code in the build for utilizing the GPU hardware
                 Values 0 (default) or 1.

Using `QPU=0` allows you to develop run code on non-Pi platforms.
Run modes `emulator` and `interpreter` will then still be available.

The build directory depends on the make flags passed. The combinations are:

| QPU | DEBUG | build directory |
| --- | ----- | --------------- |
| 0   | 0     | `obj/emu`       |
| 0   | 1     | `obj/emu-debug` |
| 1   | 0     | `obj/qpu`       |
| 1   | 1     | `obj/qpu-debug` |


## Compile Times

The first build can take a *long* time, especially on older Pi's.
The culprit here is mainly the included code from the `Mesa` library.

The following table lists the build times on all Pii versions.

| Platform | Make                 | Time    | Comment                            |
| -------- | -------------------- | ------- | -----------------------------------|
| Pi 1     | Full Initial build   | 170m    | Also builds the MESA code          |
|          | Library and examples |  22m    ||
| **REDO for doctest **         | Unit tests           |  22m    | without building the library       |
| Pi 2     | Full Initial build   |  65m    | Also builds the MESA code          |
|          | Library and examples |  12m    ||
|          | Unit tests           |   4m    | without building the library       |
| Pi 3     | Full Initial build   |  21m    | Also builds the MESA code          |
|          | Library and examples |   5m    ||
|          | Unit tests           |   1.5m  | without building the library       |
| Pi 4     | Full Initial build   |  13m    | Also builds the MESA code          |
|          | Library and examples |   3m    ||
|          | Unit tests           |   0.75m | without building the library       |


## CPU/GPU memory split

Depending on your plans, it may be useful to ensure that plenty of memory is available to the GPU.
The shared memory can be changed with `raspi-config`:

- `sudo raspi-config`
- select `Advanced Options`
- select `Memory Split`
- change the value

On a Raspberry Pi 1 Model B, 32MB seems to be the minimum that works.

The `Pi 4` uses a different shared memory model, for which a memory split is irrelevant.
