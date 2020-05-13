Frequently Asked Questions
--------------------------

This also serves as a central location for essential info.

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

: - Pi3+ 12x2x400mhz = 9.6 GFLOPs
: - Pi4:  8x2x500mhz = 8.0 GFLOPs (less!)

Perhaps the driver is not reporting the correct number of QPUs.
But the improved hardware may compensate for this.

> vc6 does add multi-gpu-core support, each with their own set of QPUs, but the driver only appears to be reporting one core with 8 QPUs). My pi4 hasn't arrived yet so I haven't tested myself.
> However, I wouldn't be too surprised if the supporting hardware has been improved enough to extract more of the theoretical QPU performance into actual realised performance, allowing rendering performance improvements with less QPUs. 