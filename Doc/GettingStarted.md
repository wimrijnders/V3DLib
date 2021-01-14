# Getting started

**TODO: Get rid of this page**

The `QPU=1` flag to `make` indicates that the physical QPUs on the
Raspberry Pi should be used to run the kernels.  Using
`make` without `QPU=1`, or setting `QPU` to any value other than 1,
will lead to **emulation mode** being used.  As the name suggests,
this means that QPU code will be emulated in software.  This is useful
for debugging, and also allows you to run `V3DLib` programs on other platforms
than a Pi.

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
