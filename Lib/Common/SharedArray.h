#ifndef _V3DLIB_COMMON_SHAREDARRAY_H_
#define _V3DLIB_COMMON_SHAREDARRAY_H_
#include <vector>
#include "BufferObject.h"
#include "../Support/basics.h"
#include "../Support/Platform.h"  // has_vc4

namespace V3DLib {

class BaseSharedArray {
public:
  BaseSharedArray(BaseSharedArray &&a) = default;
  BaseSharedArray &operator=(BaseSharedArray &&a) = default; 

  void alloc(uint32_t n);
  void dealloc();
  bool allocated() const;
  uint32_t getAddress() const { return m_phyaddr; }
  uint32_t size() const { return m_size; }
  uint32_t *getPointer();

  void heap_view(BufferObject &heap);

protected:
  uint8_t *m_usraddr = nullptr;  // Start of the heap in main memory, as seen by the CPU

  BaseSharedArray(BufferObject *heap, uint32_t element_size);
  BaseSharedArray(uint32_t element_size) : BaseSharedArray(nullptr, element_size) {}

 /**
  * Get actual offset within the heap for given index of current shared array.
  */
  uint32_t phy(uint32_t i) {
    assert(m_phyaddr % m_element_size == 0);
    int index = (int) (i - ((uint32_t) m_phyaddr/m_element_size));
    assert(index >= 0);
    return (uint32_t) index;
  }

private:
  BufferObject *m_heap = nullptr;  // Reference to used heap
  uint32_t const m_element_size;
  uint32_t m_phyaddr   = 0;        // Starting index of memory in GPU space
  uint32_t m_size      = 0;        // Number of contained elements (not memory size!)
  bool     m_is_heap_view = false;

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
  SharedArray() : BaseSharedArray(sizeof(T)) {}
  SharedArray(uint32_t n) : SharedArray() { Parent::alloc(n); }
  SharedArray(uint32_t n, BufferObject &heap) : BaseSharedArray(&heap, sizeof(T)) { Parent::alloc(n); }

  SharedArray(SharedArray &&a) = default;
  SharedArray &operator=(SharedArray &&a) = default; 

  ~SharedArray() { Parent::dealloc(); }

  T const *ptr() const { return (T *) m_usraddr; }  // Return pointer to data in main memory
  T *ptr() { return (T *) m_usraddr; }

  void fill(T val) {
    assert(allocated());
    //assertq(allocated(), "Can not fill unallocated array");

    for (int i = 0; i < (int) size(); i++)
      (*this)[i] = val;
  }


  T& operator[] (int i) { return access(i); }

  T operator[] (int i) const {
    assert(allocated());
    assert(i >= 0);
    assert(i < (int) size());

    T* base = (T *) m_usraddr;
    return (T) base[i];
  }


  /**
   * Subscript for access using physical address.
   *
   * Needed by interpreter and emulator.
   */
  inline T& phy(uint32_t i) {
    return (*this)[Parent::phy(i)];
  }


  void copyFrom(T const *src, uint32_t in_size) {
    assert(src != nullptr);
    assert(in_size <= size());

    // TODO: consider using memcpy() instead
    for (uint32_t offset = 0; offset < in_size; ++offset) {
      (*this)[offset] = src[offset];
    }
  }

  void copyFrom(std::vector<T> const &src) {
    assert(!src.empty());
    assert(src.size() <= size());

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
    assertq(i >= 0 && i < (int) size(), "SharedArray::[]: index outside of possible range", true);

    T* base = (T *) m_usraddr;
    return (T&) base[i];
  }
};


template <typename T>
class Shared2DArray : private SharedArray<T> {
  using Parent = SharedArray<T>;

public:
  // made public for Complex::Array2D. In all other cases should be regarded as private
  // TODO examine if this can be enforced
  struct Row {
    Row(Shared2DArray const *parent, int row, int row_size) :
      m_parent(const_cast<Parent *>((Parent const *) parent)),
      m_row(row),
      m_row_size(row_size) {}

    T operator[] (int col) const { return (*m_parent)[m_row*m_row_size + col]; }
    T &operator[] (int col)      { return (*m_parent)[m_row*m_row_size + col]; }

    Parent *m_parent;
    int m_row;
    int m_row_size;
  };

  Shared2DArray() = default;

  Shared2DArray(int rows, int columns) : Parent(rows*columns),  m_rows(rows), m_columns(columns) {
    validate();
  }

  Shared2DArray(int dimension) : Shared2DArray(dimension, dimension) {}  // for square array

  void alloc(uint32_t rows, uint32_t columns) {
    m_rows = rows;
    m_columns = columns;
    validate();

    Parent::alloc(rows*columns);
  }

  using Parent::fill;
  using Parent::getAddress;
  using Parent::allocated;

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


  std::string dump() const {
    using ::operator<<;  // C++ weirdness
    std::string ret;

    for (int r = 0; r < rows(); ++r) {
      ret << "( ";

      for (int c = 0; c < columns(); ++c) {
        if ( c != 0 && c % 16 == 0) ret << "\n";

        ret << (*this)[r][c] << ", ";
      }

      ret << ")\n";
    }

    return ret;
  }

private:
  int m_rows    = -1;  // init to illegal value
  int m_columns = -1;

  void validate() {
    assert(m_rows > 0);
    assert(m_columns > 0);

    // TODO you sure about next? Check!
    assertq((m_rows*m_columns) % 16 == 0, "Shared2DArray: array size must be a multiple of 16");
  }
};


inline bool no_fractions(V3DLib::SharedArray<float> const &a) {
  bool ret = true;

  for (int i = 0; i < (int) a.size(); i++) {
    if (a[i] != (float) ((int) a[i])) {
      ret = false;
      break;
    }  
  }

  return ret;
}


using Data = SharedArray<uint32_t>;

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_SHAREDARRAY_H_
