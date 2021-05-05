//
// Code here taken from `py-videocore6/blob/master/tests/test_signals.py`,
// lines 131-199.
//
///////////////////////////////////////////////////////////////////////////////
#include "rotate_kernel.h"
#include "v3d/instr/Snippets.h"

using Data         = V3DLib::Data;
using Code         = V3DLib::SharedArray<uint64_t>;
using ByteCode     = V3DLib::v3d::ByteCode;
using Instructions = V3DLib::v3d::Instructions;


//
// Issue: disassembly code in MESA does not output rotate flag
//
// **NOTE:* python code changed in the meantime! This is the previous version.
//
ByteCode const qpu_rotate_alias_code = {
  0x3c403180bb802000,  // eidx  r0             ; nop               ; ldunif
  0x3c402180b682d000,  // or  rf0, r5, r5      ; nop               ; ldunif
  0x3de010437cf7f004,  // shl  r3, 4, 4        ; mov  rf1, r5
  0x3de031807c838002,  // shl  r0, r0, 2       ; nop
  0x3c00218038806000,  // add  rf0, rf0, r0    ; nop
  0x3c00218138806040,  // add  rf1, rf1, r0    ; nop
  0x0420100cb67b6000,  // or  tmua, rf0, rf0   ; add  rf0, rf0, r3 ; thrsw
  0x3c003186bb800000,  // nop                  ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3c903186bb800000,  // nop                  ; nop               ; ldtmu.r0
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00011,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00012,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00013,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00014,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00015,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00016,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00017,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00018,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00019,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0001a,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0001b,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0001c,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0001d,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0001e,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0001f,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3c003046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00001,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00002,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00003,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00004,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00005,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00006,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00007,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00008,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe00009,  // nop                  ; mov  r1, r0

  // WRI: for some reason, smallimm = 10 gets skipped in the python output
  //      totally unclear why
  //      Added here by hand
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0000a,  // nop                  ; mov  r1, r0

  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0000b,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0000c,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0000d,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0000e,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3ee03046bbe0000f,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f011,  // or  r5, -15, -15     ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f012,  // or  r5, -14, -14     ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f013,  // or  r5, -13, -13     ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f014,  // or  r5, -12, -12     ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f015,  // or  r5, -11, -11     ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f016,  // or  r5, -10, -10     ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f017,  // or  r5, -9, -9       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f018,  // or  r5, -8, -8       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f019,  // or  r5, -7, -7       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f01a,  // or  r5, -6, -6       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f01b,  // or  r5, -5, -5       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f01c,  // or  r5, -4, -4       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f01d,  // or  r5, -3, -3       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f01e,  // or  r5, -2, -2       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f01f,  // or  r5, -1, -1       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f000,  // or  r5, 0, 0         ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f001,  // or  r5, 1, 1         ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f002,  // or  r5, 2, 2         ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f003,  // or  r5, 3, 3         ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f004,  // or  r5, 4, 4         ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f005,  // or  r5, 5, 5         ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f006,  // or  r5, 6, 6         ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f007,  // or  r5, 7, 7         ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f008,  // or  r5, 8, 8         ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f009,  // or  r5, 9, 9         ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f00a,  // or  r5, 10, 10       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f00b,  // or  r5, 11, 11       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f00c,  // or  r5, 12, 12       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f00d,  // or  r5, 13, 13       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f00e,  // or  r5, 14, 14       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3de03185b683f00f,  // or  r5, 15, 15       ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3ee03046bbe00000,  // nop                  ; mov  r1, r0
  0x3c00318bb6809000,  // or  tmud, r1, r1     ; nop
  0x3c00318cb6836040,  // or  tmua, rf1, rf1   ; nop
  0x04001046bb795040,  // tmuwt  -             ; add  rf1, rf1, r3
  0x3c203186bb800000,  // nop                  ; nop               ; thrsw
  0x3c203186bb800000,  // nop                  ; nop               ; thrsw
  0x3c003186bb800000,  // nop                  ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3c203186bb800000,  // nop                  ; nop               ; thrsw
  0x3c003186bb800000,  // nop                  ; nop
  0x3c003186bb800000,  // nop                  ; nop
  0x3c003186bb800000,  // nop                  ; nop
};


