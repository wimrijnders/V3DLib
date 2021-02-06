#ifndef _V3DLIB_TARGET_EMUSUPPORT_H_
#define _V3DLIB_TARGET_EMUSUPPORT_H_
#include <stdint.h>
#include "Common/Seq.h"


/**
 * Definitions which are used in both the emulator and the interpreter.
 */
namespace V3DLib {

const int NUM_LANES =   16;
const int MAX_QPUS  =   12;
const int VPM_SIZE  = 1024;

// This is a type for representing the values in a vector
union Word {
  int32_t intVal = 0;
  float floatVal; 
};


// Vector values
struct Vec {
  Word &get(int index) {
    assert(0 <= index && index < NUM_LANES);
    return elems[index];
  }

  Word &operator[](int index) {
    return get(index);
  }

private:
   Word elems[NUM_LANES];
};

// In-flight DMA request
struct DMAAddr {
  bool active;
  Word addr;
};

// VPM load request
struct VPMLoadReq {
  int numVecs;  // Number of vectors to load
  bool hor;     // Horizintal or vertical access?
  int addr;     // Address in VPM to load from
  int stride;   // Added to address after every vector read
};

// VPM store request
struct VPMStoreReq {
  bool hor;     // Horizintal or vertical access?
  int addr;     // Address in VPM to load from
  int stride;   // Added to address after every vector written
};

// DMA load request
struct DMALoadReq {
  bool hor;     // Horizintal or vertical access?
  int numRows;  // Number of rows in memory
  int rowLen;   // Length of each row in memory
  int vpmAddr;  // VPM address to write to
  int vpitch;   // Added to vpmAddr after each vector loaded
};

// DMA store request
struct DMAStoreReq {
  bool hor;     // Horizintal or vertical access?
  int numRows;  // Number of rows in memory
  int rowLen;   // Length of each row in memory
  int vpmAddr;  // VPM address to load from
};


// Rotate a vector
Vec rotate(Vec v, int n);

// Printing routines
void emitStr(Seq<char>* out, const char* s);
void printIntVec(Seq<char>* out, Vec x);
void printFloatVec(Seq<char>* out, Vec x);

}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_EMUSUPPORT_H_
