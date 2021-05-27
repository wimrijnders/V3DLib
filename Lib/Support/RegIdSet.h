#ifndef _LIB_SUPPORT_REGIDSET_H
#define _LIB_SUPPORT_REGIDSET_H
#include <set>
#include <string>

namespace V3DLib {

/**
 * TODO name is a misnomer, change
 */
class RegIdSet : public std::set<int> {
public:
  void add(RegIdSet const &rhs);
  void remove(RegIdSet const &rhs);
  void remove(int rhs);
  bool member(int rhs) const { return (find(rhs) != cend()); }
  int first() const;
  std::string dump() const;
};

}  // namespace V3DLib

#endif  // _LIB_SUPPORT_REGIDSET_H
