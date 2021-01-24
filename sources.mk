#
# This file is generated!  Editing it directly is a bad idea.
#
# Generated on: Sun 24 Jan 2021 03:38:20 AM CET
#
###############################################################################

# Library Object files - only used for LIB
OBJ := \
  vc4/RegAlloc.o  \
  vc4/Invoke.o  \
  vc4/vc4.o  \
  vc4/BufferObject.o  \
  vc4/Mailbox.o  \
  vc4/KernelDriver.o  \
  vc4/DMA.o  \
  vc4/Encode.o  \
  vc4/LoadStore.o  \
  vc4/RegisterMap.o  \
  vc4/PerformanceCounters.o  \
  vc4/Translate.o  \
  vc4/SourceTranslate.o  \
  v3d/RegisterMapping.o  \
  v3d/Invoke.o  \
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
  Support/basics.o  \
  Support/debug.o  \
  Support/Platform.o  \
  Support/HeapManager.o  \
  KernelDriver.o  \
  Target/Syntax.o  \
  Target/Liveness.o  \
  Target/Reg.o  \
  Target/Satisfy.o  \
  Target/Subst.o  \
  Target/BufferObject.o  \
  Target/SmallLiteral.o  \
  Target/Instr.o  \
  Target/EmuSupport.o  \
  Target/Pretty.o  \
  Target/CFG.o  \
  Target/Emulator.o  \
  Target/instr/ALUOp.o  \
  Target/instr/Conditions.o  \
  Source/Stmt.o  \
  Source/Expr.o  \
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
  Source/Float.o  \
  Common/BufferObject.o  \
  SourceTranslate.o  \
  Kernel.o  \
  vc4/dump_instr.o  \
  v3d/instr/dump_instr.o  \

# All programs in the Examples *and Tools* directory
EXAMPLES := \
  HeatMap  \
  ID  \
  Rot3D  \
  Tri  \
  Print  \
  Hello  \
  DMA  \
  GCD  \
  OET  \
  Mandelbrot  \
  ReqRecv  \
  detectPlatform  \

# support files for examples
EXAMPLES_EXTRA := \
  Examples/Kernels/Rot3D.o  \
  Examples/Support/Settings.o  \
  Examples/Support/Timer.o  \
  
# support files for tests
TESTS_FILES := \
  Tests/testSFU.o  \
  Tests/testMatrix.o  \
  Tests/testRot3D.o  \
  Tests/support/support.o  \
  Tests/support/summation_kernel.o  \
  Tests/support/rotate_kernel.o  \
  Tests/support/Gen.o  \
  Tests/support/disasm_kernel.o  \
  Tests/testDSL.o  \
  Tests/testConditionCodes.o  \
  Tests/testV3d.o  \
  Tests/testAutoTest.o  \
  Tests/testRegMap.o  \
  Tests/testBO.o  \
  Tests/testCmdLine.o  \
  Tests/testMain.o  \
  Tests/support/qpu_disasm.o  \

