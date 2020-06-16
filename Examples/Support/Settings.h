#ifndef _EXAMPLE_SUPPORT_SETINGS_H
#define _EXAMPLE_SUPPORT_SETINGS_H
#include <CmdParameters.h>

namespace QPULib {

struct Settings {
	bool output_code;

	const char *code_filename = "target_code.txt";

	int init(int argc, const char *argv[]);
	void output();

	template<typename Kernel>
	void process(Kernel &k) {
		if (output_code) {
			k.pretty(code_filename);
		}
	}
};


}  // namespace

#endif  // _EXAMPLE_SUPPORT_SETINGS_H
