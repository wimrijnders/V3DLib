#ifndef _EXAMPLE_SUPPORT_SETTINGS_H
#define _EXAMPLE_SUPPORT_SETTINGS_H
#include <cassert>
#include <string>
#include <CmdParameters.h>

namespace V3DLib {

class KernelBase;

struct Settings {
	std::string name;

	bool output_code;
	bool compile_only;
	bool silent;
	int  run_type;
	int  num_qpus = 1;
#ifdef QPU_MODE
	bool   show_perf_counters;
#endif  // QPU_MODE

	CmdParameters const &base_params(bool use_numqpus = false);
	int init(int argc, const char *argv[]);
	bool process(CmdParameters *in_params = nullptr, bool use_numqpus = false);
	void process(KernelBase &k);

protected:
	int output_count = 0;

	void set_name(const char *in_name);

private:
	void startPerfCounters();
	void stopPerfCounters();
};


}  // namespace

#endif  // _EXAMPLE_SUPPORT_SETTINGS_H
