#ifndef _V3DLIB_LIBSETTINGS_H_
#define _V3DLIB_LIBSETTINGS_H_

namespace V3DLib {

/**
 * Global settings for V3DLib
 */
class LibSettings {
public:
	static int qpu_timeout();
	static void qpu_timeout(int val);
};

}  // namespace V3DLib

#endif  // _V3DLIB_LIBSETTINGS_H_
