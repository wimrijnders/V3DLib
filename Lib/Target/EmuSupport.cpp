#include "EmuSupport.h"
#include <cstdio>
#include <cstring>  // strlen()

namespace V3DLib {

// ============================================================================
// Rotate a vector
// ============================================================================

Vec rotate(Vec v, int n)
{
  Vec w;
  for (int i = 0; i < NUM_LANES; i++)
    w[(i+n) % NUM_LANES] = v[i];
  return w;
}


// ============================================================================
// Printing routines
// ============================================================================

namespace {

void emitChar(Seq<char>* out, char c) {
  if (out == nullptr) printf("%c", c);
  else *out << c;
}

}  // anon namespace


void emitStr(Seq<char>* out, const char* s) {
  if (out == nullptr)
    printf("%s", s);
  else
    for (int i = 0; i < (int) strlen(s); i++)
      *out << s[i];
}


void printIntVec(Seq<char>* out, Vec x) {
  char buffer[1024];
  emitChar(out, '<');
  for (int i = 0; i < NUM_LANES; i++) {
    snprintf(buffer, sizeof(buffer), "%i", x[i].intVal);

    for (int j = 0; j < (int) strlen(buffer); j++) emitChar(out, buffer[j]);
    if (i != NUM_LANES-1) emitChar(out, ',');
  }
  emitChar(out, '>');
}


void printFloatVec(Seq<char>* out, Vec x) {
  char buffer[1024];
  emitChar(out, '<');
  for (int i = 0; i < NUM_LANES; i++) {
    snprintf(buffer, sizeof(buffer), "%f", x[i].floatVal);

    for (int j = 0; j < (int) strlen(buffer); j++) emitChar(out, buffer[j]);
    if (i != NUM_LANES-1) emitChar(out, ',');
  }
  emitChar(out, '>');
}

}  // namespace V3DLib
