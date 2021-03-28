#include "Label.h"

namespace V3DLib {
namespace {

int globalLabelId = 0;  // Used for fresh label generation

}  // anon namespace


/**
 * Obtain a fresh label
 */
Label freshLabel() {
  return globalLabelId++;
}


/**
 * Number of fresh labels used
 */
int getFreshLabelCount() {
  return globalLabelId;
}


/**
 * Reset fresh label generator
 */
void resetFreshLabelGen() {
  globalLabelId = 0;
}


/**
 * Reset fresh label generator to specified value
 */
void resetFreshLabelGen(int val) {
  globalLabelId = val;
}


}  // namespace V3DLib
