#include "KernelDriver.h"
#include <iostream>
#include <sstream>
#include "Source/Lang.h"
#include "Source/Translate.h"
#include "Target/RemoveLabels.h"
#include "vc4.h"
#include "DMA/Operations.h"
#include "dump_instr.h"
#include "Target/instr/Mnemonics.h"
#include "SourceTranslate.h"  // add_uniform_pointer_offset()
#include "Instr.h"

namespace V3DLib {
namespace vc4 {
namespace {

using CodeList = Seq<uint64_t>;


/**
 * Convert intermediate instruction into core instruction
 */
void convertInstr(V3DLib::Instr &instr) {
  switch (instr.tag) {
    case IRQ:
      instr.tag           = LI;
      instr.LI.imm        = Imm(1);
      instr.set_cond_clear();
      instr.assign_cond(AssignCond(AssignCond::Tag::ALWAYS));
      instr.dest(Reg(SPECIAL, SPECIAL_HOST_INT));
      break;

    case DMA_LOAD_WAIT:
    case DMA_STORE_WAIT: {
      RegId src = instr.tag == DMA_LOAD_WAIT ? SPECIAL_DMA_LD_WAIT : SPECIAL_DMA_ST_WAIT;

      instr.tag     = ALU;
      instr.ALU.op  = ALUOp(ALUOp::A_BOR);
      instr.set_cond_clear();
      instr.assign_cond(AssignCond(AssignCond::Tag::NEVER));

      instr.ALU.srcA = Reg(SPECIAL, src);  // srcA is same as srcB
      instr.ALU.srcB = Reg(SPECIAL, src);
      instr.dest(Reg(NONE, 0));
      break;
    }

    default:
      break;  // rest passes through
  }
}


CodeList encode_instructions(V3DLib::Instr::List &instrs) {
  CodeList code;

  for (int i = 0; i < instrs.size(); i++) {
    V3DLib::Instr instr = instrs.get(i);
    check_instruction_tag_for_platform(instr.tag, true);

    if (instr.tag == INIT_BEGIN || instr.tag == INIT_END) {
      continue;  // Don't encode these block markers
    }

    convertInstr(instr);
    vc4::Instr vc4_instr;
    vc4_instr.encode(instr);
    code << vc4_instr.code();
  }

  return code;
}


} // anon namespace

KernelDriver::KernelDriver() : V3DLib::KernelDriver(Vc4Buffer) {}


int KernelDriver::kernel_size() const {
  assert(qpuCodeMem.allocated());
  return qpuCodeMem.size();
}


/**
 * Add the postfix code to the kernel.
 *
 * Note that this emits kernel code.
 */
void KernelDriver::kernelFinish() {
  dmaWaitRead();                header("Kernel termination");
                                comment("Ensure outstanding DMAs have completed");
  dmaWaitWrite();

  If (me() == 0)
    Int n = numQPUs()-1;        comment("QPU 0 wait for other QPUs to finish");
    For (Int i = 0, i < n, i++)
      semaDec(15);
    End
    hostIRQ();                  comment("Send host IRQ");
  Else
    semaInc(15);
  End
}


/**
 * Encode target instructions
 *
 * Assumption: code in a kernel, once allocated, does not change.
 */
void KernelDriver::encode() {
  if (!qpuCodeMem.empty()) return;  // Don't bother if already encoded
  if (has_errors()) return;         // Don't do this if compile errors occured

  CodeList code = encode_instructions(m_targetCode);

  // Allocate memory for QPU code
  qpuCodeMem.alloc(code.size());
  assert(qpuCodeMem.size() > 0);

  // Copy kernel to code memory
  int offset = 0;
  for (int i = 0; i < code.size(); i++) {
    qpuCodeMem[offset++] = code[i];
  }
}


void KernelDriver::emit_opcodes(FILE *f) {
  fprintf(f, "Opcodes for vc4\n");
  fprintf(f, "===============\n\n");

  encode();

  if (qpuCodeMem.empty()) {
    fprintf(f, "<No opcodes to print>\n");
  } else {
    dump_instr(f, qpuCodeMem.ptr(), qpuCodeMem.size());
  }

  fflush(f);
}


void KernelDriver::compile_intern() {
  using Instr = V3DLib::Instr;

  kernelFinish();

  // NOTE During debugging, I noticed that the sequence on the statement stack is duplicated here.
  //      I can not discover why, it's benevolent, it's not clean but I'm leaving it for now.
  // TODO Fix it one day (sigh)

  obtain_ast();

  V3DLib::translate_stmt(m_targetCode, m_body);

  {
    using namespace V3DLib::Target::instr;  // for mov()
    // Add final dummy uniform handling - See Note 1, function `invoke()` in `vc4/Invoke.cpp`,
    // and uniform ptr index offsets

    Instr::List ret;
    ret << mov(VarGen::fresh(), Var(UNIFORM)).comment("Last uniform load is dummy value")
        << add_uniform_pointer_offset(m_targetCode);  // !!! NOTE: doesn't take dummy in previous into account
                                                      // This should not be a problem

    int index = m_targetCode.lastUniformOffset();
    assert(index > 0);
    m_targetCode.insert(index + 1, ret);
  }

  m_targetCode << Instr(END);

  compile_postprocess(m_targetCode);

  // Translate branch-to-labels to relative branches
  removeLabels(m_targetCode);

  encode();
}


void KernelDriver::invoke_intern(int numQPUs, IntList &params) {
  MailBoxInvoke::invoke(numQPUs, qpuCodeMem, params);
}

}  // namespace vc4
}  // namespace V3DLib

