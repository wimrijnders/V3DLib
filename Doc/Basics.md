# VideoCore Basics - What you need to know

In order to effectively program on the `VideoCore`, you need a working model of its functionality
in your head. This document describes the bare minimum you need to know.

In conformance with the linux kernel, the following naming is used:

- the `VideoCore IV` is referred to as `vc4`,
- the `VideoCore VI` as `v3d`.

The text applies to both `vc4` and `v3d`. Where there are differences, it will be explicitly noted.


## Registers

The basic storage location within the VideoCore is a register. It is a 32-bit wide location which
in the majority of cases contains an integer or float value.

The *single most important thing* to wrap your brain around is that a register is also **16 values deep**.
A single register represents 16 distinct values.

![Register](./images/basics/register.png)

Within the project, this stack of 16 32-bit wide values is called a **16-vector**, or **vector** for short.

A single instruction working on registers, will perform the operation on *all* register values, pairing
the elements by their position and working on each pair.

For example, an add operation working on two registers `RF0` and `RF1`:

    add RF0, RF0, RF1      // first operand is destination
	
with:

    RF0 = <10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25>
	RF1 = <20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35>

...will result in `RF0` having a value:

    RF0 = <30 32 34 36 38 40 42 44 46 48 50 52 54 56 58 60>

Every location in the vector has an associated offset, which in the source language code can be
specified with **index()**. For example:

    Where (index() < 8)
      RF0 = 2
    End
   
...will set locations 0..7 to value 2 in register `RF0`. The remaining locations will be unchanged.


## Register Files

There are 64 general registers organized in one or two **register files**. `vc4` has two register files
A and B, each containing 32 registers, `v3d` has a single register file with 64 registers.


![Register File(s) and accumulators](./images/basics/registers.png)

In addition, there are 6 **accumulators**, dubbed `r0` to `r5`. Usage of these can improve the
efficiency of the operations, especially for `vc4`. `r4` and `r5` have specific usage for IO and
some extended operations.


## Arithmetic Logic Units

Logically, there are two ALUs associated with the registers, the **Add ALU** and the **Mul ALU**.
They each have a set of different operations they can perform.
Input values are read from the registers, processed, and the output is again stored in a register.

![Arithmetic Logic Units](./images/basics/alus.png)

The naming comes from the fact that, originally, the Add ALU
performed the addition operations and the Mul ALU performed the multiplication operations.
On `v3d`, this is not true any more; the Mul ALU can also do addition.

The ALUS work independently and execute in parallel. By carefully combining operations,
the performance can be improved by using the Add and Mul ALUs simultaneously.


## Processing Units

At the risk of complicating the simple model, I will take it one step deeper.

Instead of one Add and one Mul ALU, there are actually four **processing units** that perform the ALU operations.

![Arithmetic Logic Units](./images/basics/processing_units.png)

The processing units perform the same operation in parallel, but they work on different elements of the handled 16-vectors.
Each processing unit operates on one index offset of the 16-vectors per go. This takes one clock cycle.
So, per clock cycle, four indexes are handled, and it takes four clock cycles to process all used 16-vectors.

In practise, this is completely transparent and you do not need to take it into account while programming.
Retaining a model with one Add and one Mul ALU is sufficient.
Regard it as an explanation for why ALU operations take four clock cycles.


## Quad Processing Unit (QPU)

The basic hardware unit in the VideoCore is called a **Quad Processing Unit (QPU)**.
The name comes from the fact that each unit contains four processing units performing the operations.

![QPU](./images/basics/qpu.png)

There are multiple QPUs present in the VideoCore, `vc4` has 12 and `v3d` has 8.
The reason that `v3d` is faster anyway is largely due to hardware improvements.

QPUs operate totally independently, even if they are executing the same program.
In theory, it is possible that they execute different programs in parallel.
In practise, they each can take a different path through a given program.

Every QPU has an associated index, which in the source language code can be
obtained with **me()**. In addition, the number of running QPUs per program is given by **numQPUs()**.
These can be used to distribute a given calculation over multiple QPUs.

For example, consider a two-dimensional array, of which each row is processed by one QPU.
The following loop can be specified:

    For (Int row = me(), row < row_size, row += numQPUs())
	  // Do something useful per row
	End


## Slices and TMU

The QPUs are organized into **slices**. Each slice contains four QPUs.

![Slice](./images/basics/slice.png)

`vc4` has three slices, `v3d` has two.

A slice had some additional functionality, shared by the QPUs within the slice.
The important bit for us is the **Texture and Memory Lookup Unt (TMU)**, which fetches and stores value to/from main memory.
The name implies that it is used for textures, but any kind of value can be fetched and stored.

The TMU is a FIFO buffer for memory lookups. You specify the addresses of the values you want and
do a fetch request.
The values are stored sequentially in the TMU and are read in the order they are retrieved.

TMU fetch operates independently from the QPUs. It is possible to prefetch values, continue with
calculations on the QPUs and load the values later on. This is a neat way to increase throughput.

On `vc4`, the TMU is *read-only*. On `v3d`, you can also do TMU writes.
