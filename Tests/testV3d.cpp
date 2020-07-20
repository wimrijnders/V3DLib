#include "catch.hpp"
#ifdef QPU_MODE
#include <sys/time.h>
#include <cstring>
#include "VideoCore/vc6/SharedArray.h"
#include "VideoCore/vc6/v3d.h"
#include "debug.h"



namespace {

static uint64_t do_nothing[] = {
    0x3c203186bb800000, // nop; thrsw
    0x3c203186bb800000, // nop; thrsw
    0x3c003186bb800000, // nop
    0x3c003186bb800000, // nop
    0x3c203186bb800000, // nop; thrsw
    0x3c003186bb800000, // nop
    0x3c003186bb800000, // nop
    0x3c003186bb800000, // nop
};


double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + t.tv_usec * 1e-6;
}

}  // anon namespace


/**
 * Adjusted from: https://gist.github.com/notogawa/36d0cc9168ae3236902729f26064281d
 */
TEST_CASE("Check v3d code is working properly", "[v3d]") {
	SECTION("Direct v3d calls should work properly") {
		REQUIRE(v3d_open());

    uint32_t handle = 0;
    uint32_t phyaddr = 0;
		void *usraddr = nullptr;
		REQUIRE(v3d_alloc(sizeof(do_nothing), handle, phyaddr, &usraddr));
		REQUIRE(handle != 0);
		REQUIRE(phyaddr != 0);
		REQUIRE(usraddr != nullptr);


		// Note that do_nothing is an array of 64-bit values!
		uint32_t array_length = sizeof(do_nothing)/sizeof(uint64_t);
		printf("array_length: %d\n", array_length);
		assert(array_length == 8);

    memcpy(usraddr, do_nothing, sizeof(do_nothing));

		QPULib::v3d::SharedArray<uint64_t> codeMem(array_length);

		for(int offset = 0; offset < array_length; ++offset) {
			codeMem[offset] = do_nothing[offset];
		}

		for(int offset = 0; offset < array_length; ++offset) {
			REQUIRE(codeMem[offset] == do_nothing[offset]);
		}
		breakpoint


		//
		// NOTE: During testing, execution time shot up from 0.1 sec to 10 sec.
		//       This probably due to the gpu hanging because of previously job faulty (too short)
		//       The 10 sec is likely the timeout.
		//
		// TODO: Find a way to reset the v3d
		//
		printf("[submit]\n");
		double start = get_time();
		REQUIRE(v3d_submit_csd(phyaddr, handle));
		double end = get_time();
		printf("[submit done: %.6lf sec]\n", end - start);

		REQUIRE(v3d_unmap(sizeof(do_nothing), handle, usraddr));
		REQUIRE(v3d_close());
	}

	SECTION("v3d SharedArray should work as expected") {
		const int size = 16;

		breakpoint
		QPULib::v3d::SharedArray<uint32_t> arr(size);
		REQUIRE(arr.size() == size);

		arr.dealloc();
		REQUIRE(arr.size() == 0);
	}
}


#endif  // QPU_MODE