/**
 * Derived from `def qpu_rotate_alias(asm)` in `py-videocore6/blob/master/tests/test_signals.py`
 * **NOTE:* python code changed in the meantime! This works with the previous version.
 */
ByteCode rotate_kernel() {
  using namespace V3DLib::v3d::instr;

  Instructions ret;

  ret << eidx(r0).ldunif()
      << mov(rf(0), r5).ldunif()
      << shl(r3, 4, 4).mov(rf(1), r5)
      << shl(r0, r0, 2)
      << add(rf(0), rf(0), r0)
      << add(rf(1), rf(1), r0)
      << mov(tmua, rf(0)).add(rf(0), rf(0), r3).thrsw()
      << nop()
      << nop()
      << nop().ldtmu(r0)
      << nop().comment("required before rotate");

  for (int i = -15; i < 16; ++i) {
    ret << rotate(r1, r0, i)  // redirects to mul alu, no point in checking `nop().rotate(r1, r0, i)`
        << mov(tmud, r1)
        << mov(tmua, rf(1))
        << tmuwt().add(rf(1), rf(1), r3);
  }

  for (int i = -15; i < 16; ++i) {
    ret << mov(r5, si(i))
        << nop().comment("required before rotate")
        << rotate(r1, r0, r5)  // redirects to mul alu, no point in checking `nop().rotate(r1, r0, r5)`
        << mov(tmud, r1)
        << mov(tmua, rf(1))
        << tmuwt().add(rf(1), rf(1), r3);
  }

  ret << end_program();

  ByteCode bytecode;
  for (auto const &instr : ret) {
    bytecode << instr.code(); 
  }

  return bytecode;
}


void run_rotate_alias_kernel(ByteCode const &bytecode) {
#ifndef QPU_MODE
  assertq(false, "Cannot run run_rotate_alias_kernel(), QPU_MODE not enabled");
#else
  using namespace V3DLib::v3d;
  REQUIRE(bytecode.size() > 0);

  uint32_t code_area_size = (uint32_t) (8*bytecode.size());  // size in bytes
  uint32_t data_area_size = (10 * 1024) * 4;                 // taken amply

  BufferObject heap;
  heap.alloc(code_area_size + data_area_size);
  Code code((uint32_t) bytecode.size(), heap);
  code.copyFrom(bytecode);

  Data X(16, heap);

  for (uint32_t offset = 0; offset < X.size(); ++offset) {
    X[offset] = offset;
  }
  //dump_data(X); 

  int y_length = 2*(16 - -15) *16;  // NB python range(-15, 16) does not include 2nd value; 'up to'
  Data Y(y_length, heap);

  for (uint32_t offset = 0; offset < Y.size(); ++offset) {
    Y[offset] = 0;
  }

  Data unif(3, heap);
  unif[0] = X.getAddress();
  unif[1] = Y.getAddress();

  V3DLib::v3d::Driver drv;
  drv.add_bo(heap.getHandle());
  REQUIRE(drv.execute(code, &unif, 1));

  for(int count = 0; count < 2; ++count) { // Output is double, first with small imm, then with r5
    for(int rot = -15; rot < 16; ++rot) {
      for(int i = 0; i < 15; ++i) {
        int y_index = count*31*16 + (rot + 15)*16 + i;
        int index   = 16 - rot;

        INFO("i: " << i << "; y index: " << y_index << "; rot: " << rot << "; rot index: " << index);
        REQUIRE(Y[y_index] == X[(i + index) % 16]);
      }
    }
  }
#endif  // QPU_MODE
}
