#include "Helpers.h"
#include "Support/debug.h"

namespace V3DLib {
namespace DMA {

V3DLib::Stmt::Tag to_tag(int in_tag) {
  assert(0 <= in_tag && in_tag < V3DLib::Stmt::Tag::NUM_TAGS);
  return (V3DLib::Stmt::Tag) in_tag;
}

}  // namespace DMA
}  // namespace V3DLib

