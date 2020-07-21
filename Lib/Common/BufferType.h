#ifndef _QPULIB_BUFFERTYPE_H_
#define _QPULIB_BUFFERTYPE_H_

namespace QPULib {

enum BufferType : int {
	HeapBuffer,
	Vc4Buffer,
	V3dBuffer  // Not used yet!
};

}  // namespace QPULib

#endif  //  _QPULIB_BUFFERTYPE_H_
