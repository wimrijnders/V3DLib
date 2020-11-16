
# TODO

## v3d

- [ ] Has the timeout hang been fixed yet in the kernel driver? Check from time to time
- [ ] Figure out when and how `sig_magic` and `sig_addr` are used.
      Clues: `mesa/src/compiler/vir_to_qpu.c`, `mesa/src/broadcom/qpu/qpu_disasm.c`
- [ ] Add performance counters; examine python project for this.
- [ ] Fix unit test for Rot3D kernel 2 with >1 QPU


## vc4

- [ ] Consider replacing DMA transfers (notably, write) with TMU.
- [ ] Consider using device driver interface for vc4 - this will get rid of need for `sudo`


## Compile source code

- [ ] `If (a != b)` appears to do the same as  `any(a1 != b)`, verify
- [ ] Following generation is wrong *(this is probably an old v3d issue and not relevant any more - CHECK!)*.

Source code:

```
    *p = 4*(index() + 16*me());
```

  Target code:

```
    22: ACC1 <- 16
    23: ACC1 <- mul24(ACC1, A0)
    24: ACC1 <- add(S[ELEM_NUM], ACC1)
    25: ACC1 <- mul24(4, ACC1)
    26: S[VPM_WRITE] <- or(ACC1, ACC1)
    27: S[DMA_ST_ADDR] <- or(A2, A2)
```

  `v3d` assembly:

```
    or  r1, 0x41800000, 0x41800000; nop
    nop                  ; smul24  r1, r1, rf0
    eidx  r0             ; nop
    add  r1, r0, r1      ; nop
    nop                  ; smul24  r1, 4, rf0   // Should be `smul24 r1, 4, r1` or similar
    or  tmud, r1, r1     ; nop
    or  tmua, rf2, rf2   ; nop
```

  In assembled code, `rf0 (QPU_NUM)` gets reloaded, cancelling the previous operations
  The result is thus: `*p = 4*me()`


## Documentation

- [ ] Explanation code
  - [ ] 16-item vectors (see below)
  - [ ] Code generation, not direct execution
- [ ] Drill-down of the bare essentials for understanding `VideoCore`
- [x] Examples to separate page under Docs
- [ ] Mailbox functions link to reference and explanation two size fields
- [x] QPUs always round *downward* *(in Issue)*
- [ ] DSL: Use of 'Expr'-constructs, e.g. `BoolExpr`, as a kind of lambda


## Unit Tests

- [x] Add test on expected source and target output for pretty print in `compileKernel`. Done for `ReqRecv`
- [x] Add tests to compare QPU and Emulation mode output (when build can be done with both)
- [x] Make display of Performance Counters a cmdline option for examples (see `Rot3D`)
- [ ] Language
  - [ ] Test missing `End` on blocks
  - [ ] Test missing `Else` without `If`
- [ ] Adjust emulator so it rounds downward like the hardware QPU's.
  Due to kernel rounding downward for floating point operations, unit tests comparing outputs
  in an emulator-only (QPU=0) build will fail. E.g.:

```
    Tests/testRot3D.cpp:33: FAILED:
      REQUIRE( y1[i] == y2[i] )
    with expansion:
      -19183.95117f == 19184.0f
    with message:
      Comparing Rot3D_2 for index 19184

  This error happens twice, for `testRot3D`.
```


## Investigate

