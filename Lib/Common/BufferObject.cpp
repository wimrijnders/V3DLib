#include "BufferObject.h"
#include "../Support/Platform.h"
#include "BufferType.h"
#include "../Target/BufferObject.h"
#include "../vc4/BufferObject.h"
#include "../v3d/BufferObject.h"

namespace {

const int INITIAL_FREE_RANGE_SIZE = 32;

}  // anon namespace


namespace QPULib {


BufferObject::BufferObject() {
	m_free_ranges.reserve(INITIAL_FREE_RANGE_SIZE);
}


uint32_t BufferObject::size() {
	return m_size;
}


/**
 * @return physical address for array if allocated, 
 *         0 if could not allocate.
 */
uint32_t BufferObject::alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address) {
	assert(m_size > 0);
	assert(size_in_bytes % 4 == 0);

	if (m_offset + size_in_bytes >= m_size) {
		return 0;
	}

	uint32_t prev_offset = m_offset;

	array_start_address = arm_base + m_offset;
	m_offset += size_in_bytes;
	return phyaddr + prev_offset;
}


/**
 * Mark given range as unused.
 *
 * This should be called from deallocating SharedArray instances, which allocated
 * from this BO.
 *
 * The goal is to detect if the entire BO is free of use. When that's the case, the
 * allocated size can be reset and the BO can be reused from scratch.
 *
 * Better would be to reuse empty spaces if possible; I'm pushing this ahead of me for now.
 * TODO Examine this
 */
void BufferObject::dealloc_array(uint32_t in_phyaddr, uint32_t size) {
	assert(phyaddr <= in_phyaddr && in_phyaddr < (phyaddr + m_size));
	assert(size > 0);


	uint32_t left  = in_phyaddr - phyaddr;
	uint32_t right = left + size - 1;

	// Find adjacent matches in current free range list
	int left_match_index  = -1;
	int right_match_index = -1;

	for (int i = 0; i < m_free_ranges.size(); ++i) {
		auto &cur = m_free_ranges[i];

		if (right + 1 == cur.left) {
			assert(right_match_index == -1);  // Expecting at most one match
			right_match_index = i;
		}

		if (left == cur.right + 1) {
			assert(left_match_index == -1); // At most one match
			left_match_index = i;
		}
	}


	if (left_match_index != -1) {
		if (right_match_index != -1) {
			// Both sides match
			assert(left_match_index != right_match_index);

			// Merge the three ranges to one
			m_free_ranges[left_match_index].right = m_free_ranges[right_match_index].right;

			// Move the last range to the one we want to delete
			auto tmp = m_free_ranges.back();
			m_free_ranges.pop_back();
			if (right_match_index != m_free_ranges.size()) {
				m_free_ranges[right_match_index] = tmp;
			} else {
			}

		} else {
			// Only left side matches
			m_free_ranges[left_match_index].right = right;
		}
	} else if (right_match_index != -1) {
		// Only right side matches
			m_free_ranges[right_match_index].left = left;
	} else {
		// No match, add given range to list
		m_free_ranges.push_back(FreeRange(left, right));

		assert(m_free_ranges.size() < INITIAL_FREE_RANGE_SIZE);  // Warn me when this ever happens
		                                                         // (possible, currently not likely)
	}

	// When fully deallocated, there should be one single range of the entire used space
	bool is_empty = false;
	if (m_free_ranges.size() == 1) {
		auto &tmp = m_free_ranges.front();
		is_empty = (tmp.left == 0) && (tmp.right == m_offset - 1);
	}

	if (is_empty) {
		// We're done, reset the buffer
		assert(m_offset > 0);  // paranoia
		m_offset = 0;          // yes, it's that simple
		m_free_ranges.clear();
		//debug("BufferObject empty again!");
	}
}


uint32_t BufferObject::getHandle() const {
	assert(false);  // Only use for v3d
	return 0;
}


BufferObject &getBufferObject() {
	if (Platform::instance().use_main_memory()) {
		return emu::getHeap();
	} else if (Platform::instance().has_vc4) {
		return vc4::getHeap();
	} else {
		return v3d::getMainHeap();
	}
}

}  // namespace QPULib
