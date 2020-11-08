#include "HeapManager.h"
//#include <cassert>
#include "Support/basics.h"  // fatal()

namespace  {

int const INITIAL_FREE_RANGE_SIZE = 32;

}  // anon namespace


namespace QPULib {

HeapManager::HeapManager() {
	m_free_ranges.reserve(INITIAL_FREE_RANGE_SIZE);
}


/**
 * @param out_offset  offset of newly allocated address range
 *
 * @return physical address for array if allocated, 
 *         0 if could not allocate.
 */
uint32_t HeapManager::alloc_array(uint32_t size_in_bytes, uint32_t &out_offset) {
	assert(m_size > 0);
	assert(size_in_bytes % 4 == 0);

	if (!check_available(size_in_bytes)) {
		return 0;
	}

	uint32_t prev_offset = m_offset;
	out_offset = m_offset;
	m_offset += size_in_bytes;
	return phyaddr + prev_offset;
}


void HeapManager::set_size(uint32_t val) {
	assert(val > 0);
	assert(m_size == 0);  // Only allow initial size setting for now
	m_size = val;
}


void HeapManager::set_phy_address(uint32_t val) {
	assert(val > 0);
	assert(phyaddr == 0);  // Only allow initial size setting for now
	phyaddr = val;
}


bool HeapManager::check_available(uint32_t n) {
	assert(n > 0);

	if (m_offset + n >= m_size) {
		fatal("QPULib: heap overflow (increase heap size)\n");  // NOTE: doesn't return
		return false;
	}

	return true;
}


void HeapManager::clear() {
	m_size = 0;
	phyaddr = 0;
	m_offset = 0;
}


bool HeapManager::is_cleared() const {
	if  (m_size == 0) {
		assert(phyaddr == 0);
		assert(m_offset == 0);
	}

	return (m_size == 0);
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
void HeapManager::dealloc_array(uint32_t in_phyaddr, uint32_t size) {
	assert(phyaddr <= in_phyaddr && in_phyaddr < (phyaddr + m_size));
	assert(m_size > 0);
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

}  // namespace QPULib
