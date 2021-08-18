#include "Invoke.h"
#include "Mailbox.h"
#include "vc4.h"
#include "defines.h"
#include "LibSettings.h"
#include "Support/Platform.h"

namespace V3DLib {
namespace  {


/**
 * Number of 32-bit words needed for the parameters (uniforms)
 *
 * - First two values are always the QPU ID and num QPU's
 * - Next come the actual kernel parameters, as defined in the user code
 * - This is terminated by a dummy uniform value, see Note 1.
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
int num_params(IntList const &params) {
  assert(!params.empty());
  return (2 + params.size() + 1);
}


/**
 * Initialize uniforms to pass into running QPUs
 *
 * The number and types of parameters will not change for a given kernel.
 * The value of the parameters, however, can change, so this needs to be reset every time.
 *
 * All uniform values are the same for all QPUs, *except* the qpu id.
 */
void init_uniforms(Data &uniforms, IntList const &params, int numQPUs) {
  assert(0 < numQPUs && numQPUs <= Platform::max_qpus());

  if (!uniforms.allocated()) {
    uniforms.alloc(num_params(params)*Platform::max_qpus());
  } else {
    assert((int) uniforms.size() == num_params(params)*Platform::max_qpus());
  }

  int offset = 0;
  for (int i = 0; i < numQPUs; i++) {
    uniforms[offset++] = (uint32_t) i;              // Unique QPU ID
    uniforms[offset++] = (uint32_t) numQPUs;        // QPU count

    for (int j = 0; j < params.size(); j++) {
      uniforms[offset++] = params[j];
    }

    uniforms[offset++] = 0;                         // Dummy final parameter, see Note 1.
  }

  assert(offset == num_params(params)*numQPUs);
}


/**
 * Initialize launch messages, if not already done so
 *
 * Doing this for max number of QPUs, so that num QPUs can be changed dynamically on calls.
 */
void init_launch_messages(Data &launch_messages, uint32_t *qpuCodePtr, int num_params, Data &uniforms) {
  if (launch_messages.allocated()) {
    return;
  }

  launch_messages.alloc(2*Platform::max_qpus());

  for (int i = 0; i < Platform::max_qpus(); i++) {
    launch_messages[2*i]     = (uint32_t) (uniforms.getPointer() + i*num_params);  // Braces are sneaky! Required
    launch_messages[2*i + 1] = (uint32_t) qpuCodePtr;
  }
}

}  // anon namespace

/**
 * TODO rewrite to shared array holding the parameters
 */
void invoke(int numQPUs, Code &codeMem, IntList const &params, Data &uniforms, Data &launch_messages) {
#ifndef ARM32
  error("invoke() will not run on this platform, only on ARM 32-bits");
  error("Failed to invoke kernel on QPUs\n");
  return;
#else
  init_uniforms(uniforms, params, numQPUs);
  init_launch_messages(launch_messages, codeMem.getPointer(), num_params(params), uniforms);

  debug(uniforms.dump());
  debug(launch_messages.dump());

  int mb = getMailbox();  // Open mailbox for talking to vc4

  unsigned result = execute_qpu(
    mb,
    numQPUs,
    (uint32_t) launch_messages.getPointer(),
    1,
    LibSettings::qpu_timeout()*1000
  );

  if (result != 0) {
    error("Failed to invoke kernel on QPUs\n");
  }
#endif
}

}  // namespace V3DLib
