#ifndef _SUPPORT_PLATFORM_H
#define _SUPPORT_PLATFORM_H
#include <string>

struct PlatformInfo {
	std::string platform_id; 
	bool is_pi_platform;
	std::string chip_version;
	bool has_vc4 = false;

	PlatformInfo();
	bool use_main_memory() const { return m_use_main_memory; }
	void use_main_memory(bool val);
	void output();

private:
	bool m_use_main_memory = false;
};


class Platform {
public:
	static PlatformInfo const &instance();
	static void use_main_memory(bool val);

private:
	static PlatformInfo &instance_local();
};

#endif  // _SUPPORT_PLATFORM_H
