#ifndef _V3DLIB_SUPPORT_HEAPMANAGER_H_
#define _V3DLIB_SUPPORT_HEAPMANAGER_H_
#include <stdint.h>
#include <string>
#include <vector>

namespace V3DLib {

/**
 * Memory manager for controlled heap objects.
 *
 * Keeps track of allocated and freed memory, handles space allocation.
 */
class HeapManager {
public:
  HeapManager();
  HeapManager(HeapManager *object) = delete;

  virtual void alloc_mem(uint32_t size_in_bytes);

  uint32_t size() const { return m_size; }
  bool empty() const { return m_offset == 0; }
  std::string dump() const;

  // Intended for unit tests
  uint32_t num_free_ranges() const { return (uint32_t) m_free_ranges.size(); }

protected:
  int alloc_array(uint32_t size_in_bytes);
  void dealloc_array(uint32_t index, uint32_t size);
  void set_size(uint32_t val);
  void clear();
  bool is_cleared() const;

private:
  // Disallow assignment
  void operator=(HeapManager a);
  void operator=(HeapManager& a);

  uint32_t m_size   = 0;  // Total allocated size of derived heap/buffer object
  uint32_t m_offset = 0;

  struct FreeRange {
    uint32_t left  = 0;
    uint32_t right = 0;

    FreeRange(uint32_t l, uint32_t r) : left(l), right(r) {}  // required by std::vector
    unsigned size() const;
    bool empty() const { return size() == 0; }

    bool overlaps(FreeRange const &rhs) const {
      return !(rhs.left > right || rhs.right < left);
    }

    std::string dump() const;
  };

  std::vector<FreeRange> m_free_ranges;

  bool check_available(uint32_t n);
  void dealloc_array(FreeRange const in_range);
};

}  // namespace V3DLib

#endif  // _V3DLIB_SUPPORT_HEAPMANAGER_H_
