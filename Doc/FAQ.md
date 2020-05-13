Frequently Asked Questions
--------------------------

This also serves as a central location for essential info.

# Table of Contents

- [What are the differences between VideoCore IV and VI?](#whatarethedifferencesbetweenvideocoreivandvi)
- [Function `compile()` is not Thread-Safe](#functioncompileisnotthreadsafe)
- [Float multiplication on the QPU always rounds downwards](#floatmultiplicationontheqpualwaysroundsdownwards)
- [Handling privileges](#handlingprivileges)

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
- Clock speed up to 500MHz from 400MHz
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

## Threads and Registers
- vc4: one or two threads per QPU.
  * one-thread mode: 64 registers in regsiter file
  * two-thread mode: 32 registers
- vc5: added a four thread per QPU mode, with 16 registers per thread.
- vc6:  doubled the size of the register file.
  * You can now use all 64 threads in two-thread mode and 32 registers in for thread mode.
  * Single thread mode removed, you always have at least two threads.

With the threading improvements, the QPUs should spent much less time idle waiting for memory requests.

## Number of QPU's
Indication that the number of QPU's has dropped to 8 (from 12).

### Calculated theoretical max FLOPs per QPU
Assuming theoretical max FLOPs per QPU per cycle is the same:

  - Pi3+ 12x2x400mhz = 9.6 GFLOPs
  - Pi4:  8x2x500mhz = 8.0 GFLOPs (less!)

Perhaps the driver is not reporting the correct number of QPUs.
But the improved hardware may compensate for this.

> vc6 does add multi-gpu-core support, each with their own set of QPUs, but the driver only appears to be reporting one core with 8 QPUs). My pi4 hasn't arrived yet so I haven't tested myself.
> However, I wouldn't be too surprised if the supporting hardware has been improved enough to extract more of the theoretical QPU performance into actual realised performance, allowing rendering performance improvements with less QPUs.


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
