#include "catch.hpp"
#include <cstdlib>
#include "../Lib/Target/Emulator.h"  // MAX_QPUS
#include "../Lib/Support/Platform.h"

#ifdef QPU_MODE
#include "../Lib/vc4/vc4.h"
#include "../Lib/vc4/RegisterMap.h"
#include "../Lib/v3d/RegisterMapping.h"

using V3DLib::Platform;

TEST_CASE("Test correct working of RegisterMap", "[regmap]") {

	SECTION("Check num QPU's vc4") {
		if (!Platform::has_vc4()) return;
		const int MAX_QPUS = 12;

		using RegMap = V3DLib::RegisterMap;
		V3DLib::enableQPUs();  // Required for accessing the registers
 
		REQUIRE(4 == RegMap::numQPUPerSlice());
		REQUIRE(3 == RegMap::numSlices());
		REQUIRE(MAX_QPUS == RegMap::numSlices()*RegMap::numQPUPerSlice());

		V3DLib::disableQPUs();
	}

	SECTION("Check num QPU's v3d") {
		if (Platform::has_vc4()) return; 
		const int MAX_QPUS_V3D = 8;  // Expected num QPU's

		V3DLib::v3d::RegisterMapping map_v3d;  // TODO change to work with instance() call
		map_v3d.init();
		REQUIRE(1 == map_v3d.num_cores());  // This is a canary; warn me if this ever changes
		REQUIRE(MAX_QPUS_V3D == map_v3d.info_per_core(0).num_qpu);
	}
}

#endif  // QPU_MODE
