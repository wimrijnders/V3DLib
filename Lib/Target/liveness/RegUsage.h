#ifndef _V3DLIB_LIVENESS_REGUSAGE_H_
#define _V3DLIB_LIVENESS_REGUSAGE_H_
#include <vector>
#include <string>
#include "../instr/Instr.h"

namespace V3DLib {

struct RegUsageItem {
  Reg reg;

  void add_dst(int n);
  void add_src(int n);
  void add_live(int n);
  bool unused() const;
  bool only_assigned() const  { return !use.dst.empty() && use.src_use == 0; }
  bool never_assigned() const { return !unused() && use.dst.empty(); }
  std::string dump() const;
  int live_range() const;
  int use_range() const;
  int first_dst() const;
  int first_live() const      { return live.first; }
  int last_live() const       { return live.last; }
  int first_usage() const;
  int last_usage() const;
  bool use_overlaps(RegUsageItem const &rhs) const;

  bool regular_use() const {
    return !(unused() || only_assigned());
  }

private:

  struct {
    int src_use = 0;       // Number of times used as src in code
    int src_first = -1;    // First instruction where var is used as src
    int src_last = -1;     // Last instruction where var is used as src
    std::vector<int> dst;  // List of line numbers where var is set
  } use;

  struct {
    int first = -1;
    int last = -1;
    int count = 0;
  } live;
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
