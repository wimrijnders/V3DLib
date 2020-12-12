#ifdef QPU_MODE

#include "PerformanceCounters.h"
#include "RegisterMapping.h"
#include "../Support/debug.h"

namespace V3DLib {
namespace v3d {

using RM = RegisterMapping;

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
 * Source returns array of values per src item
 */
void PerformanceCounters::result() {
	auto &regmap = RegisterMapping::instance();

	for (int i = 0; i < m_srcs.size(); ++i) {
		assert(i < 1);  // TODO adjust for masks
		uint32_t offset = 0;
		uint32_t mask = 127;

		uint32_t val = (regmap.core_read(core_id, RM::CORE_PCTR_0_PCTR0) >> offset) & mask;
		assert(false);  // TODO do something with this value
	}
}


}  // namespace v3d
}  // namespace V3DLib

#endif  // QPU_MODE
