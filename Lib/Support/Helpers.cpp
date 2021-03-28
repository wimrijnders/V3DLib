#include "Helpers.h"

namespace V3DLib {

std::string indentBy(int indent) {
  std::string ret;
  for (int i = 0; i < indent; i++) ret += " ";
  return ret;
}

}  // namespace V3DLib
