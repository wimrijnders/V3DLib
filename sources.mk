#
# This file is generated!  Editing it directly is a bad idea.
#
# Generated on: Tue 06 Jul 2021 10:14:39 AM CEST
#
###############################################################################

# Library Object files - only used for LIB
OBJ := \
  vc4/RegisterMap.o  \
  vc4/RegAlloc.o  \
  vc4/SourceTranslate.o  \
  vc4/Invoke.o  \
  vc4/vc4.o  \
  vc4/BufferObject.o  \
  vc4/DMA/LoadStore.o  \
  vc4/DMA/DMA.o  \
  vc4/DMA/Helpers.o  \
  vc4/DMA/Operations.o  \
  vc4/KernelDriver.o  \
  vc4/Encode.o  \
  vc4/Mailbox.o  \
  vc4/PerformanceCounters.o  \
  LibSettings.o  \
  Support/Platform.o  \
  Support/debug.o  \
  Support/InstructionComment.o  \
  Support/basics.o  \
  Support/Helpers.o  \
  Support/RegIdSet.o  \
  Support/pgm.o  \
  Support/Timer.o  \
  Support/HeapManager.o  \
  Kernels/Rot3D.o  \
  Kernels/Matrix.o  \
  Kernels/Cursor.o  \
  SourceTranslate.o  \
  KernelDriver.o  \
  BaseKernel.o  \
  Source/gather.o  \
  Source/StmtStack.o  \
  Source/Expr.o  \
  Source/Int.o  \
  Source/Interpreter.o  \
  Source/Float.o  \
  Source/CExpr.o  \
  Source/Cond.o  \
  Source/Ptr.o  \
  Source/Var.o  \
  Source/Op.o  \
  Source/BExpr.o  \
  Source/Stmt.o  \
  Source/Functions.o  \
  Source/OpItems.o  \
  Source/Pretty.o  \
  Source/Translate.o  \
  Source/Complex.o  \
  Source/Lang.o  \
  Liveness/Range.o  \
  Liveness/LiveSet.o  \
  Liveness/CFG.o  \
  Liveness/Liveness.o  \
  Liveness/UseDef.o  \
  Liveness/Optimizations.o  \
  Liveness/RegUsage.o  \
  v3d/SourceTranslate.o  \
  v3d/v3d.o  \
  v3d/BufferObject.o  \
  v3d/RegisterMapping.o  \
  v3d/Driver.o  \
  v3d/KernelDriver.o  \
  v3d/instr/Register.o  \
  v3d/instr/RFAddress.o  \
  v3d/instr/Source.o  \
  v3d/instr/Snippets.o  \
  v3d/instr/OpItems.o  \
  v3d/instr/Mnemonics.o  \
  v3d/instr/Instr.o  \
  v3d/instr/Encode.o  \
  v3d/instr/SmallImm.o  \
  v3d/PerformanceCounters.o  \
  Common/SharedArray.o  \
  Common/BufferObject.o  \
  Common/CompileData.o  \
  Target/Satisfy.o  \
  Target/BufferObject.o  \
  Target/SmallLiteral.o  \
  Target/Emulator.o  \
  Target/Pretty.o  \
  Target/instr/ALUInstruction.o  \
  Target/instr/Conditions.o  \
  Target/instr/Imm.o  \
  Target/instr/Reg.o  \
  Target/instr/Label.o  \
  Target/instr/Mnemonics.o  \
  Target/instr/Instr.o  \
  Target/instr/RegOrImm.o  \
  Target/instr/ALUOp.o  \
  Target/Subst.o  \
  Target/EmuSupport.o  \
  vc4/dump_instr.o  \
  v3d/instr/v3d_api.o  \

# All programs in the Examples *and Tools* directory
EXAMPLES := \
  ReqRecv  \
  Hello  \
  GCD  \
  Tri  \
  DMA  \
  OET  \
  Rot3D  \
  Mandelbrot  \
  HeatMap  \
  ID  \
  Matrix  \
  detectPlatform  \

# support files for examples
EXAMPLES_EXTRA := \
  Examples/Support/Settings.o  \
# support files for tests
TESTS_FILES := \
  Tests/testBO.o  \
  Tests/support/summation_kernel.o  \
  Tests/support/matrix_support.o  \
  Tests/support/disasm_kernel.o  \
  Tests/support/support.o  \
  Tests/support/rotate_kernel.o  \
  Tests/support/dft_support.o  \
  Tests/support/ProfileOutput.o  \
  Tests/testFFT.o  \
  Tests/testRot3D.o  \
  Tests/testRegMap.o  \
  Tests/testSFU.o  \
  Tests/testPrefetch.o  \
  Tests/testConditionCodes.o  \
  Tests/testV3d.o  \
  Tests/testCmdLine.o  \
  Tests/testDFT.o  \
  Tests/testDSL.o  \
  Tests/testFunctions.o  \
  Tests/testMatrix.o  \
  Tests/testImmediates.o  \
  Tests/testMain.o  \
  Tests/support/qpu_disasm.o  \

