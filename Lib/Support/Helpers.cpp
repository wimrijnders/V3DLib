#include "Helpers.h"

namespace V3DLib {

/**
 * Return a random float value between -1 and 1
 */ 
float random_float() {
  return  (1.0f*(((float) (rand() % 200)) - 100.0f))/100.0f;
}


std::string indentBy(int indent) {
  std::string ret;
  for (int i = 0; i < indent; i++) ret += " ";
  return ret;
}

}  // namespace V3DLib
