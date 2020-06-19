#ifndef _EXAMPLE_SUPPORT_SETINGS_H
#define _EXAMPLE_SUPPORT_SETINGS_H
#include <CmdParameters.h>

namespace QPULib {

struct Settings {
	bool output_code;
#ifdef EMULATION_MODE
	int run_type;
#endif

	const char *code_filename = "target_code.txt";

	int init(int argc, const char *argv[]);
	void output();

	template<typename Kernel, typename... us>
	void process(Kernel &k, us... args) {

#ifdef EMULATION_MODE
		switch (run_type) {
			case 0: k(args...); break;
			case 1: k.emu(args...); break;
			case 2: k.interpret(args...); break;
		}
#endif
#ifdef QPU_MODE
		k(args...);
#endif

		if (output_code) {
			k.pretty(code_filename);
		}
	}
};


}  // namespace

#endif  // _EXAMPLE_SUPPORT_SETINGS_H
