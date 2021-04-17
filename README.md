# V3DLib

**Version 0.5.10**

`V3DLib` is a C++ library for creating programs to run on the VideoCore GPU's of all versions of the [Raspberry Pi](https://www.raspberrypi.org/).

Prior to the Pi 4, this meant compiling for just the `VideoCore IV` GPU.
The Pi 4, however, has a `VideoCore VI` GPU which, although related, is significantly different.
`V3DLib` compiles and assembles for both versions of the VideoCore GPU.

Kernel programs compile dynamically, so that a given program can run unchanged on any version of the RaspBerry Pi.
The kernels are generated inline and offloaded to the GPU's at runtime.


-----
In this project:

- `VideoCore IV` is referred to as `vc4`
- `VideoCore VI` is referred to as `v3d`

This follows the naming convention as used in the linux kernel code and in the `Mesa` library.

-----

## Getting Started

This assumes that you are building on a Raspberry Pi.

- Please look at the [Known Issues](Doc/BuildInstructions.md#known-issues), so you have an idea what to expect.
- For more extensive details on building, see [Build Instructions](Doc/BuildInstructions.md).
- **Fair Warning:** The first build can take a *long* time, especially on older Pi's.
See the Build Instructions for details.

```
> sudo apt-get install git                                       # If not done already

> sudo apt install libexpat1-dev                                 # You need this for one lousy include file

> git clone --depth 1 https://github.com/wimrijnders/V3DLib.git  # Get only latest commit
> cd V3DLib
> make QPU=1 DEBUG=1 all                                         # Make debug version with hardware
                                                                 # GPU support.
    
> make QPU=1 DEBUG=1 test                                        # Build and run the tests
```


## Code Example

`V3DLib` contains a high-level programming language for easing the pain of GPU-programming.
The following is an example of the language (the 'Hello' program):

```
#include "V3DLib.h"
#include "Support/Settings.h"

using namespace V3DLib;

V3DLib::Settings settings;


void hello(Int::Ptr p) {                          // The kernel definition
  *p = 1;
}


int main(int argc, const char *argv[]) {
  settings.init(argc, argv);

  auto k = compile(hello);                        // Construct the kernel

  Int::Array array(16);                           // Allocate and initialise the array shared between ARM and GPU
  array.fill(100);

  k.load(&array);                                 // Invoke the kernel
  settings.process(k);  

  for (int i = 0; i < (int) array.size(); i++) {  // Display the result
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
the `VideoCore VI` as well. Hence, `V3DLib` was conceived.


## Useful Links
### References

The following works were *very* helpful in the development.

#### VideoCore IV 
* The [VideoCore IV Reference Manual] by Broadcom. [Errata].
* The [documentation, demos, and assembler](https://github.com/hermanhermitage/videocoreiv-qpu)
  by Herman Hermitage.
* The [FFT implementation](http://www.aholme.co.uk/GPU_FFT/Main.htm)
  by Andrew Holme. [Blog](https://www.raspberrypi.org/blog/accelerating-fourier-transforms-using-the-gpu/)

#### VideoCore VI 
* [v3d driver code in the linux kernel repository] - [v3d in kernel on github].
  Of special interest: [v3d_gem.c], [v3d_drm.h]. `vc4` code is  on same level
* [MESA v3d driver] - [github], `vc4` on same level
* [py-videocore6](https://github.com/Idein/py-videocore6) - Python project hacking the `VideoCore VI`
* [Broadcom code for v3d] - [relevant part], not sure if this is also for `vc4`, 2010 so probably no
- [Source doc for registers] - contains registers not in the ref doc:
- [Broadcom VideoCore V QPU Instruction Set] - [translation]
- [Notowaga example code] - useful!
- [Linux doc for v3d] - this is vc6



### Tools

* [vcgencmd](https://www.raspberrypi.org/documentation/raspbian/applications/vcgencmd.md)
* [doctest](https://github.com/onqtam/doctest) - unit test framework, [Documentation links](https://github.com/onqtam/doctest#documentation)

--------------------------

[VideoCore IV Reference Manual]: https://docs.broadcom.com/docs-and-downloads/docs/support/videocore/VideoCoreIV-AG100-R.pdf
[Errata]: https://www.elinux.org/VideoCore_IV_3D_Architecture_Reference_Guide_errata
[v3d driver code in the linux kernel repository]: https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/drivers/gpu/drm/v3d
[v3d in kernel on github]: https://github.com/torvalds/linux/tree/master/drivers/gpu/drm/v3d
[v3d_gem.c]: https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/drivers/gpu/drm/v3d/v3d_gem.c
[v3d_drm.h]: https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/uapi/drm/v3d_drm.h
[MESA v3d driver]: https://gitlab.freedesktop.org/mesa/mesa/-/tree/master/src/gallium/drivers/v3d
[github]: https://github.com/intel/external-mesa/tree/master/src/gallium/drivers/v3d
[Broadcom code for v3d]: https://android.googlesource.com/kernel/bcm/+/android-bcm-tetra-3.10-kitkat-wear/drivers/char/broadcom/mm/v3d/
[relevant part]: https://android.googlesource.com/kernel/bcm/+/android-bcm-tetra-3.10-kitkat-wear/drivers/char/broadcom/mm/v3d/v3d_user.c#179
[Source doc for registers]: https://vc4-notes.tumblr.com/post/125039428234/v3d-registers-not-on-videocore-iv-3d-architecture]
[Broadcom VideoCore V QPU Instruction Set]: http://imrc.noip.me/blog/vc4/VC5_instruction_set/
[translation]: https://translate.google.com/translate?hl=en&sl=auto&tl=en&u=http%3A%2F%2Fimrc.noip.me%2Fblog%2Fvc4%2FVC5_instruction_set%2F
[Notowaga example code]: https://gist.github.com/notogawa/36d0cc9168ae3236902729f26064281d
[Linux doc for v3d]: https://dri.freedesktop.org/docs/drm/gpu/v3d.html
