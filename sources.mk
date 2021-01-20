#
# This file is generated!  Editing it directly is a bad idea.
#
# Generated on: Wed 20 Jan 2021 03:40:17 AM CET
#
###############################################################################

# Library Object files - only used for LIB
OBJ := \
  vc4/RegisterMap.o  \
  vc4/RegAlloc.o  \
  vc4/SourceTranslate.o  \
  vc4/Invoke.o  \
  vc4/vc4.o  \
  vc4/LoadStore.o  \
  vc4/BufferObject.o  \
  vc4/DMA.o  \
  vc4/KernelDriver.o  \
  vc4/Translate.o  \
  vc4/Encode.o  \
  vc4/Mailbox.o  \
  vc4/PerformanceCounters.o  \
  Support/Platform.o  \
  Support/debug.o  \
  Support/InstructionComment.o  \
  Support/HeapManager.o  \
  SourceTranslate.o  \
  Kernel.o  \
  KernelDriver.o  \
  Source/gather.o  \
  Source/StmtStack.o  \
  Source/Expr.o  \
  Source/Int.o  \
  Source/Interpreter.o  \
  Source/Float.o  \
  Source/CExpr.o  \
  Source/Cond.o  \
  Source/Var.o  \
  Source/Op.o  \
  Source/BExpr.o  \
  Source/Stmt.o  \
  Source/Pretty.o  \
  Source/Translate.o  \
  Source/Lang.o  \
  v3d/SourceTranslate.o  \
  v3d/Invoke.o  \
  v3d/v3d.o  \
  v3d/BufferObject.o  \
  v3d/RegisterMapping.o  \
  v3d/Driver.o  \
  v3d/KernelDriver.o  \
  v3d/instr/Register.o  \
  v3d/instr/RFAddress.o  \
  v3d/instr/Snippets.o  \
  v3d/instr/Instr.o  \
  v3d/instr/SmallImm.o  \
  v3d/PerformanceCounters.o  \
  Common/BufferObject.o  \
  Target/Syntax.o  \
  Target/Satisfy.o  \
  Target/Reg.o  \
  Target/CFG.o  \
  Target/BufferObject.o  \
  Target/SmallLiteral.o  \
  Target/Emulator.o  \
  Target/Liveness.o  \
  Target/Pretty.o  \
  Target/Instr.o  \
  Target/instr/Conditions.o  \
  Target/instr/ALUOp.o  \
  Target/Subst.o  \
  Target/EmuSupport.o  \
  vc4/dump_instr.o  \
  v3d/instr/dump_instr.o  \

# All programs in the Examples *and Tools* directory
EXAMPLES := \
  ReqRecv  \
  Hello  \
  GCD  \
  Print  \
  Tri  \
  DMA  \
  OET  \
  Rot3D  \
  Mandelbrot  \
  HeatMap  \
  ID  \
  detectPlatform  \

# support files for examples
EXAMPLES_EXTRA := \
  Examples/Rot3DLib/Rot3DKernels.o  \
  Examples/Support/Timer.o  \
  Examples/Support/Settings.o  \
  
# support files for tests
TESTS_FILES := \
  Tests/testBO.o  \
  Tests/support/summation_kernel.o  \
  Tests/support/disasm_kernel.o  \
  Tests/support/support.o  \
  Tests/support/Gen.o  \
  Tests/support/rotate_kernel.o  \
  Tests/testAutoTest.o  \
  Tests/testRot3D.o  \
  Tests/testRegMap.o  \
  Tests/testSFU.o  \
  Tests/testConditionCodes.o  \
  Tests/testV3d.o  \
  Tests/testCmdLine.o  \
  Tests/testDSL.o  \
  Tests/testMatrix.o  \
  Tests/testMain.o  \
  Tests/support/qpu_disasm.o  \

