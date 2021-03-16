#ifndef _V3DLIB_COMMON_SET_H_
#define _V3DLIB_COMMON_SET_H_
#include "Seq.h"

namespace V3DLib {

template <class T>
class Set : private Seq<T> {
  using Parent = Seq<T>;

public:

  using Parent::size;
  using Parent::set_size;
  using Parent::clear;
  using Parent::empty;
  using Parent::operator[];

  Set() : Parent() {}
  Set(int initialSize) : Parent(initialSize) {}

  /**
   * Check if given value in sequence
   */
  bool member(T x) const {
    for (int i = 0; i < size(); i++) {
      if ((*this)[i] == x) return true;
    }
    return false;
  }


  /**
   * Insert element into sequence if not already present
   */
  bool insert(T x) {
    bool alreadyPresent = member(x);
    if (!alreadyPresent) Parent::append(x);
    return !alreadyPresent;
  }


  void add(Set<T> const &set) {
    for (int j = 0; j < set.size(); j++)
      insert(set[j]);
  }


  /**
   * Remove element from sequence used as set
   *
   * @return  true if element is removed, false otherwise
   */
  bool remove(T x) {
    int  count = 0;
    bool found_it = true;

    // TODO inefficient, optimize (don't care right now)
    while (found_it) {
      int index = -1;

      // Search for first occurance
      for (int i = 0; i < size(); i++) {
        if ((*this)[i] == x) {
          index = i;
          break;
        }
      }

      if (index != -1) {
        Parent::remove(index);
        count++;
      }

      found_it = (index != -1);
    }

    assert(count == 0 || count ==1);
    return (count != 0);
  }
};


/**
 * A small set is a set with a small initial size
 */
template <class T> class SmallSet : public Set<T> {
public:
  SmallSet() : Set<T>(8) {};
};

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_SET_H_
