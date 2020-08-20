#ifndef _QPULIB_BUFFERTYPE_H_
#define _QPULIB_BUFFERTYPE_H_
//
// SharedArray's for v3d have not been properly implemented yet
// They are currently actually Buffer Objects (BO's), this needs to change
//
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
