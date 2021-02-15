# TODO

## General

- [x] `vc4` set TMU transfer as default. Selecting DMA should still be possible (also unit test it).
- [ ] Automate loading of new versions external libraries
- [ ] Complete conversion `Seq<Instr>` to `Instr::List`
- [ ] Get `Pi 1` running again; fails in `qpu_enable()`
- [ ] Figure out segfault with imm(15) in immediates unit test; happens on `pi4 32b`
- [ ] Make heap memory size configurable (ideally cmldine option)
- [x] ! Fix '+ 0' hack for kernel pointers, this is confusing
- [ ] Find a way to detect `For....}` issue. Should terminate with `End` but compiles fine.
- [x] Refactor derived settings in examples, too much duplicated screen noise.
- [x] Fix indentation tabs/spaces
- [ ] Tone down mesa library, compile takes long.
      Tried this but gave up after it became evident nothing could be removed.
      Perhaps leave out the `*.c` files? Not looking forward to it, lots of work.

## Consider these

- [ ] Option for disabling L2 cache, for decent cooperation with `OpenGL`.
      **TODO** profile this!
- [ ] Adding **Navier-Stokes** as an example.
      [This document](http://graphics.cs.cmu.edu/nsp/course/15-464/Fall09/papers/StamFluidforGames.pdf)
      looks promising.
- [ ] Implement [Raytracing](https://gabrielgambetta.com/computer-graphics-from-scratch/02-basic-raytracing.html).
- [ ] Make [ARCHITECTURE.md](https://matklad.github.io//2021/02/06/ARCHITECTURE.md.html) - [example](https://github.com/rust-analyzer/rust-analyzer/blob/master/docs/dev/architecture.md)
- [ ] Use [inherited enums](https://stackoverflow.com/questions/644629/base-enum-class-inheritance#644651) - for isolating DMA stuff
- [ ] Implement DFT (and then FFT) - consider [sliding windows](https://github.com/glidernet/ogn-rf/issues/36#issuecomment-775688969)


## v3d

- [ ] Has the timeout hang been fixed yet in the kernel driver? Check from time to time
- [ ] Figure out when and how `sig_magic` and `sig_addr` are used.
      Clues: `mesa/src/compiler/vir_to_qpu.c`, `mesa/src/broadcom/qpu/qpu_disasm.c`
- [x] Add performance counters; examine python project for this.
  * [ ] figure out what the counters signify on `v3d`
- [x] Fix unit test for Rot3D kernel 2 with >1 QPU


## vc4

- [ ] Consider using device driver interface for vc4 - this will get rid of need for `sudo`
- [ ] Enforce acc4 (r4) as a read-only register, notably in emulator
- [ ] Enforce non-usage of acc4 (r4) during sfu-call, notably in emulator


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

- [x] Is the gather limit 8 or 4? This depends on threading being enabled, check code for this.
  * **Answer:** 8 for single threading, less for multi-threading, but we don't do multi
  * **Answer:** 4 for `vc4`, 8 for `v3d`
- [ ] Improve heap implementation and usage. The issue is that heap memory can not be reclaimed. Suggestions:
  - [x] Add freeing of memory to `SharedArray` heap. This will increase the complexity of the heap code hugely
  - [x] Get rid of AST heap
	- [x] Fix unfreed elements of `Stmt` (perhaps elsewhere). Made a start with using `std::shared_ptr` for `Expr`
  - [ ] Verify correct freeing of previous with a memory checker (`valgrind`?)


## CmdParameter
- [x] Allow for chained blocks of parameter definitions
- [ ] For display, sort the parameters (except for `--help`, which should be at top)
- Issue, when leaving out `=` for param `-n`:

```
> sudo ./obj/qpu-debug/bin/Mandelbrot  -n12
Error(s) on command line:
  Parameter 'Num QPU's' (-n) takes a value, none specified.

  Use 'help' or '-h' to view options

```

## Library Code
- [ ] Add check in emulator for too many `gather()` calls. Or not enough `receive()` calls, same thing
- [x] Determine num QPUs from hardware
- [x] Add method to determine RPi hardware revision number via mailbox
- [x] Add code for using the `Special Functions Unit (SFU)`, operations: `RECIP`, `RECIPSQRT`, `LOG`, `EXP`
- [x] Add performance counters to interface of `RegisterMap`
- [ ] Add Logging class
- [ ] Add method for build/platform info, for display on startup of an application
- [x] Make QPU execution timeout a runtime setting


## Other

- [x] `Rot3D` make various versions selectable on command line
- [x] enable `-Wall` on compilation and deal with all the fallout
- [x] enable build for QPU and Emulation mode together


## Long Term

- [ ] Add optional doc generation with `doxygen`.
      This is only useful if there are a sufficient number of header comments.
- [ ] Scheduling of kernels - see VideoCore `fft` project.
