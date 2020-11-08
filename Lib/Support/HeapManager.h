#ifndef _QPULIB_SUPPORT_HEAPMANAGER_H_
#define _QPULIB_SUPPORT_HEAPMANAGER_H_
#include <stdint.h>
#include <vector>

namespace QPULib {

/**
 * Memory manager for controlled heap objects.
 *
 * Keeps track of allocated and freed memory, handles space allocation.
 */
class HeapManager {
public:
	HeapManager();
	HeapManager(HeapManager *object) = delete;

	void dealloc_array(uint32_t in_phyaddr, uint32_t size);

	uint32_t size() const { return m_size; }
	bool empty() const { return m_offset == 0; }
	uint32_t phy_address() const { return phyaddr; }

#ifdef DEBUG
	int num_free_ranges() const { return m_free_ranges.size(); }
#endif

protected:
	uint32_t alloc_array(uint32_t size_in_bytes, uint32_t &out_offset);
	void set_size(uint32_t val);
	void set_phy_address(uint32_t val);
	bool check_available(uint32_t n);
	void clear();
	bool is_cleared() const;

private:
	// Disallow assignment
	void operator=(HeapManager a);
	void operator=(HeapManager& a);

  uint32_t m_size   = 0;  // Total allocated size of derived heap/buffer object
	uint32_t phyaddr  = 0;
	uint32_t m_offset = 0;

	struct FreeRange {
		FreeRange(uint32_t l, uint32_t r) : left(l), right(r) {}  // required by std::vector

		uint32_t left  = 0;
		uint32_t right = 0;
	};

	std::vector<FreeRange> m_free_ranges;
};

}  // namespace QPULib

#endif  // _QPULIB_SUPPORT_HEAPMANAGER_H_
