#include "support.h"
#include <sys/time.h>

double get_time() {
	struct timeval t;
	gettimeofday(&t, NULL);
	return (double)t.tv_sec + t.tv_usec * 1e-6;
}


/**
 * @param skip_nops  If true, don't compare nops in received output.
 *                   This indicates instructions which can't be generated to bytecode (yet)
 */
void match_kernel_outputs(
	std::vector<uint64_t> const &expected,
	std::vector<uint64_t> const &received,
	bool skip_nops) {
		using namespace QPULib::v3d::instr;
		auto _nop = nop();

		// Arrays should eventually match exactly, including length
		// For now, just check progress
		uint32_t len = expected.size();
		if (len > received.size()) {
			len = received.size();
		}

		// Outputs should match exactly
		for (uint32_t n = 0; n < len; ++n) {
			if (skip_nops && (_nop == received[n])) {
				//printf("nop at index %d\n", n);
				continue;
			}

			INFO("Comparing assembly index: " << n << ", code length: " << received.size() <<
				"\nExpected: " << Instr(expected[n]).dump() <<
				"Received: " << Instr(received[n]).dump()
			);

			REQUIRE(Instr::compare_codes(expected[n], received[n]));
		}
}
