
# TODO

- Checked items are implemented but not merged yet. the PR-s are pending.
- An item that is completed and merged to `development` is removed from the list.


## v3d

- [ ] Figure out when and how `sig_magic` and `sig_addr` are used.
      Clues: `mesa/src/compiler/vir_to_qpu.c`, `mesa/src/broadcom/qpu/qpu_disasm.c`
- [ ] Add performance counters (see `Rot3D` for usage under `vc4`


## Makefile

- [X] Enable debug-build, for debugging. Currently, an indication is given in the Makefile how to do this.

## Compile source code

- [ ] Following generation is wrong *(this is probably a v3d issue)*. Source code:

    *p = 4*(index() + 16*me());

  Target code:

    22: ACC1 <- 16
    23: ACC1 <- mul24(ACC1, A0)
    24: ACC1 <- add(S[ELEM_NUM], ACC1)
    25: ACC1 <- mul24(4, ACC1)
    26: S[VPM_WRITE] <- or(ACC1, ACC1)
    27: S[DMA_ST_ADDR] <- or(A2, A2)

  `v3d` assembly:

    or  r1, 0x41800000, 0x41800000; nop
    nop                  ; smul24  r1, r1, rf0
    eidx  r0             ; nop
    add  r1, r0, r1      ; nop
    nop                  ; smul24  r1, 4, rf0   // Should be `smul24 r1, 4, r1` or similar
    or  tmud, r1, r1     ; nop
    or  tmua, rf2, rf2   ; nop

  In assembled code, rf0 (QPU_NUM) gets reloaded, cancelling the previous operations
  The result is thus: `*p = 4*me()`




## Documentation

- [ ] Explanation code
  - [ ] 16-item vectors
  - [ ] Code generation, not direct execution
- [ ] Drill-down of the bare essentials for understanding `VideoCore`
- [x] Examples to separate page under Docs
- [ ] Mailbox functions link to reference and explanation two size fields
- [x] QPUs always round *downward* *(in Issue)*
- [ ] DSL: Use of 'Expr'-constructs, e.g. `BoolExpr`, as a kind of lambda


## Unit Tests

- [x] Add test on expected source and target output for pretty print in `compileKernel`. Done for `ReqRecv`
- [ ] Add tests to compare QPU and Emulation mode output (when build can be done with both)
- [ ] Language
  - [ ] Test missing `End` on blocks
  - [ ] Test missing `Else` without `If`
- [ ] Adjust emulator so it rounds downward like the hardware QPU's.
  Due to kernel rounding downward for floating point operations, unit tests comparing outputs
  in an emulator-only (QPU=0) build will fail. E.g.:

    Tests/testRot3D.cpp:33: FAILED:
      REQUIRE( y1[i] == y2[i] )
    with expansion:
      -19183.95117f == 19184.0f
    with message:
      Comparing Rot3D_2 for index 19184

  This error happens twice, for `testRot3D`.


## Unit Tests

- [x] Make display of Performance Counters a cmdline option for examples (see `Rot3D`)


## Investigate

- [ ] Is the gather limit 8 or 4? This depends on threading being enabled, check code for this.
- [ ] Improve heap implementation and usage. The issue is that heap memory can not be reclaimed. Suggestions:
  - Allocate `astHeap` for each kernel *- Nope, global heap required for language*
  - Increase heap size dynamically when needed *- Can only be done, by creating a new heap and transferring data*
  - Add freeing of memory to heap definitions. This will increase the complexity of the heap code hugely


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

- [ ] Add optional doc generation with `doxygen`. This is only useful if there are a sufficient number of header comments.
- [ ] Viewer for graphic output (e.g. `Mandelbrot`). Should be really simple to set up.
  - **NOTE:** `Qt` is a good candidate for this. However, while the viewer application itself may be simple, it
    requires a `Qt Creator` installation of 5.7GB. This might be total overkill.
  - Alternative to `Qt`: web-page accessing a minimal server.   
- [ ] Scheduling of kernels - see VideoCore `fft` project.


-----

## Principles

- QPU-mode should not compile on non-RPi platforms, default to emulator instead


## Stuff to Consider

### Measure performance in various ways

E.g. compare between:

  - different iterations of a program
  - number of QPUs used
  - RPi versions
  
  
### The QPULib compiler does not do much in the way of optimisation.

So the question is how far QPULib programs are off hand-written QPU assembly, and what we can do to get closer.


### Set up Guidelines for the Project

This blog contains great tips for setting up open source projects: 

- [The Bitter Guide to Open Source](https://medium.com/@ken_wheeler/a-bitter-guide-to-open-source-a8e3b6a3c1c4) by Ken Wheeler.

One day, I would like to convert the points mentioned here to a checklist and implement these for `QPULib`.
