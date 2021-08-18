# Architecture and Design
cycles to deliver a full 16-element result vector.

This document explains the basics of the QPU architecture and documents some design decisions within `V3DLib` to deal with it.

-----
# Naming

In conformance with the linux kernel, the following naming is used within the project:

- the `VideoCore IV` is referred to as `vc4`,
- the `VideoCore VI` as `v3d`.

By convention:

- A program running on a VideoCore is called a [(compute) kernel](https://en.wikipedia.org/wiki/Compute_kernel). I tend to leave out 'compute' when describing kernels.
- Values passed from a CPU program into a kernel are called *uniform values* or **uniforms**.


# Support

`V3DLib` is supported for Raspbian distributions from `wheezy` onwards.
Unit tests are run regularly on the following platforms:

| Platform             | Last unit test run |
| -------------------- | ------------------ |
| Pi 1 Model B         | |
| Pi 2                 | |
| Pi 3 Model B         | 20210718           |
| Pi 4 Model B 32-bits | |
| Pi 4 Model B 64-bits | |


# VideoCore Background

For an overview of the VideoCore functionality, please view the [Basics Page](Basics.md).

The VideoCore is a [vector processor](https://en.wikipedia.org/wiki/Vector_processor)
developed by [Broadcom](http://www.broadcom.com/) with
instructions that operate on 16-element vectors of 32-bit integer or
floating point values.

All Pi's prior to the `Pi 4` have a **VideoCore IV**. The `Pi 4` itself has a **VideoCore VI**,
which has many improvements. 

The basic hardware unit in the VideoCore is the **Quad Processing Unit (QPU)**.
The `Pi 4` had 8 of these, all previous Pi's have 12.
Due to the hardware improvements, the `Pi 4` GPU is still faster than in previous versions of the Pi.

The QPUs are part of the Raspberry Pi's graphics pipeline.  If you're
interested in doing efficient graphics on the Pi then you probably want
[OpenGL ES](https://www.raspberrypi.org/documentation/usage/demos/hello-teapot.md).
The added value of `V3DLib` is accelerating non-graphics parts of your Pi projects.


# QPU Registers

## Vector Offsets

In a kernel, when loading values in a register in a manner that would be considered intuitive for a programmer:

```c++
  Int a = 2;
```

...you end up with a 16-vector containing the same values:

    a = <2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2>

In order to use the vector processing capabalities effectively, you want to be able to perform the calculations
with different values.
The following functions at source code level are supplied to deal this:

### Function `index()`

Returns an index value unique to each vector element, in the range `0..15`.


### Function `me()`

Returns an index value unique to each QPU participating an a calculation.
A single running QPU would have `me() == 0`, any further QPU's are indexed sequentially.

### Function `numQPUs()` 

Returns  number of QPU's participating in a calculation.

For `vc4`, the number of QPU's is selectable between 1 and 12, 12 being the maximum.
The participating QPU's would then have `me() == 0, 1, 2...` up to the selected maximum.

For `v3d`, you can use either 1 or 8 QPU's. In the latter case, `me()` would return 0, 1, 2, 3, 4, 5, 6 or 7 per QPU.


### Vector offset calculation

The previous functions are useful to differentiate pointers to memory addresses.
The following is a method to load in consective values from shared main memory:

```c++
void kernel(Ptr<Int> x) {
  x = x + index();
  a = *x;
}
```

*Keep in mind that Int and Float values are 4 bytes. Pointer arithmetic takes this into account.*

The incoming value `x` is a pointer to an address in shared memory (i.e. accessible by both the CPU and the QPU's).
By adding `index()`, each vector element of `x` will point to consecutive values.
On the assignment to `a`, these consecutive values will be loaded into the vector elements of `a`.


When using multiple QPUs, you could load consecutive blocks of values into separate QPUs int the following way:

```c++
void kernel(Ptr<Float> x) {
  x = x + index() + (me() << 4);
  a = *x;
}
```


## Automatic Uniform Pointer Initialization

Adding `index()` to uniform pointers is so common in kernel code, that I made the following design decision:

**all uniform pointers are initialized with an index offset**

I.e. you don't need to do it yourself.

For most applications, the adjustment is useful and required.
However, it is good to be aware of this pointer adjustment, as it is conceivable
that you might need something different in your own code
(the FFT calculation is an example of this).

Automatic uniform pointer initialization places restrictions on pointer usage:

- All accessed memory blocks must be a number of elements which is a multiple of 16.

Not adhering to this may lead to reads and writes outside the memory blocks.
This is not necessarily fatal, but you can expect wild and unexpected results.


# DMA and VPM (`vc4` only)

*There's a shitload of complexity here, most of which I don't want to deal with.
This section documents the stuff I need to know now ('now' being a moving target).*

See the example program `DMA` for basic usage, which is a level deeper than the regular use in the
source language. Normally, you won't explicitly use DMA and VPM at all.

`DMA` is just what you would expect, given the acronym.

The `VPM` (Vertex Pipe Memory) is a 12KB storage buffer, used to load and save data processed by the QPUs.
This buffer is shared by all QPUs, so there's plenty of opportunity for screwups in accessing it in a 
multi-QPU situation.

There are several memory mappings possible for the VPM, use of which is pretty arcane
(see the `VC IV Architecture reference Guide`).
I don't want to delve into this and just stick to the way `V3DLib` uses the VPM (which has been
inherited from `QPULib`). In particular, `V3DLib` restricts itself to *horizontal* memory accesses.

Loading values from shared main memory to QPUs is a two-stage process:

1. Start a DMA load to VPM
2. Load values from VPM into QPU registers

Saving values from QPUs to shared main memory is the process in reverse.

## DMA Usage

DMA deals with *bytes*.

- DMA read/write can operate in parallel with QPU execution. You will need to juggle a bit for
  optimal performance.
- Per QPU, a single DMA read and a single DMA write can execute in parallel.
- Multiple reads need to be processed sequentially. Either you wait for the previous read to finish,
  of the subsequent read blocks until the current read is finished.
- Likewise for multiple writes.


### Load example in source language (taken from example `DMA`)

The DMA load is first configured, then you start it and wait for it to complete.

To load 16 consecutive 16-byte vectors from shared main memory to (byte) address 0 in the VPM:

```c++
dmaSetReadPitch(64);               // 64: size of one 16-byte vector
dmaSetupRead(HORIZ, 16, 0);        // 16: number of vectors to load; 0: target address in VPM

dmaStartRead(p);                   // p:  source pointer in shared memory
dmaWaitRead();                     // Wait until load complete
```

C++ pseudo code:
```c++
byte *p         = <assigned value>;
byte *vpm       = 0;
int pitch       = 64;
int num_vectors = 16;

for (int i = 0; i < num_vectors; i++) {
  for (int j = 0; j < pitch; j++) {
    *(vpm + j) = *(p + j);
  }
  vpm += pitch;
  p   += pitch;
}

```


### Store example in source language (adapted from example `DMA`)

The DMA store is first configured, then you start it and wait for it to complete.

This example is a bit more elaborate, to better illustrate how the stride works.
For full vector transfers, the call to `dmaSetWriteStride()` is removed, as well as 
the last parameter to `dmaSetupWrite()` (see example program `DMA`).

To move **the first 3 values** of 16 consecutive 16-byte vectors
from (byte) address 256 in VPM to shared main memory:

```c++
dmaSetWriteStride(13*4);           // Skip 13 values of vector
dmaSetupWrite(HORIZ, 16, 256, 3);  // 16:  number of vectors to handle;
                                   // 256: start address for read;
                                   // 3:   number of consecutive values to transfer
dmaStartWrite(p);                  // p:   target address in shared main memory
dmaWaitWrite();                    // Wait until store complete
```

Note the discrepancy in parameter types:
- in `dmaSetWriteStride()`, the parameter is in **bytes**
- in `dmaSetupWrite()`, the final parameter is in **words**, i.e. the number of 4-byte elements

This is a source language thing which could be changed, but I don't want to go there.
I just want to understand it.

C++ pseudo code:
```c++
// Assume that word size is 4 bytes

byte *p         = <assigned value>;
byte *vpm       = 256;
int num_elems   = 3;
int stride      = 13*4;  // i.e. 16 - num_bytes
int num_vectors = 16;

for (int i = 0; i < num_vectors; i++) {
  for (int j = 0; j < num_elems; j++) {
    *((word *) p) = *((word *) vpm);
    vpm += sizeof(word);
    p   += sizeof(word);
  }
  vpm += stride;
  p   += stride;
}

```


## VPM Usage

VPM deals with 64-byte *vectors*.

Sizes and VPM addresses are defined in terms of these vectors, i.e. a size of '1' means one 64-byte vector,
and and address of '1' means the VPM data location of the second vector in a sequence.

VPM load/store is configured initially; this sets up offset values which are updated on every access.

The example `DMA` combines the load and store in an artful manner.

### Example of loading values to QPU:

Load 16 consecutive values from VPM into a QPU register:

```c++
  Int a;                             // Variable which is assigned to a QPU register on compile
  vpmSetupRead(HORIZ, 16, 0);        // 16: number of vectors to load; 0: start index of vectors

  for (int i = 0; i < 16; i++) {     // Read each vector
   a = vpmGetInt();
   // Do some operation on value here
  }
```

C++ pseudo code:
```c++
vector *vpm_in  = 0;   // vector: 64-byte data structure; 0: starting index of vectors
int num_vectors = 16;

for (int i = 0; i < num_vectors; i++) {
  a = *vpm_in;
  vpm_in++;
  // Do some operation on value here
}
```


### Example of saving values from QPU:
```c++
  vpmSetupWrite(HORIZ, 16);          // 16: vector index to store at

  Int a = 0;
  for (int i = 0; i < 16; i++) {     // write back values of a
    vpmPut(a);
    a++;
  }
```

C++ pseudo code:
```c++
vector *vpm_out = 16;                // vector: 64-byte data structure; 16: starting index of vectors
int num_vectors = 16;

Int a = 0;
for (int i = 0; i < num_vectors; i++) {
  *vpm_out = a;
  vpm_out++;
  a += 1;
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


-----

# Things to Remember

*Just plain skip this if you're browsing. It's only useful for archaological purposes. Must rid myself of this hoarding tendency one day.*

## vc4 DMA write: destination pointer is impervious to offset changes

For `vc4`, when doing DMA writes, the index offset is taken into account
in the DMA setup, therefore there is no need to add it.
In fact, the added offset is completely disregarded.

This came to light in a previous version of the `DSL` unit test.
The following was done before a write (kernel source code):

```
  outIndex = index();
  ...
  result[outIndex] = res;
  outIndex = outIndex + 16;
```

I would expect DMA to write to wrong locations, **but it doesn't**.
The DMA write ignores this offset and writes to the correct location, i.e. just like:

```
  ...
  *result = res;
  ...
```

My working hypothesis is that only the pointer value for vector index 0 is used to
initialize DMA.


## Initialization of stride for `vc4` at the level of the target language

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
