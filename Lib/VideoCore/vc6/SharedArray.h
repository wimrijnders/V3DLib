#ifndef _QPULIB_V3D_SHAREDARRAY_H_
#define _QPULIB_V3D_SHAREDARRAY_H_
#include <stdint.h>


namespace QPULib {
namespace v3d {

class SharedArrayBase {
public:
	~SharedArrayBase(); 

  uint32_t getAddress() { return  (uint32_t) usraddr; }
  uint32_t getHandle()  { return handle; }

private:
  uint32_t handle = 0;
  void* usraddr = nullptr;
  uint32_t m_size = 0;
};


template <typename T> class SharedArray : public SharedArrayBase {
public:
	SharedArray() {} 

private:
  // Disallow assignment & copying
  void operator=(SharedArray<T> a);
  void operator=(SharedArray<T>& a);
  SharedArray(const SharedArray<T>& a);
};


}  // namespace v3d
}  // namespace QPULib

#endif  // _QPULIB_V3D_SHAREDARRAY_H_
