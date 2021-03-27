#include "KernelDriver.h"
#include <iostream>
#include <sstream>
#include "Source/Lang.h"
#include "Source/Translate.h"
#include "Target/RemoveLabels.h"
#include "vc4.h"
#include "Encode.h"
#include "DMA/Operations.h"
#include "dump_instr.h"
#include "Target/instr/Instructions.h"
#include "SourceTranslate.h"  // add_uniform_pointer_offset()
#include "dump_instr.h"

namespace V3DLib {
namespace vc4 {

KernelDriver::KernelDriver() : V3DLib::KernelDriver(Vc4Buffer) {}


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
 * Encode target instrs into array of 32-bit ints
 */
void KernelDriver::encode() {
  if (code.size() > 0) return;      // Don't bother if already encoded
  if (has_errors()) return;         // Don't do this if compile errors occured
  assert(!qpuCodeMem.allocated());

  V3DLib::vc4::encode(m_targetCode, code);
}


void KernelDriver::emit_opcodes(FILE *f) {
  fprintf(f, "Opcodes for vc4\n");
  fprintf(f, "===============\n\n");
  fflush(f);

  if (code.empty()) {
    fprintf(f, "<No opcodes to print>\n");
  } else {
    // Note: this is the same as code, but code is uint64_t and this is uint64_t.
    Seq<uint64_t> instructions;

    for (int i = 0; i < m_targetCode.size(); ++i ) {
      instructions << vc4::encode(m_targetCode[i]);
    }

    assert(instructions.size()*2 == code.size());
    dump_instr(f, instructions.data(), instructions.size());
  }
}


void KernelDriver::compile_intern() {
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
  //debug("Called vc4 KernelDriver::invoke()");  
  assertq(code.size() > 0, "invoke_intern() vc4: no code to invoke", true );

  unsigned numWords = code.size() + 12*MAX_KERNEL_PARAMS + 12*2;

  // Assumption: code in a kernel, once allocated, doesnt' change
  if (qpuCodeMem.allocated()) {
    //debug("vc4 KernelDriver::invoke() code and parameters memory already allocated");
    assert(qpuCodeMem.size() == numWords);
  } else {
    // Allocate memory for QPU code and parameters
    qpuCodeMem.alloc(numWords);
    assert(qpuCodeMem.size() > 0);

    // Copy kernel to code memory
    int offset = 0;
    for (int i = 0; i < code.size(); i++) {
      qpuCodeMem[offset++] = code[i];
    }
    qpuCodeMemOffset = offset;
  }

  enableQPUs();
  V3DLib::invoke(numQPUs, qpuCodeMem, qpuCodeMemOffset, &params);
  disableQPUs();
}

}  // namespace vc4
}  // namespace V3DLib

