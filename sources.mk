#
# This file is generated!  Editing it directly is a bad idea.
#
# Generated on: Sat 20 Mar 2021 02:18:50 AM CET
#
###############################################################################

# Library Object files - only used for LIB
OBJ := \
  vc4/RegAlloc.o  \
  vc4/Invoke.o  \
  vc4/DMA/Helpers.o  \
  vc4/DMA/DMA.o  \
  vc4/DMA/LoadStore.o  \
  vc4/DMA/Operations.o  \
  vc4/vc4.o  \
  vc4/BufferObject.o  \
  vc4/Mailbox.o  \
  vc4/KernelDriver.o  \
  vc4/Encode.o  \
  vc4/RegisterMap.o  \
  vc4/PerformanceCounters.o  \
  vc4/SourceTranslate.o  \
  v3d/RegisterMapping.o  \
  v3d/BufferObject.o  \
  v3d/v3d.o  \
  v3d/KernelDriver.o  \
  v3d/Driver.o  \
  v3d/PerformanceCounters.o  \
  v3d/instr/SmallImm.o  \
  v3d/instr/Instr.o  \
  v3d/instr/Snippets.o  \
  v3d/instr/Register.o  \
  v3d/instr/RFAddress.o  \
  v3d/SourceTranslate.o  \
  Support/InstructionComment.o  \
  Support/pgm.o  \
  Support/basics.o  \
  Support/Helpers.o  \
  Support/debug.o  \
  Support/Timer.o  \
  Support/Platform.o  \
  Support/HeapManager.o  \
  KernelDriver.o  \
  BaseKernel.o  \
  LibSettings.o  \
  Target/Liveness.o  \
  Target/Satisfy.o  \
  Target/Subst.o  \
  Target/BufferObject.o  \
  Target/SmallLiteral.o  \
  Target/EmuSupport.o  \
  Target/Pretty.o  \
  Target/CFG.o  \
  Target/Emulator.o  \
  Target/instr/Instructions.o  \
  Target/instr/Reg.o  \
  Target/instr/Label.o  \
  Target/instr/ALUOp.o  \
  Target/instr/Instr.o  \
  Target/instr/Conditions.o  \
  Source/Stmt.o  \
  Source/Functions.o  \
  Source/Expr.o  \
  Source/Ptr.o  \
  Source/gather.o  \
  Source/Interpreter.o  \
  Source/BExpr.o  \
  Source/Var.o  \
  Source/Op.o  \
  Source/CExpr.o  \
  Source/Lang.o  \
  Source/Cond.o  \
  Source/Pretty.o  \
  Source/StmtStack.o  \
  Source/Int.o  \
  Source/Translate.o  \
  Source/Complex.o  \
  Source/Float.o  \
  Common/SharedArray.o  \
  Common/BufferObject.o  \
  Common/CompileData.o  \
  Kernels/Rot3D.o  \
  Kernels/Matrix.o  \
  SourceTranslate.o  \
  vc4/dump_instr.o  \
  v3d/instr/dump_instr.o  \

# All programs in the Examples *and Tools* directory
EXAMPLES := \
  HeatMap  \
  ID  \
  Rot3D  \
  Tri  \
  Matrix  \
  Hello  \
  DMA  \
  GCD  \
  OET  \
  Mandelbrot  \
  ReqRecv  \
  detectPlatform  \

# support files for examples
EXAMPLES_EXTRA := \
  Examples/Support/Settings.o  \
# support files for tests
TESTS_FILES := \
  Tests/testSFU.o  \
  Tests/testMatrix.o  \
  Tests/testRot3D.o  \
  Tests/support/support.o  \
  Tests/support/summation_kernel.o  \
  Tests/support/rotate_kernel.o  \
  Tests/support/disasm_kernel.o  \
  Tests/testDSL.o  \
  Tests/testConditionCodes.o  \
  Tests/testV3d.o  \
  Tests/testImmediates.o  \
  Tests/testPrefetch.o  \
  Tests/testRegMap.o  \
  Tests/testBO.o  \
  Tests/testCmdLine.o  \
  Tests/testMain.o  \
  Tests/support/qpu_disasm.o  \

