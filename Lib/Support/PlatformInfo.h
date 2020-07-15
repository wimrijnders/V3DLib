#ifndef _SUPPORT_PLATFORMINFO_H
#define _SUPPORT_PLATFORMINFO_H

struct PlatformInfo {
	std::string platform_id; 
	bool is_pi_platform;
	std::string chip_version;
	bool has_vc4 = false;

	PlatformInfo();
	void output(); 
};

#endif  // _SUPPORT_PLATFORMINFO_H
