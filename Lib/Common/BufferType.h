#ifndef _V3DLIB_BUFFERTYPE_H_
#define _V3DLIB_BUFFERTYPE_H_

// SharedArray's for v3d have not been fully tested yet
#define USE_V3D_BUFFERS

namespace V3DLib {

enum BufferType : int {
  HeapBuffer,
#ifdef USE_V3D_BUFFERS
  Vc4Buffer,
  V3dBuffer
#else
  Vc4Buffer
#endif
};

}  // namespace V3DLib

#endif  //  _V3DLIB_BUFFERTYPE_H_
