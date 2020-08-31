#include "EmuSupport.h"
#include <cstdio>
#include <cstring>  // strlen()

namespace QPULib {

// ============================================================================
// Rotate a vector
// ============================================================================

Vec rotate(Vec v, int n)
{
  Vec w;
  for (int i = 0; i < NUM_LANES; i++)
    w.elems[(i+n) % NUM_LANES] = v.elems[i];
  return w;
}


// ============================================================================
// Printing routines
// ============================================================================

void emitChar(Seq<char>* out, char c)
{
  if (out == NULL) printf("%c", c);
  else out->append(c);
}

void emitStr(Seq<char>* out, const char* s)
{
  if (out == NULL)
    printf("%s", s);
  else
    for (int i = 0; i < strlen(s); i++)
      out->append(s[i]);
}

void printIntVec(Seq<char>* out, Vec x)
{
  char buffer[1024];
  emitChar(out, '<');
  for (int i = 0; i < NUM_LANES; i++) {
    snprintf(buffer, sizeof(buffer), "%i", x.elems[i].intVal);
    for (int j = 0; j < strlen(buffer); j++) emitChar(out, buffer[j]);
    if (i != NUM_LANES-1) emitChar(out, ',');
  }
  emitChar(out, '>');
}

void printFloatVec(Seq<char>* out, Vec x)
{
  char buffer[1024];
  emitChar(out, '<');
  for (int i = 0; i < NUM_LANES; i++) {
    snprintf(buffer, sizeof(buffer), "%f", x.elems[i].floatVal);
    for (int j = 0; j < strlen(buffer); j++) emitChar(out, buffer[j]);
    if (i != NUM_LANES-1) emitChar(out, ',');
  }
  emitChar(out, '>');
}

}  // namespace QPULib
