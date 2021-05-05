#include "HeapManager.h"
#include "Support/basics.h"  // fatal()

namespace  {

int const INITIAL_FREE_RANGE_SIZE = 32;

}  // anon namespace


namespace V3DLib {

void HeapManager::alloc(uint32_t size_in_bytes) {
  assert(size_in_bytes > 0);
  assertq(size() == 0, "HeapManager::alloc(): Buffer object already allocated");

  alloc_mem(size_in_bytes);
}

/**
 * Explicit virtual here to avoid compile errors
 * It should actually be a pure virtual.
 */
void HeapManager::alloc_mem(uint32_t size_in_bytes) {
  assertq(false, "HeapManager::alloc_mem(): this virtual method must be overridden in derived classes", true);
}


unsigned HeapManager::FreeRange::size() const {
  int ret = (int) right - (int) left + 1;
  assert(ret >= 0);  // empty range will have left + 1 == right

  return (unsigned) ret;
}


std::string HeapManager::FreeRange::dump() const {
  std::string ret;
  ret << "[" << left << ", " << right << "]";
  return ret;
}


HeapManager::HeapManager() {
  m_free_ranges.reserve(INITIAL_FREE_RANGE_SIZE);
}


void HeapManager::set_size(uint32_t val) {
  assert(val > 0);
  assert(m_size == 0);  // Only allow initial size setting for now
  m_size = val;
}


bool HeapManager::check_available(uint32_t n) {
  assert(n > 0);

  if (m_offset + n > m_size) {
    fatal("V3DLib: heap overflow (increase heap size)");  // throws, doesn't return
    return false;
  }

  return true;
}


void HeapManager::clear() {
  m_size = 0;
  m_offset = 0;
  m_free_ranges.clear();
}


bool HeapManager::is_cleared() const {
  if  (m_size == 0) {
    assert(m_offset == 0);
    assert(m_free_ranges.empty());
  }

  return (m_size == 0);
}


/**
 * @param size_in_bytes number of bytes to allocate
 *
 * @return Start offset into heap if allocated, -1 if could not allocate.
 */
int HeapManager::alloc_array(uint32_t size_in_bytes) {
  assert(m_size > 0);
  assert(size_in_bytes > 0);
  assert(size_in_bytes % 4 == 0);

  // Find the first available space that is large enough
  int found_index = -1;
  for (int i = 0; i < (int) m_free_ranges.size(); ++i) {
    auto &cur = m_free_ranges[i];

    if (size_in_bytes <= cur.size()) {
      found_index = i;
      break;
    }
  }


  if (found_index == -1) {
    // Didn't find a freed location, reserve from the end
    if (!check_available(size_in_bytes)) {
      return -1;
    }

    uint32_t prev_offset = m_offset;
    m_offset += size_in_bytes;
    return (int) prev_offset;
  }

  int ret = -1;

  auto &cur = m_free_ranges[found_index];
  ret = cur.left;
  cur.left += size_in_bytes;
  if (cur.empty()) {
    // remove from list
    m_free_ranges.erase(m_free_ranges.begin() + found_index);
  }

  return ret;
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
 *
 * @param index  index of memory range to deallocate
 * @param size   number of bytes to deallocate
 */
void HeapManager::dealloc_array(uint32_t index, uint32_t size) {
  assert(size > 0);
  assert((index + size - 1) < m_size);

  dealloc_array(FreeRange(index, index + size -1));
}


void HeapManager::dealloc_array(FreeRange const in_range) {
  assert(m_size > 0);

#ifdef DEBUG
  // Check if incoming range is already deallocated
  for (int i = 0; i < (int) m_free_ranges.size(); ++i) {
    auto &cur = m_free_ranges[i];

    if (!cur.overlaps(in_range)) continue;

    std::string msg;
    msg << "HeapManager::dealloc_array(): "
        << "range to deallocate " << in_range.dump() << " "
        << "overlaps with free range " << cur.dump();
    assertq(msg, true);
  }
#endif

  // Find adjacent matches in current free range list
  int left_match_index  = -1;
  int right_match_index = -1;

  for (int i = 0; i < (int) m_free_ranges.size(); ++i) {
    auto &cur = m_free_ranges[i];

    if (in_range.right + 1 == cur.left) {
      assert(right_match_index == -1);  // Expecting at most one match
      right_match_index = i;
    }

    if (in_range.left == cur.right + 1) {
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
      if (right_match_index != (int) m_free_ranges.size()) {
        m_free_ranges[right_match_index] = tmp;
      } else {
      }

    } else {
      // Only left side matches
      m_free_ranges[left_match_index].right = in_range.right;
    }
  } else if (right_match_index != -1) {
    // Only right side matches
      m_free_ranges[right_match_index].left = in_range.left;
  } else {
    // No match, add given range to list
    m_free_ranges.push_back(in_range);

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


std::string HeapManager::dump() const {
  std::string ret;

  uint32_t used_size = m_offset;

  for (int i = 0; i < (int) m_free_ranges.size(); i++) {
    auto const &item = m_free_ranges[i];
    used_size -= item.size();
  }

  ret << "HeapManager Usage\n"
      << "-----------------\n"
      << "  Size/used      : " << size() << ", " << used_size << "\n"
      << "  Num free ranges: " << num_free_ranges() << "\n";

  return ret;
}

}  // namespace V3DLib
