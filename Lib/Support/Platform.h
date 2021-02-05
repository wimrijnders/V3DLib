#ifndef _V3DLIB_SUPPORT_PLATFORM_H
#define _V3DLIB_SUPPORT_PLATFORM_H
#include <string>

namespace V3DLib {



class Platform {
public:
  static bool is_pi_platform();
  static std::string platform_info();
  static bool has_vc4();
  static void compiling_for_vc4(bool val);
  static bool compiling_for_vc4();
  static void use_main_memory(bool val);
  static bool use_main_memory();
  static int  size_regfile();
  static int  max_qpus();
  static int  gather_limit();
};

}  // namespace V3DLib


#endif  // _V3DLIB_SUPPORT_PLATFORM_H
