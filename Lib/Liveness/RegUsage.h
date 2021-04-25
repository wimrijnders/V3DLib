#ifndef _V3DLIB_LIVENESS_REGUSAGE_H_
#define _V3DLIB_LIVENESS_REGUSAGE_H_
#include <vector>
#include <string>
#include "Target/instr/Instr.h"
#include "Range.h"

namespace V3DLib {

struct RegUsageItem {
  Reg reg;

  void add_dst(int n);
  void add_src(int n);
  void add_live(int n);
  bool unused() const;
  bool only_assigned() const  { return !use_dst.empty() && src_range.count() == 0; }
  bool never_assigned() const { return !unused() && use_dst.empty(); }
  std::string dump() const;
  int live_range() const;
  int use_range() const;
  int first_dst() const;
  int first_live() const      { return m_live_range.first(); }
  int last_live() const       { return m_live_range.last(); }
  int first_usage() const;
  int last_usage() const;
  bool use_overlaps(RegUsageItem const &rhs) const;

  bool regular_use() const {
    return !(unused() || only_assigned());
  }

private:

  Range src_range;           // First and last instructions where var is used as src
  std::vector<int> use_dst;  // List of line numbers where var is set
  Range m_live_range;
};


class Liveness;

struct RegUsage : private std::vector<RegUsageItem> {
  using Parent = std::vector<RegUsageItem>;
  using Parent::size;
  using Parent::operator[];

  RegUsage(int numVars);

  void reset();
  void set_used(Instr::List &instrs);
  void set_live(Liveness &live);
  std::string dump(bool verbose = false) const;
  void check() const;
  std::string dump_use_ranges() const;
  void check_overlap_usage(Reg acc, RegUsageItem const &item) const;

private:
  std::string allocated_registers_dump() const;
};

}  // namespace V3DLib

#endif  // _V3DLIB_LIVENESS_REGUSAGE_H_
