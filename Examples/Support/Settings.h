#ifndef _EXAMPLE_SUPPORT_SETINGS_H
#define _EXAMPLE_SUPPORT_SETINGS_H
#include <CmdParameters.h>
#include <string>

namespace QPULib {

struct Settings {
	std::string name;

	bool output_code;
	bool compile_only;
//#ifdef EMULATION_MODE
	int run_type;
//#endif

	int init(int argc, const char *argv[]);
	void output();

	template<typename Kernel, typename... us>
	void process(Kernel &k, us... args) {

		if (!compile_only) {
//#ifdef EMULATION_MODE
			switch (run_type) {
				case 0: k(args...); break;
				case 1: k.emu(args...); break;
				case 2: k.interpret(args...); break;
			}
//#endif
//#ifdef QPU_MODE
//		k(args...);
//#endif
		}

		if (output_code) {
			std::string code_filename = name + "_code.txt";
			k.pretty(code_filename.c_str());
		}
	}
};


}  // namespace

#endif  // _EXAMPLE_SUPPORT_SETINGS_H
