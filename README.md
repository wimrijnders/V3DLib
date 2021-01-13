# V3DLib

**Version 0.0.0**

`V3DLib` is a C++ library for creating programs to run on the `VideoCore` GPU's of all versions of the [Raspberry Pi](https://www.raspberrypi.org/).

Prior to the Pi 4, this meant compiling for just the `VideoCore 4` GPU.
The Pi 4, however, has a `VideoCore 6` GPU which, although related, is significantly different.
`V3DLib` compiles and assembles for both versions of the VideoCore GPU.

Kernel programs compile dynamically, so that a given program can run unchanged on any version of the RaspBerry Pi.
The kernels are generated inline and offloaded to the GPU's at runtime.

## First Release

When I started this project, I resolved to release at the point that all examples would compile
and run on the `VideoCore 6`.
I have finally reached this point; however, **there are caveats**.

There are some parts which will compile perfectly but not run properly; notably the `Mandelbrot` demo
will run *sometimes* on a `VideoCore 6`, and otherwise hang.
This is in part due to issues in the linux kernel, see the [Issues page](Doc/Issues.md).
There are also some unit tests which have the same problem, these are disabled when running on `VideoCore 6`.

I haven't been able to resolve these issues and I am waiting for a kernel update with fixes.
All code for the `VideoCore 4` compiles and runs fine.

## The Programming Language

`V3DLib` contains a high-level programming language for easing the pain of GPU-programming.
The following is an example of the language (the 'Hello' example):

```
#include "V3DLib.h"
#include "Support/Settings.h"

using namespace V3DLib;

V3DLib::Settings settings;


// Define function that runs on the GPU.
void hello(Ptr<Int> p) {
  *p = 1;
}


int main(int argc, const char *argv[]) {
  auto ret = settings.init(argc, argv);
  if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Construct kernel
  auto k = compile(hello);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(16);
  array.fill(100);

  // Invoke the kernel
  k.load(&array);
  settings.process(k);

  // Display the result
  for (int i = 0; i < array.size(); i++) {
    printf("%i: %i\n", i, array[i]);
  }
  return 0;
}
```


## Credit where Credit is Due
This project builds upon the [QPULib](https://github.com/mn416/QPULib) project, by **Matthew Naylor**.
I fully acknowledge his work for the Videcore 4 and am grateful for what he has achieved in setting
up the compilation and assembly.

`QPULib`, however, is no longer under development, and I felt the need to expand it to support
the VideoCore 6 as well. Hence, `V3DLib` was conceived.


## Getting Started

This assumes that you are building on a Raspberry Pi.

For more extensive details on building, see [Build Instructions](Doc/BuildInstructions.md).

    > sudo apt-get install git                                       # If not done already
    > git clone --depth 1 https://github.com/wimrijnders/V3DLib.git  # get only latest commit
                                                                     # TODO test
    > git clone --depth 1 ssh://wim@shanna/home/git_masters/QPULib   # local pull, for comparisong

    > cd V3DLib
	
    # As long as the files don't change, you need to run these script only once.
    > script/install.sh                                              # Pull in and build external library
    > script/gen.sh                                                  # Generate file dependencies
    # After this, it's sufficient to do just the following line for a build
	
    > make QPU=1 DEBUG=1 all                                         # Made debug version with hardware GPU support
                                                                     # First compile will take some time
    
    > make QPU=1 DEBUG=1 test                                        # Run the tests

## Useful Links
### References

The following works were *very* helpful in the development.

#### VideoCore 4
* The [VideoCore IV Reference Manual] by Broadcom. [Errata].
* The [documentation, demos, and assembler](https://github.com/hermanhermitage/videocoreiv-qpu)
  by Herman Hermitage.
* The [FFT implementation](http://www.aholme.co.uk/GPU_FFT/Main.htm)
  by Andrew Holme.
	
#### VideoCore 6
* [v3d driver code in the linux kernel repository] - of special interest: [v3d_gem.c],
  [v3d_drm.h], `vc4` on same level
* [MESA v3d driver] - [github], `vc4` on same level
* [py-videocore6](https://github.com/Idein/py-videocore6) - Python project hacking the `VideoCore 6`
* [Broadcom code for v3d] - [relevant part], not sure if this is also for `vc4`, 2010 so probably no
- [Source doc for registers] - contains registers not in the ref doc:
- [Broadcom VideoCore V QPU Instruction Set] - [translation]
- [Notowaga example code] - useful!
- [Linux doc for v3d] - this is vc6



### Tools

* [vcgencmd](https://www.raspberrypi.org/documentation/raspbian/applications/vcgencmd.md)

--------------------------

[VideoCore IV Reference Manual]: https://docs.broadcom.com/docs-and-downloads/docs/support/videocore/VideoCoreIV-AG100-R.pdf

[Errata]: https://www.elinux.org/VideoCore_IV_3D_Architecture_Reference_Guide_errata

[v3d driver code in the linux kernel repository]: https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/drivers/gpu/drm/v3d

[v3d_gem.c]: https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/drivers/gpu/drm/v3d/v3d_gem.c

[v3d_drm.h]: https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/uapi/drm/v3d_drm.h

[MESA v3d driver]: https://gitlab.freedesktop.org/mesa/mesa/-/tree/master/src/gallium/drivers/v3d)
[github]: https://github.com/intel/external-mesa/tree/master/src/gallium/drivers/v3d

[Broadcom code for v3d]: https://android.googlesource.com/kernel/bcm/+/android-bcm-tetra-3.10-kitkat-wear/drivers/char/broadcom/mm/v3d/

[relevant part]: https://android.googlesource.com/kernel/bcm/+/android-bcm-tetra-3.10-kitkat-wear/drivers/char/broadcom/mm/v3d/v3d_user.c#179

[Source doc for registers]: https://vc4-notes.tumblr.com/post/125039428234/v3d-registers-not-on-videocore-iv-3d-architecture]

[Broadcom VideoCore V QPU Instruction Set]: http://imrc.noip.me/blog/vc4/VC5_instruction_set/

[translation]: https://translate.google.com/translate?hl=en&sl=auto&tl=en&u=http%3A%2F%2Fimrc.noip.me%2Fblog%2Fvc4%2FVC5_instruction_set%2F

[Notowaga example code]: https://gist.github.com/notogawa/36d0cc9168ae3236902729f26064281d

[Linux doc for v3d]: https://dri.freedesktop.org/docs/drm/gpu/v3d.html
