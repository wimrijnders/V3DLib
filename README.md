# V3DLib

Version 0.1.0.

`V3DLib` is a C++ library for easing the creation of programs to run on the `VideoCore` GPU's of all versions of the [Raspberry Pi](https://www.raspberrypi.org/).

Prior to the Pi 4, this meant compiling for the `VideoCore 4` GPU. The Pi 4, however, has a `VideoCore 6 GPU`, which is significantly differen.

`V3DLib` contains a high-level programming language and compilers for both version of the VideoCore GPU.
These compile dynamically, so that a given program can run unchanged on any version of the RaspBerry Pi.
Kernel programs are generated inline and offloaded to the GPU's at runtime.


## Credit where Credit is Due
This project builds upon the [QPULib](https://github.com/mn416/QPULib) project, by *Matthew Naylor*.
I fully acknowledge his work for the Videcore 4 and am grateful for the work he has done in setting
up the compilation and assembly.

`QPULib`, however, is no longer under development, and I felt the need to expand it to support
the VideoCore 6 as well. Hence, `V3DLib` was conceived.


## Getting Started

This assumes that you are building on a Raspberry Pi.

For more extensive details on building, see [Build Instructions](Doc/BuildInstructions.md).

    > sudo apt-get install git  # If not done already
    > git clone --depth 1 https://github.com/wimrijnders/V3DLib.git  # get only latest commit
                                                                     # TODO test
    > git clone --depth 1 ssh://wim@shanna/home/git_masters/QPULib   # local pull, for comparisong

    > cd V3DLib
	
    # As long as the files don't change, you need to run these script only once.
    > script/install.sh  # Pull in and build external library
    > script/gen.sh      # Generate file dependencies
    # After this, it's sufficient to do just the following line for a build
	
    > make QPU=1 DEBUG=1 all  # Made debug version with hardware GPU support
                              # First compile will take some time
    
    > make QPU=1 DEBUG=1 test # Run the tests


## References

The following works were *very* helpful in the development of
QPULib.

  * The [VideoCore IV Reference Manual](https://docs.broadcom.com/docs-and-downloads/docs/support/videocore/VideoCoreIV-AG100-R.pdf) by Broadcom. [Errata](https://www.elinux.org/VideoCore_IV_3D_Architecture_Reference_Guide_errata).

  * The [documentation, demos, and
    assembler](https://github.com/hermanhermitage/videocoreiv-qpu)
    by Herman Hermitage.

  * The [FFT implementation](http://www.aholme.co.uk/GPU_FFT/Main.htm)
    by Andrew Holme.
