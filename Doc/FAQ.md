Frequently Asked Questions
--------------------------

This also serves as a central location for essential info.

# Table of Contents

- [What are the differences between VideoCore IV and VI?](#whatarethedifferencesbetweenvideocoreivandvi)
- [Function `compile()` is not Thread-Safe](#functioncompileisnotthreadsafe)
- [Float multiplication on the QPU always rounds downwards](#floatmultiplicationontheqpualwaysroundsdownwards)
- [Handling privileges](#handlingprivileges)
- [Known limitations for old distributions](#knownlimitationsforolddistributions)

-----


# What are the differences between VideoCore IV and VI?

There is no architecture specification available yet for VC5 and/or VC6.
The stuff below is cobbled from whatever others have found out.
The strategy appears to be to investigate the available open source drivers.

[Source](https://www.raspberrypi.org/forums/viewtopic.php?t=244519)

## What remains the same
- The QPU pipeline stays mostly the same
- you still have an add ALU and a multiply ALU and it can issue two ALU OPs per cycle.
- There is still 4 SIMD lanes, interleaved over 4 cycles.
- The theoretical max FLOPs per QPU remains the same at two per cycle


## What is different

Here is an overview for the easily comparable stuff:

| Item                | vc4          | v3d | Comment |
|---------------------|--------------|-----|-|
| **Clock speed :**   | 400MHz  | 500MHz | |
| **Num QPU's:**      | 12  | 8 | |
| **Threads per QPU** | | | *Shows  num registers from register file per thread* |
| 1 thread | 64 registers |  *not supported* | 
| 2 threads     | 32 registers | 64 registers | |
| 4 threads | *not supported* | 32 registers | |

- vc5 added a four thread per QPU mode, with 16 registers per thread. vc5 was skipped in the Pi's.
- v3d doubled the size of the register file.


Further:

- vc6 is clearly derived from vc4, but it is significantly different. vc6 is only a slight extension over vc5
- The instruction encoding for the QPUs is different, but the core instructions are the same.
- Instructions for packed 8 bit int math has been dropped, along with most of the pack modes.
- Instructions for packed 16 bit float math has been added (2 floats at in a single operation)
- the multiply ALU can now fadd, so you can issue two fadds per instruction.
- the add ALU has gained a bunch of new instructions, that I don't recognise by name and I haven't explored.
- the A and B register files have been merged. You still only get an A read and a B read per instruction, but they read from one big register file (which means the underlying memory block has gone from two sets of "one read port, one write port" to one "two read ports, one write port" block)
- It looks like a lot of effort has been put putting the theoretical FLOPs to better use.
- Most of the design changes have gone to improving the fixed function hardware around the QPUs.
- A fixed function blend unit has been added, which should reduce load on the QPUs when doing alpha blending.
- Some concern if software blending is still possible.
- The tile buffer can now store upto 4 render targets (I think it's up to 128bits per pixel, so if you are using 4 32bit render targets, you can't have a depth buffer)
- Faster LPDDR4 memory.
- A MMU, allowing a much simpler/faster kernel driver.
- Many more texture formats, framebuffer formats.
- All the features needed for opengl es 3.2 and vulkan 1.1
- With the threading improvements, the QPUs should spent much less time idle waiting for memory requests.

### Calculated theoretical max FLOPs per QPU

From the [VideoCore® IV 3D Architecture Reference Guide](https://docs.broadcom.com/doc/12358545):

- The QPU is a 16-way SIMD processor.
- Each processor has two vector floating-point ALUs which carry out multiply and non-multiply operations in parallel with single instruction cycle latency.
- Internally the QPU is a 4-way SIMD processor multiplexed 4× (over PPU's) over four cycles, making it particularly suited to processing streams of quads of pixels.

So:

- 4 operations per clock cycle, when properly pipelined
- 2 ALU's per operation

So, calculation:

    flop/clock = 4 [PPU's] x 2 [ALU's] = 8
    GFLOPs = [Clock Speed (MHz)]x[num slices]x[qpu/slice]x[op/clock]

- VideoCore IV @ 250MHz: 250x3x4x8 = 24   GFLOPs
- VideoCore IV @ 300MHz: 300x3x4x8 = 28.8 GFLOPs
- Pi3+                 : 400x3x4x8 = 38.4 GFLOPs
- VideoCore VI @ 500MHz: 500x2x4x8 = 32   GFLOPs (less!)

- The improved hardware in `v3d` may compensate for performance.
- v3d adds multi-gpu-core support, each with their own set of QPUs. However, there is only one core in `v3d`.

-----
# Function `compile()` is not Thread-Safe
Function `compile()` is used to compile a kernel from a class generator definition into a format that is runnable on a QPU. This uses *global* heaps internally for e.g. generating the AST and for storing the resulting statements.

Because the heaps are global, running `compile()` parallel on different threads will lead to problems. The result of the compile, however, should be fine, so it's possible to have multiple kernel instances on different threads.

As long a you run `compile()` on a single thread at a time, you're OK.


**TODO:** examine further.


-----
# Float multiplication on the QPU always rounds downwards

Most CPU's make an effort to round up or down to the value nearest to the actual result of a multiplication. The `ARM` is one of those. The QPU's of the `VideoCore`, however, do not make such an effort: *they always round downward*.

This means that there will be small differences in the outputs of the exact same calculation on the CPU and a QPU; at first only in the least significant bits, but if you continue calculating, the differences will accumulate.

**Expect results to differ between CPU and QPU calculations.**

Of special note, the results between the `QPULib` interpreter and the actual hardware `VideoCore` will likely be different.


-----
# Handling privileges

In order to use the `VideoCore`, special privileges are required to access certain devices files.  The default way is to run the applications with `sudo`.

You might run into the following situation (e.g.):
```
> obj-qpu/bin/detectPlatform 
Detected platform: Raspberry Pi 2 Model B Rev 1.1
Can't open device file: /dev/vcio
Try creating a device file with: sudo mknod /dev/vcio c 100 0
```
The solution for this is to become a member of group `video`:
```
> sudo useradd -g video <user>
```

Where you fill in  a relevant user name for `<user>`. To enable this, logout and login, or start a new shell.

Unfortunately, this solution will not work for access to `/dev/mem`. You will still need to run with `sudo` for any application that uses the `VideoCore` hardware.


----
# Known limitations for old distributions

Following is known to occur with `Raspbian wheezy`.

## Certain expected functions are not defined yet

Notably, missing in in `/opt/vc/include/bcm_host.h`:

- `bcm_host_get_peripheral_address()`
-  `bcm_host_get_peripheral_size()`

There is a check on the presence of these function definitions in the Makefile; if not detected,
drop-in local functions will be used instead.

 However, the detection is not foolproof. If you get compilation errors anyway due to absence of these `bcm` functions, force the following makefile variable to 'no':

```make
#USE_BCM_HEADERS:= $(shell grep bcm_host_get_peripheral_address /opt/vc/include/bcm_host.h && echo "no" || echo "yes")
USE_BCM_HEADERS:=no       # <---- use this instead
```

## Known limitations with compiler

`Raspbian wheezy` uses `gcc` version *(@mn416 please supply your gcc version here!)*.
At time of writing, the code is compiled with `-std=c++0x`.

* This gcc version does not compile inline initialization of class variables (`C++11`standard):

```c++
class Klass {
   Klass(): m_value(0) {}   // <-- Use this instead

  int m_value{0};           // <-- This won't compile
}
```

* Some function definitions, which are found automatically in `c++11`, need explicit includes.

Known cases (there may be more):

| Function | Needs include |
|-|-|
| `exit(int)` | `#include <stdlib.h>` |
| `errno()` | `#include <errno.h>` |
| `printf()` etc | `#include <stdio.h>` |

