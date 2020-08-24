#ifndef _QPULIB_TARGET_EMULATOR_H_
#define _QPULIB_TARGET_EMULATOR_H_
#include <stdint.h>
#include "Common/Seq.h"
#include "Common/Queue.h"
#include "Target/Syntax.h"
#include "BufferObject.h"

#define NUM_LANES 16
#define MAX_QPUS 12
#define VPM_SIZE 1024

namespace QPULib {

// This is a type for representing the values in a vector
union Word {
  int32_t intVal;
  float floatVal; 
};

// Vector values
struct Vec {
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

// Emulator
void emulate(
	int numQPUs,              // Number of QPUs active
	Seq<Instr>* instrs,       // Instruction sequence
	int maxReg,               // Max reg id used
	Seq<int32_t>* uniforms,   // Kernel parameters
	BufferObject &heap,
	Seq<char>* output = NULL  // Output from print statements (if NULL, stdout is used)
);

// Rotate a vector
Vec rotate(Vec v, int n);

// Printing routines
void emitChar(Seq<char>* out, char c);
void emitStr(Seq<char>* out, const char* s);
void printIntVec(Seq<char>* out, Vec x);
void printFloatVec(Seq<char>* out, Vec x);

}  // namespace QPULib

#endif  // _QPULIB_TARGET_EMULATOR_H_
