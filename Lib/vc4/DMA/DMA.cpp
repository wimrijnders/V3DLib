#include "DMA.h"
#include "Support/basics.h"
#include "Support/Helpers.h"
#include "Helpers.h"

namespace V3DLib {
namespace DMA {

using ::operator<<;  // C++ weirdness

void Stmt::setupVPMRead(int n, Expr::Ptr addr, bool hor, int stride) {
  m_setupVPMRead.numVecs = n;
  m_setupVPMRead.stride = stride;
  m_setupVPMRead.hor = hor;
  address_internal(addr);
}


void Stmt::setupVPMWrite(Expr::Ptr addr, bool hor, int stride) {
  m_setupVPMWrite.stride = stride;
  m_setupVPMWrite.hor = hor;
  address_internal(addr);
}


void Stmt::setupDMARead(bool is_horizontal, int numRows, Expr::Ptr addr, int rowLen, int vpitch) {
  m_setupDMARead.hor = is_horizontal?1 : 0;
  m_setupDMARead.numRows = numRows;
  m_setupDMARead.rowLen = rowLen;
  m_setupDMARead.vpitch = vpitch;
  address_internal(addr);
}


void Stmt::setupDMAWrite(bool is_horizontal, int numRows, Expr::Ptr addr, int rowLen) {
  m_setupDMAWrite.hor = is_horizontal? 1 : 0;
  m_setupDMAWrite.numRows = numRows;
  m_setupDMAWrite.rowLen = rowLen;
  address_internal(addr);
}


Expr::Ptr Stmt::stride_internal() {
  assert(m_exp.get() != nullptr);
  return m_exp;
}


bool Stmt::address(int in_tag, Expr::Ptr e0) {
  auto tag = to_tag(in_tag);

  if (is_dma_readwrite_tag(tag)
    || tag == V3DLib::Stmt::SET_READ_STRIDE
    || tag == V3DLib::Stmt::SET_WRITE_STRIDE) {

    address_internal(e0);
    return true;
  }

  return false;
}


void Stmt::address_internal(Expr::Ptr e0) {
  assertq(e0 != nullptr, "DMA set address");
  m_exp = e0;
}


Expr::Ptr Stmt::address_internal() {
  assert(m_exp.get() != nullptr);
  return m_exp;
}


std::string Stmt::pretty(int indent, int in_tag) {
  auto tag = to_tag(in_tag);
  std::string ret;

  switch (tag) {
    case V3DLib::Stmt::SET_READ_STRIDE:
      ret << indentBy(indent) << "dmaSetReadPitch(" << stride_internal()->pretty() << ");";
      break;

    case V3DLib::Stmt::SET_WRITE_STRIDE:
      ret << indentBy(indent) << "dmaSetWriteStride(" << stride_internal()->pretty() << ")";
      break;

    case V3DLib::Stmt::SEMA_INC:  // Increment semaphore
      ret << indentBy(indent) << "semaInc(" << m_semaId << ");";
      break;

    case V3DLib::Stmt::SEMA_DEC: // Decrement semaphore
      ret << indentBy(indent) << "semaDec(" << m_semaId << ");";
      break;

    case V3DLib::Stmt::SEND_IRQ_TO_HOST:
      ret << indentBy(indent) << "hostIRQ();";
      break;

    case V3DLib::Stmt::SETUP_VPM_READ:
      ret << indentBy(indent)
          << "vpmSetupRead("
          << "numVecs=" << m_setupVPMRead.numVecs               << ","
          << "dir="     << (m_setupVPMRead.hor ? "HOR" : "VIR") << ","
          << "stride="  << m_setupVPMRead.stride                << ","
          << address_internal()->pretty()
          << ");";
      break;

    case V3DLib::Stmt::SETUP_VPM_WRITE:
      ret << indentBy(indent)
          << "vpmSetupWrite("
          << "dir="    << (m_setupVPMWrite.hor ? "HOR" : "VIR") << ","
          << "stride=" << m_setupVPMWrite.stride                << ","
          << address_internal()->pretty()
          << ");";
      break;

    case V3DLib::Stmt::DMA_READ_WAIT:
      ret << indentBy(indent) << "dmaReadWait();";
      break;

    case V3DLib::Stmt::DMA_WRITE_WAIT:
      ret << indentBy(indent) << "dmaWriteWait();";
      break;

    case V3DLib::Stmt::DMA_START_READ:
      ret << indentBy(indent)
          << "dmaStartRead(" << address_internal()->pretty() << ");";
      break;

    case V3DLib::Stmt::DMA_START_WRITE:
      ret << indentBy(indent)
          << "dmaStartWrite(" << address_internal()->pretty() << ");";
      break;

    case V3DLib::Stmt::SETUP_DMA_READ:
      ret << indentBy(indent)
          << "dmaSetupRead("
          << "numRows=" << m_setupDMARead.numRows                  << ","
          << "rowLen=%" << m_setupDMARead.rowLen                   << ","
          << "dir="     << (m_setupDMARead.hor ? "HORIZ" : "VERT") << ","
          << "vpitch="  <<  m_setupDMARead.vpitch                  << ","
          << address_internal()->pretty()
          << ");";
      break;

    case V3DLib::Stmt::SETUP_DMA_WRITE:
      ret << indentBy(indent)
          << "dmaSetupWrite("
          << "numRows=" << m_setupDMAWrite.numRows                  << ","
          << "rowLen="  << m_setupDMAWrite.rowLen                   << ","
          << "dir="     << (m_setupDMAWrite.hor ? "HORIZ" : "VERT") << ","
          << address_internal()->pretty()
          << ");";
      break;

    default:
      break;
  }

  return ret;
}


/**
 * TODO test
 */
std::string disp(int in_tag) {
  auto tag = to_tag(in_tag);
  std::string ret;

  switch (tag) {
    case V3DLib::Stmt::SET_READ_STRIDE:  ret << "SET_READ_STRIDE";  break;
    case V3DLib::Stmt::SET_WRITE_STRIDE: ret << "SET_WRITE_STRIDE"; break;
    case V3DLib::Stmt::SEND_IRQ_TO_HOST: ret << "SEND_IRQ_TO_HOST"; break;
    case V3DLib::Stmt::SEMA_INC:         ret << "SEMA_INC";         break;
    case V3DLib::Stmt::SEMA_DEC:         ret << "SEMA_DEC";         break;
    case V3DLib::Stmt::SETUP_VPM_READ:   ret << "SETUP_VPM_READ";   break;
    case V3DLib::Stmt::SETUP_VPM_WRITE:  ret << "SETUP_VPM_WRITE";  break;
    case V3DLib::Stmt::SETUP_DMA_READ:   ret << "SETUP_DMA_READ";   break;
    case V3DLib::Stmt::SETUP_DMA_WRITE:  ret << "SETUP_DMA_WRITE";  break;
    case V3DLib::Stmt::DMA_READ_WAIT:    ret << "DMA_READ_WAIT";    break;
    case V3DLib::Stmt::DMA_WRITE_WAIT:   ret << "DMA_WRITE_WAIT";   break;
    case V3DLib::Stmt::DMA_START_READ:   ret << "DMA_START_READ";   break;
    case V3DLib::Stmt::DMA_START_WRITE:  ret << "DMA_START_WRITE";  break;

    default: break;
  }

  return ret;
}


bool Stmt::is_dma_readwrite_tag(int in_tag) {
  auto tag = to_tag(in_tag);

  switch (tag) {
    case V3DLib::Stmt::SETUP_VPM_READ:
    case V3DLib::Stmt::SETUP_VPM_WRITE:
    case V3DLib::Stmt::SETUP_DMA_READ:
    case V3DLib::Stmt::SETUP_DMA_WRITE:
    case V3DLib::Stmt::DMA_START_READ:
    case V3DLib::Stmt::DMA_START_WRITE:
      return true;

    default:
      return false;
  }
}


bool Stmt::is_dma_tag(int in_tag) {
  if (is_dma_readwrite_tag(in_tag)) return true;

  auto tag = to_tag(in_tag);

  switch (tag) {
    case V3DLib::Stmt::SET_READ_STRIDE:
    case V3DLib::Stmt::SET_WRITE_STRIDE:
    case V3DLib::Stmt::SEMA_INC:
    case V3DLib::Stmt::SEMA_DEC:
    case V3DLib::Stmt::SEND_IRQ_TO_HOST:
    case V3DLib::Stmt::DMA_READ_WAIT:
    case V3DLib::Stmt::DMA_WRITE_WAIT:
      return true;

    default:
      return false;
  }
}


}  // namespace DMA
}  // namespace V3DLib
