#include "DMA.h"
#include "Support/basics.h"
#include "Support/Helpers.h"

namespace V3DLib {
namespace DMA {

using ::operator<<;  // C++ weirdness

std::string pretty(int indent, Stmt::Ptr s) {
  std::string ret;

  switch (s->tag) {
    case SET_READ_STRIDE:
      ret << indentBy(indent) << "dmaSetReadPitch(" << s->stride()->pretty() << ");";
      break;

    case SET_WRITE_STRIDE:
      ret << indentBy(indent) << "dmaSetWriteStride(" << s->stride()->pretty() << ")";
      break;

    case SEMA_INC:  // Increment semaphore
      ret << indentBy(indent) << "semaInc(" << s->semaId << ");";
      break;

    case SEMA_DEC: // Decrement semaphore
      ret << indentBy(indent) << "semaDec(" << s->semaId << ");";
      break;

    case SEND_IRQ_TO_HOST:
      ret << indentBy(indent) << "hostIRQ();";
      break;

    case SETUP_VPM_READ:
      ret << indentBy(indent)
          << "vpmSetupRead("
          << "numVecs=" << s->setupVPMRead.numVecs               << ","
          << "dir="     << (s->setupVPMRead.hor ? "HOR" : "VIR") << ","
          << "stride="  << s->setupVPMRead.stride                << ","
          << s->address()->pretty()
          << ");";
      break;

    case SETUP_VPM_WRITE:
      ret << indentBy(indent)
          << "vpmSetupWrite("
          << "dir="    << (s->setupVPMWrite.hor ? "HOR" : "VIR") << ","
          << "stride=" << s->setupVPMWrite.stride                << ","
          << s->address()->pretty()
          << ");";
      break;

    case DMA_READ_WAIT:
      ret << indentBy(indent) << "dmaReadWait();";
      break;

    case DMA_WRITE_WAIT:
      ret << indentBy(indent) << "dmaWriteWait();";
      break;

    case DMA_START_READ:
      ret << indentBy(indent)
          << "dmaStartRead(" << s->address()->pretty() << ");";
      break;

    case DMA_START_WRITE:
      ret << indentBy(indent)
          << "dmaStartWrite(" << s->address()->pretty() << ");";
      break;

    case SETUP_DMA_READ:
      ret << indentBy(indent)
          << "dmaSetupRead("
          << "numRows=" << s->setupDMARead.numRows                  << ","
          << "rowLen=%" << s->setupDMARead.rowLen                   << ","
          << "dir="     << (s->setupDMARead.hor ? "HORIZ" : "VERT") << ","
          << "vpitch="  <<  s->setupDMARead.vpitch                  << ","
          << s->address()->pretty()
          << ");";
      break;

    case SETUP_DMA_WRITE:
      ret << indentBy(indent)
          << "dmaSetupWrite("
          << "numRows=" << s->setupDMAWrite.numRows                  << ","
          << "rowLen="  << s->setupDMAWrite.rowLen                   << ","
          << "dir="     << (s->setupDMAWrite.hor ? "HORIZ" : "VERT") << ","
          << s->address()->pretty()
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
std::string disp(StmtTag tag) {
  std::string ret;

  switch (tag) {
    case SET_READ_STRIDE:  ret << "SET_READ_STRIDE";  break;
    case SET_WRITE_STRIDE: ret << "SET_WRITE_STRIDE"; break;
    case SEMA_DEC:         ret << "SEMA_DEC";         break;
    case SETUP_VPM_READ:   ret << "SETUP_VPM_READ";   break;
    case SETUP_VPM_WRITE:  ret << "SETUP_VPM_WRITE";  break;
    case SETUP_DMA_READ:   ret << "SETUP_DMA_READ";   break;
    case SETUP_DMA_WRITE:  ret << "SETUP_DMA_WRITE";  break;
    case DMA_READ_WAIT:    ret << "DMA_READ_WAIT";    break;
    case DMA_WRITE_WAIT:   ret << "DMA_WRITE_WAIT";   break;
    case DMA_START_READ:   ret << "DMA_START_READ";   break;
    case DMA_START_WRITE:  ret << "DMA_START_WRITE";  break;

    default: break;
  }

  return ret;
}


}  // namespace DMA
}  // namespace V3DLib
