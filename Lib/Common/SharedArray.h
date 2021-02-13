#ifndef _V3DLIB_COMMON_SHAREDARRAY_H_
#define _V3DLIB_COMMON_SHAREDARRAY_H_
#include <vector>
#include "BufferObject.h"
#include "../Support/debug.h"
#include "../Support/Platform.h"  // has_vc4

namespace V3DLib {

class BaseSharedArray {
public:
  BaseSharedArray(BaseSharedArray &&a) = default;
  BaseSharedArray &operator=(BaseSharedArray &&a) = default; 

  bool allocated() const;
  uint32_t getAddress() { return m_phyaddr; }
  uint32_t size() const { return m_size; }

  void heap_view(BufferObject &heap) {
    assert(!allocated());
    assert(m_heap == nullptr);

    m_heap = &heap;
    m_is_heap_view = true;
    m_size = m_heap->size();
    assert(m_size > 0);
    m_usraddr = m_heap->usr_address();
    m_phyaddr = m_heap->phy_address();
  }

protected:
  BaseSharedArray() {}
  BaseSharedArray(BufferObject &heap) : m_heap(&heap) {}

  // TODO make member var's as private as possible
  BufferObject *m_heap = nullptr;  // Reference to used heap
  uint8_t *m_usraddr   = nullptr;  // Start of the heap in main memory, as seen by the CPU
  uint32_t m_phyaddr   = 0;        // Starting index of memory in GPU space
  uint32_t m_size      = 0;        // Number of contained elements (not memory size!)
  bool     m_is_heap_view = false;

  void alloc(uint32_t n, uint32_t element_size);
  void dealloc(uint32_t element_size);
  void *getPointer();

private:
  BaseSharedArray(BaseSharedArray const &a) = delete;  // Disallow copy
};


/**
 * Reserve and access a memory range in the underlying buffer object.
 *
 * All SharedArray instances use the same global BufferObject (BO) instance.
 * This is how the memory and v3d access already worked.
 *
 * For vc4, this is a change. Previously, each SharedArray instance had its
 * own BO. Experience will tell if this new setup works
 */
template <typename T>
class SharedArray : public BaseSharedArray {
  using Parent = BaseSharedArray;

public:
  SharedArray() {}
  SharedArray(uint32_t n) { Parent::alloc(n, sizeof(T)); }
  SharedArray(uint32_t n, BufferObject &heap) : BaseSharedArray(heap) { Parent::alloc(n, sizeof(T)); }
  SharedArray(SharedArray &&a) = default;
  SharedArray &operator=(SharedArray &&a) = default; 

  ~SharedArray() { Parent::dealloc(sizeof(T)); }

  void alloc(uint32_t n) { Parent::alloc(n, (uint32_t) sizeof(T)); }

  void fill(T val) {
    for (int i = 0; i < (int) size(); i++)
      (*this)[i] = val;
  }

  T *getPointer() { return (T *) Parent::getPointer(); }
  void dealloc() { Parent::dealloc(sizeof(T)); }
  T& operator[] (int i) { return access(i); }


  T operator[] (int i) const {
    assert(allocated());
    assert(i >= 0);
    assert(i < (int) m_size);

    T* base = (T *) m_usraddr;
    return (T) base[i];
  }


  /**
   * Subscript for access using physical address.
   *
   * Needed by interpreter and emulator.
   */
  inline T& phy(uint32_t i) {
    assert(m_phyaddr % sizeof(T) == 0);
    int index = (int) (i - ((uint32_t) m_phyaddr/sizeof(T)));
    return (*this)[index];
  }


  void copyFrom(T const *src, uint32_t size) {
    assert(src != nullptr);
    assert(size <= m_size);

    // TODO: consider using memcpy() instead
    for (uint32_t offset = 0; offset < size; ++offset) {
      (*this)[offset] = src[offset];
    }
  }

  void copyFrom(std::vector<T> const &src) {
    assert(!src.empty());
    assert(src.size() <= m_size);

    // TODO: consider using memcpy() instead
    for (uint32_t offset = 0; offset < src.size(); ++offset) {
      (*this)[offset] = src[offset];
    }
  }


  /**
   * Debug method for showing a range in a shared array
   */
  void dump(int first_offset, int last_offset) {
    assert(first_offset >= 0);
    assert(first_offset <= last_offset);
    assert(last_offset < (int) size());

    char const *format = "%8d: 0x%x - %d\n";

    for (int offset = first_offset; offset <= last_offset; ++offset) {
      printf(format, offset, (*this)[offset], (*this)[offset]);
    }

    printf("\n");
  }


  bool operator==(SharedArray const &rhs) const { 
    if (size() != rhs.size()) return false;

    for (int i = 0; i < (int) size(); ++i) {
      if ((*this)[i] != rhs[i]) return false;
    }

    return true;
  }

protected:

  T& access(int i) { 
    assert(allocated());
    assertq(i >= 0 && i < (int) m_size, "SharedArray::[]: index outside of possible range", true);

    T* base = (T *) m_usraddr;
    return (T&) base[i];
  }
};


template <typename T>
class Shared2DArray : private SharedArray<T> {
  using Parent = SharedArray<T>;

  struct Row {
    Row (Parent const *parent, int row, int row_size) :
      m_parent(const_cast<Parent *>(parent)),
      m_row(row),
      m_row_size(row_size) {}

    T operator[] (int col) const { return (*m_parent)[m_row*m_row_size + col]; }
    T &operator[] (int col)      { return (*m_parent)[m_row*m_row_size + col]; }

    Parent *m_parent;
    int m_row;
    int m_row_size;
  };

public:
  Shared2DArray(int rows, int columns) : SharedArray<T>(rows*columns),  m_rows(rows), m_columns(columns) {
    assert(rows > 0);
    assert(columns > 0);

    assertq(rows    % 16 == 0, "Shared2DArray: row dimension must be a multiple of 16");
    assertq(columns % 16 == 0, "Shared2DArray: column dimension must be a multiple of 16"); // TODO you sure? Check!
  }

  using Parent::fill;
  using Parent::getAddress;

  Parent const &get_parent() { return (Parent const &) *this; }  // explicit cast

  int rows()    const { return m_rows; }
  int columns() const { return m_columns; }

  /**
   * Copy values from square array `a` to array `b`, tranposing the array in the process
   */
  void copy_transposed(Shared2DArray const &rhs) {
    assertq(is_square() && rhs.is_square(), "copy_transposed(): can only transpose if both arrays are square");
    assertq(m_rows == rhs.m_rows, "copy_transposed(): can only copy if arrays have same dimension");

    int dim = m_rows;

    for (int r = 0; r < dim; r++) {
      for (int c = 0; c < dim; c++) {
        (*this)[c][r] = rhs[r][c];
      }
    }
  }

  bool is_square() const {
    return m_rows == m_columns;
  }

  Row operator[] (int row) {
    assert(0 <= row && row < m_rows);
    return Row(this, row, m_columns);
  }

  Row operator[] (int row) const {  // grumbl
    assert(0 <= row && row < m_rows);
    return Row(this, row, m_columns);
  }

  void make_unit_matrix() {
    assert(m_rows == m_columns);  // square matrices only

    int dim = m_columns;

    for (int r = 0; r < dim; r++) {
      for (int c = 0; c < dim; c++) {
        Parent::access(r*dim + c) = (r == c)? 1 : 0;
      }
    }
  }

private:
  int m_rows;
  int m_columns;
};

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_SHAREDARRAY_H_
