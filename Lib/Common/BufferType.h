#ifndef _QPULIB_BUFFERTYPE_H_
#define _QPULIB_BUFFERTYPE_H_

namespace QPULib {

enum BufferType : int {
	HeapBuffer,
#ifdef USE_V3D_BUFFERS
	Vc4Buffer,
	V3dBuffer
#else
	Vc4Buffer
#endif
};

}  // namespace QPULib

#endif  //  _QPULIB_BUFFERTYPE_H_
