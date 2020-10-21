#ifndef _SUPPORT_PLATFORM_H
#define _SUPPORT_PLATFORM_H
#include <string>

struct PlatformInfo {
	friend class Platform;

	std::string platform_id; 
	bool is_pi_platform;
	std::string chip_version;
	bool has_vc4 = false;

	PlatformInfo();
	bool use_main_memory() const { return m_use_main_memory; }
	bool compiling_for_vc4() const { return m_compiling_for_vc4; }
	void output();
	int size_regfile() const;

private:
	bool m_use_main_memory   = false;
	bool m_compiling_for_vc4 = true;
};


class Platform {
public:
	static PlatformInfo const &instance();
	static void use_main_memory(bool val);
	static void compiling_for_vc4(bool val);

private:
	static PlatformInfo &instance_local();
};

#endif  // _SUPPORT_PLATFORM_H
