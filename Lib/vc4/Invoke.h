#ifndef _V3DLIB_VC4_INVOKE_H_
#define _V3DLIB_VC4_INVOKE_H_
#include <stdint.h>
#include "Common/Seq.h"
#include "Common/SharedArray.h"

namespace V3DLib {

/**
 * Mixin class for Mailbox invocation
 *
 * Prepares the data for the call and executes the call.
 */
class MailBoxInvoke {
public:
  void invoke(int numQPUs, Code const &code, IntList const &params);

private:
  Data m_uniforms;  // Memory region for QPU parameters


  /**
   * Container for launch info per QPU to run
   *
   * Array consecutively containing two values per QPU to run:
   *  - pointer to uniform parameters to pass per QPU
   *  - Start of code block to run per QPU
   *
   * The uniforms are essentially the same for all QPUs, *except* qpu id, the first parameter.
   *
   * It thus be possible to run different code per QPU.
   * Haven't tried this yet, till now all the QPUs run the same code.
   */
  Data launch_messages;
};

}  // namespace V3DLib

#endif  // _V3DLIB_VC4_INVOKE_H_
