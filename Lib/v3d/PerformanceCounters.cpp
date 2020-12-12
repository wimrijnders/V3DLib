#ifdef QPU_MODE

#include "PerformanceCounters.h"
#include <sstream>
#include "RegisterMapping.h"
#include "../Support/debug.h"

namespace V3DLib {
namespace v3d {

using RM = RegisterMapping;


const char *PerformanceCounters::Description[PerformanceCounters::NUM_PERF_COUNTERS] = {
	"Unknown 0",
	"Unknown 1",
	"Unknown 2",
	"Unknown 3",
	"Unknown 4",
	"Unknown 5",
	"Unknown 6",
	"Unknown 7",
	"Unknown 8",
	"Unknown 9",
	"Unknown 10",
	"Unknown 11",
	"Unknown 12",
	"Unknown 13",
	"Unknown 14",
	"Unknown 15",
	"Unknown 16",
	"Unknown 17",
	"Unknown 18",
	"Unknown 19",
	"Unknown 20",
	"Unknown 21",
	"Unknown 22",
	"Unknown 23",
	"Unknown 24",
	"Unknown 25",
	"Unknown 26",
	"Unknown 27",
	"Unknown 28",
	"Unknown 29",
	"Unknown 30",
	"Unknown 31",
	"CORE_PCTR_CYCLE_COUNT"
};


PerformanceCounters::PerformanceCounters() {
	m_srcs = {CORE_PCTR_CYCLE_COUNT};    // init value from py-videocore6 example, using as default for now
	m_mask = (1 << m_srcs.size()) - 1;
}


void PerformanceCounters::enter() {
	auto &regmap = RegisterMapping::instance();

	regmap.core_write(core_id, RM::CORE_PCTR_0_EN, 0);

	for (int i = 0; i < m_srcs.size(); ++i) {
		assert(i < 1);  // TODO adjust for masks
		uint32_t offset = 0;
		uint32_t mask = 127 << offset;

		uint32_t val    = regmap.core_read(core_id, RM::CORE_PCTR_0_SRC_0);
		uint32_t newval = val & (mask ^ 1) + (m_srcs[i] << offset);

		regmap.core_write(core_id, RM::CORE_PCTR_0_SRC_0, newval);
	}

	regmap.core_write(core_id, RM::CORE_PCTR_0_CLR     , m_mask);
	regmap.core_write(core_id, RM::CORE_PCTR_0_OVERFLOW, m_mask);
	regmap.core_write(core_id, RM::CORE_PCTR_0_EN      , m_mask);
}


void PerformanceCounters::exit() {
	auto &regmap = RegisterMapping::instance();

	regmap.core_write(core_id, RM::CORE_PCTR_0_EN      , 0);
	regmap.core_write(core_id, RM::CORE_PCTR_0_CLR     , m_mask);
	regmap.core_write(core_id, RM::CORE_PCTR_0_OVERFLOW, m_mask);
}


/**
 * @brief Create a string representations of the enabled counters and their values.
 */
std::string PerformanceCounters::showEnabled() {
	auto &regmap = RegisterMapping::instance();

	int const SLOT_COUNT = 32;
	uint32_t bitMask = m_mask;
	std::ostringstream os;

	os << "Enabled counters:\n";

	for (int i = 0; i < SLOT_COUNT; ++i) {
		bool enabled = (0 != (bitMask & (1 << i)));
		if (!enabled) {
			//os << "   Performance counter slot " << i << " not enabled\n";
			continue;
		}

		assert(i < 1);  // TODO adjust for masks
		uint32_t offset = 0;
		uint32_t mask = 127;

		uint32_t sourceIndex = i;
		uint32_t val = (regmap.core_read(core_id, RM::CORE_PCTR_0_PCTR0) >> offset) & mask;
		uint32_t counterIndex = regmap.core_read(core_id, RM::CORE_PCTR_0_SRC_0);

		if (counterIndex < 0 || counterIndex >= NUM_PERF_COUNTERS) {
			os << "   WARNING: Performance counter index 0x" << std::hex << counterIndex << std::dec
			   << " out of bounds for slot index " << i << "\n";
		} else {
			os << "  " <<  Description[counterIndex] << ": " << val << "\n";
		}
	}

	return os.str();
}


}  // namespace v3d
}  // namespace V3DLib

#endif  // QPU_MODE
