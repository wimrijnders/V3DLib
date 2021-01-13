# Architecture and Design

This document explains the basics of the QPU architecture and documents some design decisions within `V3DLib` to deal with it.

-----
# Naming

In conformance with the linux kernel, the following naming is use within the project:
- The `VideoCore IV` is referred to as `vc4`,
- the `VideoCore VI` as `v3d`.

# Support

`V3DLib` is supported for Raspbian distributions from  `wheezy` onwards.
It has been tested on the:
- `Pi 1 Model B`
- `Pi 2`
- `Pi 3 Model B`
- `Pi 4 Model B`

*....although I must confess that my Pi 1 and Pi 2 have died in the meantime. I will get new Pi's to make the test platform complete.*

**TODO** setup full hardware testing.


# VideoCore Background

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


# QPU Registers

**TODO:** Make this a coherent text.

All registers within a QPU are actually a stack of 16 registers. This is referred to as a **vector** in the code and  documentation.
The vector elements may contains different values, but the exact same code is used in parallel to perform calculations with them.


## Vector Offsets

When uniform values are loaded, all elements of a vector receive the same value.
If you then run a kernel, all vector elements will have identical values at every step of the way;
this makes for boring duplication.

In addition, when using multiple QPU's for a calculation, this would result in each QPU performing exactly the
same calculation.

The following functions at source code level are supplied to deal this:

### Function `index()`

Returns an index value unique to each vector element, in the range `0..15`.


### Function `me()`

Returns an index value unique to each QPU participating an a calculation.
A single running QPU would have `me() ==0`, any further QPU's are indexed sequentially.

### Function `numQPUs()` 

Returns  number of QPU's participating in a calculation.

For `vc4`, the number of QPU's is selectable between 1 and 12, 12 being the maximum.
The participating QPU's would then have `me() == 0, 1, 2...` up to the selected maximum.

For `v3d`, you can use either 1 or 8 QPU's. In the latter case, `me()` would return 0, 1, 2, 3, 4, 5, 6 or 7 per QPU.

### Vector offset calculation

The functions are used to differentiate pointers to memory addresses, in the following way:

```c++
void kernel(Ptr<Float> x) {
  x = x + index() + (me() << 4);
  ...
}
```

The incoming value `x` is a pointer to an address in shared memory (i.e. accessible by both the CPU and the QPU's).
It is assumed that this points to a memory block containing values which need to be processed by the QPU's.

What happens here, is that each vector element gets assigned an offset into this memory block. Therefore,
each vector element will access a different consecutive value.
In addition, an offset is added in jumps of 16 items per QPU, according to the QPU ID.
Each QPU will thus handle a distinct block of 16 consecutive values.

For multiple QPU's, you would need to take an offset per QPU into account (called 'stride' in the code).
This can be done as follows:

```c++
void kernel(Ptr<Float> x) {
  Int stride = numQPUs() << 4;
  x = x + index() + (me() << 4);

  ...
	// Perform some calculation
  ...

	x = x + stride;   // Prepare for handling the next block 
  ...
}
```

## Design decision: Automatic uniform pointer initialization

The uniform pointers are initialized with vector offsets on the execution of a kernel.
The stride is added implicitly to all uniform pointers on initialization of a kernel.
There is therefore no need to explicitly do this yourself.

It is, however, useful to be aware of this pointer adjustment, and it is conceivable that you might need to use it
in your own code.

This adjustment has been integrated in the pointer usage, because it is so frequently recurring that I consider it
to be the standard way of dealing with pointers.

Automatic uniform pointer initialization places restrictions on pointer usage:

- All accessed memory blocks should have a number of elements which is a multiple of 16.
- If multiple QPU's are used for a calculation, the number of elements should be (num QPU's) * 16.

Not adhering to this will lead to reads and writes outside the memory blocks.
This is not necessarily fatal, but you can expect wild and unexpected results.

**Unfortunately:**

It turns out that this default case is useful for only the basic usage. For more sophisticated use of pointers, this becomes a hindrance.

For example, the example programs `HeatMap` and `Mandelbrot` deal with uniform pointers differently, and actually
need to unset the automatic uniform pointer adjustment to work.

I'm not too happy about this. I'll keep the uniform pointer initialization in for now, until I think of a better way.

### Previous Attempts

Code here has been preserved for sentimental and archaological reasons.
I'm retaining it to preserve the insights encountered along the way, should I ever need to do something similar again.

#### Initialization of stride for `vc4` at the level of the target language

This places the initialization code in the INIT-block, after translation of source to target.
The INIT-block therefore needs to be added first.

This did not work for DMA, because DMA add the offset itself.

```
void SourceTranslate::add_init(Seq<Instr> &code) {
	using namespace V3DLib::Target::instr;

	int insert_index = get_init_begin_marker(code);

	Seq<Instr> ret;

	// When DMA is used, the index number is compensated for automatically, hence no need for it
	// offset = 4 * ( 16 * qpu_num);
	ret << shl(ACC0, rf(RSV_QPU_ID), 4 + 2)
	    << add_uniform_pointer_offset(code);

	code.insert(insert_index + 1, ret);  // Insert init code after the INIT_BEGIN marker
}
```
-----

# Setting of Branch Conditions

**TODO:** Make this a coherent text.

Source: qpu_instr.h, line 74, enum v3d_qpu_uf:

How I interpret this:
  - AND: if all bits set
  - NOR: if no bits set
  - N  : field not set
  - Z  : Field zero
  - N  : field negative set
  - C  : Field negative cleared

What the bits are is not clear at this point.
These assumptions are probably wrong, but I need a starting point.

**TODO:** make tests to verify these assumptions (how? No clue right now)

So:
  - vc4 `if all(nc)...` -> ANDC
  - vc4 `nc` - negative clear, ie. >= 0
  - vc4 `ns` - negative set,   ie.  < 0
