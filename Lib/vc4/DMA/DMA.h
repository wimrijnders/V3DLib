#ifndef _V3DLIB_VC4_DMA_DMA_H_
#define _V3DLIB_VC4_DMA_DMA_H_
#include <string>
#include "Source/Expr.h"
#include "Source/Int.h"
#include "Target/instr/Instr.h"

namespace V3DLib {
namespace DMA {

// ============================================================================
// Class DMA::Stmt
// ============================================================================

class Stmt {
public:
  void setupVPMRead(int n, Expr::Ptr addr, bool hor, int stride);
  void setupVPMWrite(Expr::Ptr addr, bool hor, int stride);
  void setupDMARead(bool is_horizontal, int numRows, Expr::Ptr addr, int rowLen, int vpitch);
  void setupDMAWrite(bool is_horizontal, int numRows, Expr::Ptr addr, int rowLen);

  Expr::Ptr stride_internal();
  bool address(int in_tag, Expr::Ptr e0);
  Expr::Ptr address_internal();
  std::string pretty(int indent, int in_tag);

  int  semaId() const { return m_semaId; }
  void semaId(int val) { m_semaId = val; }

  // Following methods defined in LoadStore.cpp
  Seq<Instr> setupVPMRead();
  Seq<Instr> setupDMARead();
  Seq<Instr> setupDMAWrite();
  Seq<Instr> setupVPMWrite();

  static bool is_dma_readwrite_tag(int in_tag);
  static bool is_dma_tag(int in_tag);

private:
  // Semaphore id for increment / decrement
  int m_semaId;

  // VPM read setup
  struct { int numVecs; int hor; int stride; } m_setupVPMRead;

  // VPM write setup
  struct { int hor; int stride; } m_setupVPMWrite;

  // DMA read setup
  struct {
    int numRows;
    int rowLen;
    int hor;
    int vpitch;
  } m_setupDMARead;

  // DMA write setup
  struct {
    int numRows;
    int rowLen;
    int hor;
  } m_setupDMAWrite;

  Expr::Ptr m_exp;

  void address_internal(Expr::Ptr e0);
};

std::string disp(int in_tag);

}  // namespace DMA
}  // namespace V3DLib

#endif  // _V3DLIB_VC4_DMA_DMA_H_
