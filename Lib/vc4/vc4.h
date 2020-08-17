#ifdef QPU_MODE

#ifndef _QPULIB_VC4_VC4_H_
#define _QPULIB_VC4_VC4_H_

namespace QPULib {

// Globals
extern int mailbox;
extern int numQPUUsers;

// Operations
int getMailbox();
void enableQPUs();
void disableQPUs();

}  // namespace QPULib

#endif  // _QPULIB_VC4_VC4_H_
#endif  // QPU_MODE
