# Build Instructions

**TODO**

## Run Modes

There are three run modes for the example programs.
These can be selected in the examples with flagi `-s=` on the command line.

The run modes are:

- `interpreter` - interprets the source level code
- 'emulator'    - compiles to `vc4` code and runs this on a `vc4` emulator
- ``qpu`        - compiles to either `vc4` or v3d`, depending on which hardware you're running on,
                  and runs on the GPU.

Note that `interpreter` and `emulator` deal with `vc4	 code only.
The examples have a special run-mode `default`, which select the most suitable platform to run on,
depending on the build flags below and the hardware.

On a Pi-platform, `interpreter` and `emulator` are useful for asserting that the hardware output
is as correct. You can expect, however, that these will run a *lot* slower than hardware.


## Build flags

The makefile takes two flags:

- **DEBUG=1**  - enables debug information in the builds.
                 Values 0 (default) or 1
- **QPU=1**    - includes the code in the build for utilizing the GPU hardware
                 Values 0 (default) or 1.

Using `QPU=0` allows you to develop run code on non-Pi platforms.
Run modes `emulator` and `interpreter` will then still be available.

The output directory
depends on the make flags passed.  For example, `make all` will output to directory
`obj`, `make QPU=1 all` will output to directory `obj-qpu`.

Strictly speaking, any program that works in emulation mode but not on
the Pi's physical QPUs is probably a bug in `V3DLib` and should be
reported, although there may be valid explanations for such
differences.


##### CPU/GPU memory split

Depending on your plans, it may be useful to ensure that plenty of
memory is available to the GPU.  This can be done by using
`raspi-config`, selecting `Advanced Options` and then `Memory Split`:
(On a Raspberry Pi 1 Model B, 32M seems to be the minimum that works
for me.)
