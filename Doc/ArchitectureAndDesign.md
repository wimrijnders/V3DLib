# Architecture and Design

This document explains the basics of the QPU architecture and documents some design decisions within `V3DLib` to deal with it.

-----
# Naming

In conformance with the linux kernel, the following naming is use within the project:

- the `VideoCore IV` is referred to as `vc4`,
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
[VideoCore IV](http://www.broadcom.com/docs/support/videocore/VideoCoreIV-AG100-R.pdf)
is a [vector processor](https://en.wikipedia.org/wiki/Vector_processor)
developed by [Broadcom](http://www.broadcom.com/) with
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

The Pis prior to `Pi 4` contain 12 QPUs in total, each running at 250MHz.
That is a maximum throughput of 750M vector instructions per second (250M cycles
divided by 4 cycles-per-instruction times 12 QPUs).
Or: 12B operations per second (750M instructions times 16 vector elements).
QPU instructions can in some cases deliver two results at a
time, so the Pi's QPUs are often advertised at 24
[GFLOPS](https://en.wikipedia.org/wiki/FLOPS).

The `Pi 4` has 8 QPUs, also running at 250MHz. Due to hardware improvements, the GPU
is still faster than in previous versions of the Pi.

The QPUs are part of the Raspberry Pi's graphics pipeline.  If you're
interested in doing efficient graphics on the Pi then you probably want
[OpenGL ES](https://www.raspberrypi.org/documentation/usage/demos/hello-teapot.md).
The added value of `V3DLib` is accelerating non-graphics parts of your Pi projects.


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


# Design decisions

## Automatic uniform pointer initialization

The uniform pointers are initialized with vector offsets on the execution of a kernel.
The stride is added implicitly to all uniform pointers on initialization.
There is therefore no need to explicitly do this yourself.

It is, however, useful to be aware of this pointer adjustment, and it is conceivable
that you might need to use it in your own code.

This adjustment has been integrated in the pointer usage, because it is so frequently
recurring that I consider it to be the standard way of dealing with pointers.

Automatic uniform pointer initialization places restrictions on pointer usage:

- All accessed memory blocks should have a number of elements which is a multiple of 16.

Not adhering to this will lead to reads and writes outside the memory blocks.
This is not necessarily fatal, but you can expect wild and unexpected results.

### Previous Attempts

Code here has been preserved for sentimental and archaological reasons.
I'm retaining it to preserve the insights encountered along the way, should I ever need to do something similar again.

#### Initialization of stride for `vc4` at the level of the target language

This places the initialization code in the INIT-block, after translation of source to target.
The INIT-block therefore needs to be added first.

The pointer offset is only effective for TMU accesses.
When VPM/DMA is used, the index number is compensated for automatically, hence no need for it.

```
void SourceTranslate::add_init(Seq<Instr> &code) {
	using namespace V3DLib::Target::instr;

	int insert_index = get_init_begin_marker(code);

	Seq<Instr> ret;

/*
    // Previous version, adding an offset for multiple QPUs
	// This was silly idea and has been removed. Kept here for reference
    // offset = 4 * (vector_id + 16 * qpu_num);
    ret << shl(ACC1, rf(RSV_QPU_ID), 4) // Avoid ACC0 here, it's used for getting QPU_ID and ELEM_ID (next stmt)
        << mov(ACC0, ELEM_ID)
        << add(ACC1, ACC1, ACC0)
        << shl(ACC0, ACC1, 2)           // offset now in ACC0
        << add_uniform_pointer_offset(code);
*/

  // offset = 4 * vector_id;
  ret << mov(ACC0, ELEM_ID)
      << shl(ACC0, ACC0, 2)             // offset now in ACC0
      << add_uniform_pointer_offset(code);
		

	code.insert(insert_index + 1, ret);  // Insert init code after the INIT_BEGIN marker
}
```

## Strict versus Relaxed Definition of the Target Language

*Adapted from text by Matthew Naylor, originally in `Target/Syntax.h`*

This abstract syntax is a balance between a strict and relaxed
definition of the target language:
 
- a "strict" definition would allow only instructions that can run on
the target machine to be expressed
- a "relaxed" one allows
instructions that have no direct mapping to machine instructions.

A relaxed definition allows the compilation process to be incremental:
after each pass, the target code gets closer to being executable, by
transforming away constructs that do not have a direct mapping to
hardware.  However, we do not want to be too relaxed, otherwise we
lose scope for the type checker to help us.

For example, the definition below allows an instruction to read two
operands from the *same* register file.  In fact, two operands must be
taken from different register files in the target language. It is the
job of a compiler pass to enforce such a constraint.


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
