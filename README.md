# V3DLib

Version 0.1.0.

`V3DLib` is a C++ library for easing the creation of programs to run on the `VideoCore` GPU of the [Raspberry Pi's](https://www.raspberrypi.org/)'s.

This project builds upon the [QPULib](https://github.com/mn416/QPULib) project, which is intended for the `VideoCore IV` GPU present in all RaspBerry Pi's prior to Pi 4.
The Pi 4, however, includes the `VideoCore VI` GPU, which is significantly different from its predecessor. I therefore felt the urge to enable support for the new GPU.

In conformance with the linux kernel, the `VideoCore IV` will be referred as `vc4`,
and the `VideoCore VI` as `v3d`.

`V3DLib` contains a high-level programming language and compilers for `vc4` and `v3d`.
These compile dynamically, so that a given program can run unchanged on any version of the RaspBerry Pi.
Kernel programs are generated inline offloaded programs to the GPU's at runtime.


**TODO**  *Quad Processing Units* (QPUs).

`V3DLib` is supported for Raspbian distributions from  `wheezy` onwards.
It has been tested on the `Pi 1 Model B`, the `Pi 2` and the `Pi 3 Model B`. **TODO** setup hardware testing.

## Local Links

* [Background](#background)
* [Examples](Doc/Examples.md).
* [References](#references)

## Credit where credit is due
**TODO**

## Getting Started

For more extensive details on building, see [Build Instructions](Doc/BuildInstructions.md).

`QPULib` requires an external library to compile.
Run the following to pull it in:

```
script/install.sh
```

In addition, the file depencies must be generated:

```
script/gen.sh
```
As long as the files don't change, you need to run this script only once.


Then:

```
sudo apt-get install git
git clone https://github.com/mn416/QPULib
make QPU=1 all
make test
...
make clean
```

- `QPU=1` will use the GPU to run kernels.
- Any other value, notably `QPU=0`, will run the kernels in an emulator


`make QPU=1 all` will build all the examples.
To build a specific example (e.g. `QCD`):

```
make QPU=1 QCD
sudo obj-qpu/bin/GCD
```

## Background

The
[QPU](http://www.broadcom.com/docs/support/videocore/VideoCoreIV-AG100-R.pdf)
is a [vector
processor](https://en.wikipedia.org/wiki/Vector_processor) developed by
[Broadcom](http://www.broadcom.com/) with
instructions that operate on 16-element vectors of 32-bit integer or
floating point values.
For example, given two 16-element vectors

`10 11 12 13` `14 15 16 17` `18 19 20 21` `22 23 24 25`

and

`20 21 22 23` `24 25 26 27` `28 29 30 31` `32 33 34 35`

the QPU's *integer-add* instruction computes a third vector

`30 32 34 36` `38 40 42 44` `46 48 50 52` `54 56 58 60`

where each element in the output is the sum of the
corresponding two elements in the inputs.

Each 16-element vector is comprised of four *quads*.  This is where
the name "Quad Processing Unit" comes from: a QPU processes one quad
per clock cycle, and a QPU instruction takes four consecutive clock
cycles to deliver a full 16-element result vector.

The Pi contains 12 QPUs in total, each running at 250MHz.  That's a
max throughput of 750M vector instructions per second (250M cycles
divided by 4 cycles-per-instruction times 12 QPUs).  Or: 12B
operations per second (750M instructions times 16 vector elements).
QPU instructions can in some cases deliver two results at a
time, so the Pi's QPUs are often advertised at 24
[GFLOPS](https://en.wikipedia.org/wiki/FLOPS).

The QPUs are part of the Raspberry Pi's graphics pipeline.  If you're
interested in doing efficient graphics on the Pi then you probably
want [OpenGL
ES](https://www.raspberrypi.org/documentation/usage/demos/hello-teapot.md).
But if you'd like to try accellerating a non-graphics part of your Pi
project then QPULib is worth a look.  (And so too are
[these references](#user-content-references).)



## References

The following works were *very* helpful in the development of
QPULib.

  * The [VideoCore IV Reference Manual](https://docs.broadcom.com/docs-and-downloads/docs/support/videocore/VideoCoreIV-AG100-R.pdf) by Broadcom. [Errata](https://www.elinux.org/VideoCore_IV_3D_Architecture_Reference_Guide_errata).

  * The [documentation, demos, and
    assembler](https://github.com/hermanhermitage/videocoreiv-qpu)
    by Herman Hermitage.

  * The [FFT implementation](http://www.aholme.co.uk/GPU_FFT/Main.htm)
    by Andrew Holme.
