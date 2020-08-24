#ifndef _QPULIB_BUFFERTYPE_H_
#define _QPULIB_BUFFERTYPE_H_

// SharedArray's for v3d have not been fully tested yet
#define USE_V3D_BUFFERS

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
