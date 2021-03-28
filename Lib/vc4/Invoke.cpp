#include "Invoke.h"
#include "Mailbox.h"
#include "vc4.h"
#include "defines.h"
#include "LibSettings.h"

namespace V3DLib {

using Code = SharedArray<uint32_t>;


/**
 * TODO rewrite to shared array holding the parameters
 *
 * ----------------------------------------------------------------------------
 * Notes
 * =====
 *
 * 1. Often (not always!), the final param is passed garbled when input as 
 *    a uniform to a kernel program executing on vc4 hardware.
 *    It appears to happen to direct Float/Int values.
 *    After spending days on this with paranoid debugging, I could not find the
 *    cause and gave up. Instead, I'll just pass a final dummy uniform value,
 *    which can be mangled to the heart's content of the hardware.
 */
void invoke(int numQPUs, Code &codeMem, int qpuCodeMemOffset, IntList *params) {
  //
  // Number of 32-bit words needed for kernel code & parameters
  // - First two values are always the QPU ID and num QPU's
  // - Next come the actual kernel parameters, as defined in the user code
  // - This is terminated by a dummy uniform value, see Note 1.
  // - The final two words are the pointer to the parameters per QPU, and
  //   the pointer to the kernel program to execute.
  //
  unsigned numWords = qpuCodeMemOffset + (2 + params->size() + 1)*numQPUs + 2*numQPUs;
  assert(numWords < codeMem.size());

  // Pointer to start of code
  uint32_t *qpuCodePtr = codeMem.getPointer();

  // Copy parameters to instruction memory
  int offset = qpuCodeMemOffset;
  uint32_t** paramsPtr = new uint32_t* [numQPUs];  // TODO check shouldn't this be deleted?
  for (int i = 0; i < numQPUs; i++) {
    paramsPtr[i] = qpuCodePtr + offset;
    codeMem[offset++] = (uint32_t) i;              // Unique QPU ID
    codeMem[offset++] = (uint32_t) numQPUs;        // QPU count
    for (int j = 0; j < params->size(); j++)
      codeMem[offset++] = params->get(j);
    codeMem[offset++] = 0;                         // Dummy final parameter, see Note 1.
  }

#ifdef ARM32
  int mb = getMailbox();  // Open mailbox for talking to vc4

  // Copy launch messages
  uint32_t* launchMsgsPtr = qpuCodePtr + offset;
  for (int i = 0; i < numQPUs; i++) {
    codeMem[offset++] = (uint32_t) paramsPtr[i];
    codeMem[offset++] = (uint32_t) qpuCodePtr;
  }

  assertq(offset == (int) numWords, "Check final offset failed");

  // Launch QPUs
  unsigned result = execute_qpu(mb, numQPUs, (uint32_t) launchMsgsPtr, 1, LibSettings::qpu_timeout()*1000);
#else
  assertq(false, "invoke() will not run on this platform, only on ARM 32-bits");
  unsigned result = 1;  // Force error message
#endif

  if (result != 0) {
    printf("Failed to invoke kernel on QPUs\n");
  }
}

}  // namespace V3DLib
