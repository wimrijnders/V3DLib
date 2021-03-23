#include "RegUsage.h"
#include "Support/basics.h"
#include "Support/Platform.h"  // size_regfile()
#include "../Liveness.h"
#include "UseDef.h"

namespace V3DLib {
namespace {

std::string get_unused_list(RegUsage const &alloc_list) {
  std::string ret;

  for (int i = 0; i < (int) alloc_list.size(); i++) {
    if (alloc_list[i].unused()) {
      ret << i << ",";
    }
  }

  return ret;
}


std::string get_never_assigned_list(RegUsage const &alloc_list) {
  std::string ret;

  for (int i = 0; i < (int) alloc_list.size(); i++) {
    if (alloc_list[i].never_assigned()) {
      ret << i << ",";
    }
  }

  return ret;
}


std::string get_assigned_only_list(RegUsage const &alloc_list) {
  std::string ret;

  // NOTE: var 0 (QPU id) skipped, this always gets set because passed in as uniform,
  //       but often does not get used.

  for (int i = 1; i < (int) alloc_list.size(); i++) {
    if (alloc_list[i].only_assigned()) {
      ret << i << ",";
    }
  }

  return ret;
}

}  // anon namespace

///////////////////////////////////////////////////////////////////////////////
// Class RegUsageItem
///////////////////////////////////////////////////////////////////////////////

bool RegUsageItem::unused() const {
  return (use.dst.empty() && use.src_use == 0);
}


std::string RegUsageItem::dump() const {
  std::string ret;

  if (unused()) {
    ret << "Not used";
    return ret;
  }

  ret << reg.dump() << "; ";

  ret << "use(src_first, src_last, src_count, dst): ("
      << use.src_first << ", "
      << use.src_last  << ", "
      << use.src_use   << ", {";

  for (int i = 0; i < (int) use.dst.size(); ++i) {
    if (i != 0) {
      ret << ", ";
    }
    ret << use.dst[i];
  }

  ret << "}); live(first, last, count): ("
      << live.first << ", " << live.last << ", " << live.count << ")";
  return ret;
}


void RegUsageItem::add_dst(int n) {
  assertq(use.dst.empty() || use.dst.back() < n, "RegUsageItem::add_dst() failed", true);
  use.dst << n;
}


void RegUsageItem::add_src(int n) {
  use.src_use++;

  if (use.src_first == -1 || use.src_first > n) {
    use.src_first = n;
  }

  if (use.src_last == -1 || use.src_last < n) {
    use.src_last = n;
  }
}


void RegUsageItem::add_live(int n) {
  if (live.first == -1 || live.first > n) {
    live.first = n;
  }

  if (live.last == -1 || live.last < n) {
    live.last = n;
  }

  live.count++;
}


/**
 * Number of instructions over which this variable is live.
 *
 * Note that it is possible (in theory only, I hope), that there
 * are gaps of liveness possible within this range.
 * This could happen if the variable assigned to within this range.
 */
int RegUsageItem::live_range() const {
  if (live.first == -1 && live.last == -1) {
    assert(use.src_use == 0);  // dst_use may be nonzero!
    return 0;
  }

  assert(live.first != -1 && live.last != -1);

  // NOTE: this is not true: `(live.last - live.first + 1) == live.count)`,
  //       because there can conceivably be gaps for liveness, due to interim assignments.
  return (live.last - live.first + 1);
}


/**
 * Number of instructions from first assignment till last usage, inclusive.
 */
int RegUsageItem::use_range() const {
  if (unused()) return 0;

   int first = live.first;
  if (!use.dst.empty()) first = use.dst[0];  // Guard for case where var is read only (eg. dummy input)

  int last = live.last;
  if (last == -1) {                        // Guard for case where var is write only (eg. dummy output)
    last = first;
  }

  int ret = (last - first + 1);
  assert(ret > 0);  // really expecting something here
  return ret;
}


int RegUsageItem::first_dst() const {
  assert(!use.dst.empty());
  return use.dst[0];
}


/**
 * Return the first in in which this variable is used (either as src or dst)
 */
int RegUsageItem::first_usage() const {
  assert(use.src_first == -1 || use.src_first >= first_dst());
  assert(!use.dst.empty());
  return use.dst[0];
}


/**
 * Get last line number for which variable is used (either as src or dst)
 */
int RegUsageItem::last_usage() const {
  if (only_assigned()) return first_dst();
  assert(use.src_last == -1 || use.src_last >= use.src_first);
  return use.src_last;
}


bool RegUsageItem::use_overlaps(RegUsageItem const &rhs) const {
  return !((first_usage() > rhs.last_usage()) || (last_usage() < rhs.first_usage()));
}


///////////////////////////////////////////////////////////////////////////////
// Class RegUsage
///////////////////////////////////////////////////////////////////////////////

RegUsage::RegUsage(int numVars) : Parent(numVars) {
  reset();
}


void RegUsage::reset() {
  assert(size() > 0);

  for (int i = 0; i < (int) size(); i++) {
    auto &item = (*this)[i];
    item = RegUsageItem();
    item.reg.tag = NONE;
  }
}


void RegUsage::set_used(Instr::List &instrs) {
  for (int i = 0; i < instrs.size(); i++) {
    UseDef out;
    out.set_used(instrs[i]);

    for (int j = 0; j < out.def.size(); j++) {
      (*this)[out.def[j]].add_dst(i);
    }

    for (int j = 0; j < out.use.size(); j++) {
      (*this)[out.use[j]].add_src(i);
    }
  }
}


void RegUsage::set_live(Liveness &live) {
  for (int i = 0; i < live.size(); i++) {
    auto &item = live[i];

    for (int j = 0; j < item.size(); j++) {
      (*this)[item[j]].add_live(i);
    }
  }
}


/**
 * Check internal consistency of used variables
 *
 * If anything is detected here, it is a compile error.
 */
void RegUsage::check() const {
  std::string ret;


  std::string tmp;

  //
  // Following is pretty common and not much of an issue (any more).
  // E.g. It occurs if condition flags need to be set and the result of the 
  // operation is discarded.
  //
  // Might need to further specify this, i.e. by removing var's which are known
  // and intended to be used as dummy's (TODO?)
  //
/*
  tmp = get_assigned_only_list(*this);
  if (!tmp.empty()) {
    std::string msg = prefix;
    msg << "There are internal instruction variables which are assigned but never used.\n"
        << "Variables: " << tmp << "\n";
    warning(msg);
  }
*/

  {
    std::string tmp = get_never_assigned_list(*this);
    if (!tmp.empty()) {
      std::string msg;
      msg << "  There are internal instruction variables which are used but never assigned.\n"
          << "  Variables: " << tmp << "\n";

      ret << msg;
    }
  }

  {
    std::string tmp;

    for (int i = 0; i < (int) size(); i++) {
      auto const &item = (*this)[i];
      if (!item.regular_use()) continue;

      if (item.first_live() <= item.first_dst()) {
        tmp << "  Variable " << i << " is live before first assignment" << "\n";
      }
    }

    if (!tmp.empty()) {
      tmp << "\n  This can happen if the first assignment is in a conditional block (If, When, For etc).\n";
      ret << tmp;

      //debug(dump(true));
      breakpoint
    }
  }
 
  if (!ret.empty()) {
    std::string prefix = "RegUsage internal error(s) ";
    if (Platform::compiling_for_vc4()) {
      prefix << "vc4";
    } else {
      prefix << "v3d";
    }
    prefix << ":\n";

    error(prefix + ret, true);
  } 
}


std::string RegUsage::allocated_registers_dump() const {
  std::string ret;

  for (int i = 0; i < (int) size(); i++) {
    ret << i << ": " << (*this)[i].reg.dump() << "\n";
  }

  return ret;
}


std::string RegUsage::dump(bool verbose) const {
  if (empty()) return "<Empty>";

  if (!verbose) return allocated_registers_dump();

  bool const ShowUnused = false;

  std::string ret;

  for (int i = 0; i < (int) size(); i++) {
    if (ShowUnused || !(*this)[i].unused()) {
      ret << i << ": " << (*this)[i].dump() << "\n";
    }
  }

  std::string tmp = get_unused_list(*this);
  if (!tmp.empty()) {
    ret << "\nNot used: " << tmp << "\n";
  }

  tmp = get_assigned_only_list(*this);
  if (!tmp.empty()) {
    ret << "\nOnly assigned: " << tmp << "\n";
  }

  tmp = get_never_assigned_list(*this);
  if (!tmp.empty()) {
    ret << "\nNever assigned: " << tmp << "\n";
  }

  return ret;
}


std::string RegUsage::dump_use_ranges() const {
  std::string ret;

  Seq<int> ranges;

  for (int i = 0; i < (int) size(); ++i) {
    ranges.append((*this)[i].use_range());
  }


  for (int i = 0; i < ranges.size(); ++i) {
    ret << ranges[i] << ", ";
  }

  return ret;
}


/**
 * Check if found acc does not conflict with other uses of this acc.
 * It may have been assigned to another var whose use-range overlaps with current var.
 *
 * This is something waiting to happen. It has not occured yet.
 * This test serves as a canary; if it fires, we need to do something about the situation.
 */
void RegUsage::check_overlap_usage(Reg acc, RegUsageItem const &item) const {
  assert(acc.tag == ACC);
  assert(acc.regId >= 0);

  for (int i = 0; i < (int) size(); ++i) {
    auto const &cur = (*this)[i];
    if (cur.reg != acc) continue;

    assertq(!cur.use_overlaps(item), "Detected conflicting usage of replacement acc", true);
  }
}


}  // namespace V3DLib
