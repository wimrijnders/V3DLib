Frequently Asked Questions
--------------------------

This serves as a central location for essential info.

**NOTE:**
VideoCore VI is mostly referred to as `v3d`, because this is how it is named in the linux kernel code and in the `Mesa` library.
For the same reason, VideoCore IV is refered to as `vc4`.


# Table of Contents

- [What are the differences between VideoCore IV and VI?](#whatarethedifferencesbetweenvideocoreivandvi)
- [Differences in execution](#differencesinexecution)
- [Calculated theoretical max FLOPs per QPU](#calculatedtheoreticalmaxflopsperqpu)
- [Function `compile()` is not Thread-Safe](#functioncompileisnotthreadsafe)
- [Handling privileges](#handlingprivileges)
- [Known limitations for old distributions](#knownlimitationsforolddistributions)

-----


# What are the differences between VideoCore IV and VI?

There is no architecture specification available yet for VC5 and/or VC6.
The stuff below is cobbled from whatever I and others have found out.
The strategy appears to be to investigate the available open source drivers.

[Source](https://www.raspberrypi.org/forums/viewtopic.php?t=244519)

## What remains the same
- The QPU pipeline stays mostly the same
- you still have an add ALU and a multiply ALU and it can issue two ALU OPs per cycle.
- There is still 4 SIMD lanes, interleaved over 4 cycles.
- The theoretical max FLOPs per QPU remains the same at two per cycle


## What is different

Here is an overview for the easily comparable stuff:

| Item                | vc4             | v3d              | Comment |
|---------------------|-----------------|------------------|-|
| **Clock speed :**   | 400MHz (Pi3+)   | 500MHz           | |
| **Num QPU's:**      | 12              | 8                | |
| **Threads per QPU** |                 |                  | *Shows num available registers in register file per thread* |
| 1 thread            | 64 registers    |  *not supported* | |
| 2 threads           | 32 registers    | 64 registers     | |
| 4 threads           | *not supported* | 32 registers     | |

- vc5 added a four thread per QPU mode, with 16 registers per thread. vc5 was skipped in the Pi's.
- v3d doubled the size of the register file (A and B combined).


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

## Differences in execution

This section records differences between the `vc4` and `v3d` QPU hardware and consequently in the instructions.

The `vc4`-specific items can be found in the "VideoCore IV Architecture Reference Guide";
the corresponding `v3d` stuff has mostly been found due to empirical research and hard thinking.


### Setting of condition flags

- `vc4` - all conditions are set together, on usage condition to test is specified
- `v3d` - a specific condition to set is specified, on usage a generic condition flag is read

To elaborate:

**vc4**

Each vector element has three associated condition flags:

- `N` - Negative
- `Z` - Zero
- `C` - Complement? By the looks of it `>= 0`, but you tell me

These are set with a single bitfield in an ALU instruction.
Each flag is explicitly tested in conditions.

See: "VideoCore IV Architecture Reference Guide", section "Condition Codes", p. 28.

**v3d**

- Each vector element has two associated condition flags: `a` and `b`

To set, a specific condition is specified in an instruction and the result is stored in `a`.
The previous value of `a` is put in `b`.

See: My brain after finally figuring this out.


### Float multiplication

- `vc4`: Float multiplication on the QPU always rounds downwards
- `v3d`: Float multiplication rounds to the nearest value of the result

In other words, `v3d` will multiply as you would normally expect. The result will be identical to float multiplication on the `ARM` processor.
With `vc` however, small differences can creep in, which can accumulate with continued computation.

**Expect results to differ between CPU and QPU calculations for `vc4`.**

Of special note: the interpreter and emulator run on the ARM CPU, meaning that the outcome may be different from that from the `vc4` QPU's .


### Integer multiplication

- `vc4`: multiplication of negative integers will produce unexpected results
- `v3d`: works as expected

The following source code statements yield different results for `vc4` and `v3d`

```
    a = 16
    b = -1 * a
```

- For `vc4`, the result is `268435440`
- For `v3d`, the result is `-16`

This has to do with the integer multiply instruction working only on the lower 24 bits of integers.
Thus, a negative value gets its ones-complement prefix chopped off, and whatever is left is treated as an integer.


-----
# Calculated theoretical max FLOPs per QPU

From the [VideoCore® IV 3D Architecture Reference Guide](https://docs.broadcom.com/doc/12358545):

- The QPU is a 16-way SIMD processor.
- Each processor has two vector floating-point ALUs which carry out multiply and non-multiply operations in parallel with single instruction cycle latency.
- Internally the QPU is a 4-way SIMD processor multiplexed 4× (over PPU's) over four cycles, making it particularly suited to processing streams of quads of pixels.

So:

- 4 operations per clock cycle, when properly pipelined
- 2 ALU's per operation, when instructions uses both

So, calculation:

    op/clock per QPU = 4 [PPU's] x 2 [ALU's] = 8
    GFLOPs           = [Clock Speed (MHz)]x[num slices]x[qpu/slice]x[op/clock]

- Pi2  : 250x3x4x8 = 24.0 GFLOPs
- Pi3  : 300x3x4x8 = 28.8 GFLOPs
- Pi3+ : 400x3x4x8 = 38.4 GFLOPs
- Pi4  : 500x2x4x8 = 32.0 GFLOPs (less!)

- The improved hardware in `v3d` may compensate for performance.
- v3d adds multi-gpu-core support, each with their own set of QPUs. However, there is only one core in `v3d`.


-----
# Function `compile()` is not Thread-Safe
Function `compile()` is used to compile a kernel from a class generator definition into a format that is runnable on a QPU. This uses *global* heaps internally for e.g. generating the AST and for storing the resulting statements.

Because the heaps are global, running `compile()` parallel on different threads will lead to problems. The result of the compile, however, should be fine, so it's possible to have multiple kernel instances on different threads.

As long a you run `compile()` on a single thread at a time, you're OK.


**TODO:** examine further.


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

