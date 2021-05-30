#include "RegIdSet.h"
//#include <algorithm>  // merge
#include "basics.h"

namespace V3DLib {

void RegIdSet::add(RegIdSet const &rhs) {
/*
  // more than 2x slower than insert
  RegIdSet m;
	std::merge(begin(), end(), rhs.begin(), rhs.end(), inserter(m, m.end()));
   (*this) = m;
*/

  insert(rhs.begin(), rhs.end());  // Doesn't make much of a difference in speed

/*
  for (auto r : rhs) {
    insert(r);
  }
*/
}


void RegIdSet::remove(RegIdSet const &rhs) {
  // This didn't work previously, probably due to Seq::clear() not cleaning items up
  // See Liveness::clear().
  // Doesn't help much with speed
  erase(rhs.begin(), rhs.end());

/*
  for (auto r : rhs) {
    erase(r);
  }
*/
}


void RegIdSet::remove(int rhs) {
  erase(rhs);
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
