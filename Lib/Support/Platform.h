#ifndef _SUPPORT_PLATFORM_H
#define _SUPPORT_PLATFORM_H

struct PlatformInfo {
	std::string platform_id; 
	bool is_pi_platform;
	std::string chip_version;
	bool has_vc4 = false;

	PlatformInfo();
	void output(); 
};


class Platform {
public:
	static const PlatformInfo &instance();
};

#endif  // _SUPPORT_PLATFORM_H
