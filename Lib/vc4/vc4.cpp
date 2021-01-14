#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "vc4.h"
#include "Mailbox.h"
#include "Support/basics.h"  // fatal()
#include "../Support/debug.h"

namespace V3DLib {

// Globals
int mailbox = -1;
int numQPUUsers = 0;

// Get mailbox (open if not already opened)
int getMailbox()
{
  if (mailbox < 0) mailbox = mbox_open();
  return mailbox;
}

// Enable QPUs (if not already enabled)
void enableQPUs()
{
  int mb = getMailbox();
  if (numQPUUsers == 0) {
    int qpu_enabled = !qpu_enable(mb, 1);
    if (!qpu_enabled) {
      fatal("Unable to enable QPUs. Check your firmware is latest.");
    }
  }
  numQPUUsers++;
}

// Disable QPUs
void disableQPUs()
{
  assert(numQPUUsers > 0);
  int mb = getMailbox();
  numQPUUsers--;
  if (numQPUUsers == 0) {
    qpu_enable(mb, 0);
  }
}

}  // namespace V3DLib