- [ ] Is the gather limit 8 or 4? This depends on threading being enabled, check code for this.
- [ ] Improve heap implementation and usage. The issue is that heap memory can not be reclaimed. Suggestions:
  - Allocate `astHeap` for each kernel *- Nope, global heap required for language*
  - Increase heap size dynamically when needed *- Can only be done by creating a new heap and transferring data*
  - Add freeing of memory to heap definitions. This will increase the complexity of the heap code hugely *(already done for BO's)*


## Library Code

- [ ] CMDLine
  - [x] Allow for chained blocks of parameter definitions
  - [ ] For display, sort the parameters (except for `--help`, which should be at top)
- [ ] Add check in emulator for too many `gather()` calls
- [x] Determine num QPUs from hardware
- [x] Add method to determine RPi hardware revision number via mailbox
- [ ] Add code for using the `Special Functions Unit (SFU)`, operations: `SQRT`, `RECIPSQRT`, `LOG`, `EXP` *(in progress)*
- [x] Add performance counters to interface of `RegisterMap`
- [ ] Add Logging class
- [ ] Add method for build/platform info, for display on startup of an application
- [ ] Make QPU execution timeout a runtime setting


## Other

- [x] `Rot3D` make various versions selectable on command line
- [ ] enable `-Wall` on compilation and deal with all the fallout
- [ ] Scan current docs for typos, good language
- [x] enable build for QPU and Emulation mode together


## Long Term

- [ ] Add optional doc generation with `doxygen`.
      This is only useful if there are a sufficient number of header comments.
- [ ] Scheduling of kernels - see VideoCore `fft` project.


-----

# Mysteries

## vc4 DMA write: destination pointer impervious to offset changes

Pointers are initialized on kernel startup to contain offsets.
For all intents and purposes, they are redefined as follows:

```
    p = p + 4*(index() + 16*me());
```

For `vc4` when doing DMA writes, this works a little differently;
the index offset is taken into account in the DMA setup, therefore there is no need to add it.

However, in a previous version of the DSL unit test, the following was done before a write (kernel source code):

```
  outIndex = index();
  ...
  result[outIndex] = res;
  outIndex = outIndex + 16;
```

This is the 'old', pre-`v3d` way of doing things. I would expect DMA to write to wrong locations.

**But it doesn't**

The DMA write ignores this offset and writes to the correct location, i.e. just like:

```
  ...
  *result = res;
  ...
```

This undoubtedly has something to do with DMA setup. I really have no patience to examine this.
As far as I'm concerned, DMA writes are old-school, and relevant only to `vc4` anyway.
If it works, it works.

I much prefer to focus on `v3d`, which uses only TMU for main memory access.
Maybe one day I'll rewrite the `vc4` assembly to do the same *(hereby noted as TODO)*.

-----

# Snippets

Stuff which is partially worked out and needs to be completed one day.


## Documentation blurb for vector offsets

**TODO:** Make this a coherent text and find a good place for it in the docs

All registers within a QPU are actually a stack of 16 registers. This is referred to as a `vector` in
the documentation.
They may contains different values, but the exact same code is used in parallel to perform calculations with them.

When uniform values are loaded, all elements of a vector receive the same value. If you run a program on these,
all vector elements will have identical values at every step of the way; this makes for boring duplication.

In addition, when using multiple QPU's for a calculation, this would result in each QPU performing exactly the
same calculation.

The following functions at source code level are supplied to deal this:

  - `index()`   - returns an index value unique to each vector element, in the range `0..15`.
  - `me()`      - return an index value unique to each QPU participating an a calculation.
                  A single running QPU would have `me() ==0`, any further QPU's are indexed sequentially.
  - `numQPUs()` - The number of QPU's participating in a calculation

For `vc4`, the number of QPU's is selectable between 1 and 12, 12 being the maximum.
The participating QPU's would then have `me() == 0, 1, 2...` up to the selected maximum.

For `v3d`, you can use either 1 or 8 QPU's. In the latter case, `me()` would return 0, 1, 2, 3, 4, 5, 6 or 7 per QPU. 

These two functions are used to differentiate pointers to memory addresses, in the following way:

```c++
void kernel(Ptr<Float> x) {
  x = x + index() + (me() << 4);
  ...
}
```

Incoming value `x` is a pointer to an address in shared memory (i.e. accessible by both the CPU and the QPU's).
It is assumed that this points to a memory block containing values which need to be processed by the QPU's.

What happens here, is that each vector element gets assigned an offset into this memory block. Therefore,
each vector element will access a different consecutive value.
In addition, an offset is added in jumps of 16 items per QPU, according to the QPU ID.
Each QPU will thus handle a distinct block of 16 consecutive values.

For multiple QPU's, you need to take an offset per QPU into account (called 'stride' in the code).
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


**This happens automatically for pointer passed in as uniforms**. There is therefore no need
to explicitly code this for uniform pointers.
It is, however, useful to be aware of this pointer adjustment, and it is conceivable that you might need to use it
in your own code.

This adjustment has been integrated in the pointer usage, because it is so frequently recurring that I consider it
to be the standard way of dealing with pointer. I have yet to encounter a case where a different approach is 
required (if you do encounter one, please let me know!).

This does place restrictions on pointer usage:

- All accessed memory blocks should have a number of elements which is a multiple of 16.
- If multiple QPU's are used for a calculation, the number of elements should be (num QPU's) * 16.

Not adhering to this will lead to reads and writes outside the memory blocks.
This is not necessarily fatal, but you can expect wild and unexpected results.


-----

# Principles

- QPU-mode should not compile on non-RPi platforms, default to emulator instead


## Stuff to Consider

### Measure performance in various ways

E.g. compare between:

  - different iterations of a program
  - number of QPUs used
  - RPi versions
  
  
### The QPULib compiler does not do much in the way of optimisation.

So the question is how far QPULib programs are off hand-written QPU assembly, and what we can do to get closer.
