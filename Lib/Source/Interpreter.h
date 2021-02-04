#ifndef _V3DLIB_INTERPRETER_H_
#define _V3DLIB_INTERPRETER_H_
#include <stdint.h>
#include "Source/Stmt.h"

namespace V3DLib {

class BufferObject;

template<typename T>
class Seq;

void interpreter(
  int numCores,
  Stmt::Ptr stmt,
  int numVars,
  Seq<int32_t> &uniforms,
  BufferObject &heap,
  Seq<char> *output = nullptr
);

}  // namespace V3DLib

#endif  // _V3DLIB_INTERPRETER_H_
