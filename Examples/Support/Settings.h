#ifndef _EXAMPLE_SUPPORT_SETINGS_H
#define _EXAMPLE_SUPPORT_SETINGS_H
#include <cassert>
#include <string>
#include <CmdParameters.h>

namespace QPULib {

struct Settings {
	std::string name;

	bool output_code;
	bool compile_only;
	int run_type;
#ifdef QPU_MODE
	bool   show_perf_counters;
#endif  // QPU_MODE

	int init(int argc, const char *argv[]);
	void process(CmdParameters *in_params = nullptr);
	static CmdParameters &params();

	template<typename Kernel, typename... us>
	void process(Kernel &k, us... args) {
		startPerfCounters();

		if (!compile_only) {
			switch (run_type) {
				case 0: k(args...); break;
				case 1: k.emu(args...); break;
				case 2: k.interpret(args...); break;
			}
		}

		stopPerfCounters();

		if (output_code) {
			assert(!name.empty());
			std::string code_filename = name + "_code.txt";
			k.pretty(code_filename.c_str());
		}
	}

protected:
	void set_name(const char *in_name);

private:
	void startPerfCounters();
	void stopPerfCounters();
};


}  // namespace

#endif  // _EXAMPLE_SUPPORT_SETINGS_H
