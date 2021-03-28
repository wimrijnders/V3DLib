#ifndef _V3DLIB_VC4_VC4_H_
#define _V3DLIB_VC4_VC4_H_

namespace V3DLib {

// Globals
//extern int mailbox;
//extern int numQPUUsers;

// Operations
int getMailbox();
void enableQPUs();
void disableQPUs();

}  // namespace V3DLib

#endif  // _V3DLIB_VC4_VC4_H_
