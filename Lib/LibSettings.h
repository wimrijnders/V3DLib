#ifndef _V3DLIB_LIBSETTINGS_H_
#define _V3DLIB_LIBSETTINGS_H_

namespace V3DLib {

/**
 * Global settings for V3DLib
 */
class LibSettings {
public:
  static int  qpu_timeout();
  static void qpu_timeout(int val);
  static int  heap_size();
  static void heap_size(int val);
  static bool use_tmu_for_load();
  static void use_tmu_for_load(bool val);
};

}  // namespace V3DLib

#endif  // _V3DLIB_LIBSETTINGS_H_
