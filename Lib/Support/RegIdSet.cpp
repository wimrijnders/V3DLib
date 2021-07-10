#include "RegIdSet.h"
//#include <algorithm>  // merge
#include "basics.h"

namespace V3DLib {

/**
  * Using the following idiom was more than 2x slower than insert:
  *
  *    RegIdSet m;
	*    std::merge(begin(), end(), rhs.begin(), rhs.end(), inserter(m, m.end()));
  *    (*this) = m;
  */
void RegIdSet::add(RegIdSet const &rhs) {
  insert(rhs.begin(), rhs.end());  // A tiny bit faster
/*
  for (auto const &r : rhs) {
    insert(r);
  }
*/
}


void RegIdSet::remove(RegIdSet const &rhs) {
  erase(rhs.begin(), rhs.end());  // Doesn't help much with speed

/*
  for (auto r : rhs) {
    erase(r);
  }
*/
}


int RegIdSet::first() const { assert(!empty()); return *cbegin(); }


std::string RegIdSet::dump() const {
  std::string ret;

  ret << "(";

  for (auto reg : *this) {
    ret << reg << ", ";
  }

  ret << ")";

  return ret;
}

}  // namespace V3DLib
