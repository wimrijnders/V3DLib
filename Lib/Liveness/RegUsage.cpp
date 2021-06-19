#include "RegUsage.h"
#include "Support/basics.h"
#include "Support/Platform.h"  // size_regfile()
#include "Liveness.h"
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
  return (use_dst.empty() && src_range.count() == 0);
}


bool RegUsageItem::assigned_once() const {
  assert(!unused());
  return use_dst.size() == 1;
}


std::string RegUsageItem::dump() const {
  std::string ret;

  if (unused()) {
    ret << "Not used";
    return ret;
  }

  ret << reg.dump() << "; ";

  ret << "use(src_first, src_last, src_count, dst): ("
      << src_range.dump() << ", {";

  for (int i = 0; i < (int) use_dst.size(); ++i) {
    if (i != 0) {
      ret << ", ";
    }
    ret << use_dst[i];
  }

  ret << "}); live(first, last, count): (" << m_live_range.dump() << ")";
  return ret;
}


void RegUsageItem::add_dst(int n, bool is_cond_assign) {
  assertq(use_dst.empty() || use_dst.back() < n, "RegUsageItem::add_dst() failed", true);

/*
  // See disabled code where this is used

  if (is_cond_assign && !use_dst.empty()) {
    // Conditional assign counts as src access as well (remember why, old man?)
    add_src(n);
  }
*/

  use_dst << n;
}


void RegUsageItem::add_src(int n)  { src_range.add(n); }
void RegUsageItem::add_live(int n) { m_live_range.add(n); }


/**
 * Number of instructions over which this variable is live.
 *
 * Note that it is possible (in theory only, I hope), that there
 * are gaps of liveness possible within this range.
 * This could happen if the variable assigned to within this range.
 */
int RegUsageItem::live_range() const {
  return m_live_range.range();
}


/**
 * Number of instructions from first assignment till last usage, inclusive.
 */
int RegUsageItem::use_range() const {
  if (unused()) return 0;

#if 0
  if (only_assigned()) return 0;  // This is wrong for the normal case, used to solve issue with this code 

  //
  // Alternate way of calculating use range, without touching live range
  //
  // The idea here is to get rid of liveness analysis before optimization.
  // However, there are just so many cases to handle. The last one I ran into is (pseudo code):
  //
  //    A0 = 0
  //    If something
  //      A0 = 1
  //    End
  //
  //    dst = cmd A0,...
  //
  //  - As far as liveness is concerned, A0 is live from 'A0 = 0' onward, which is correct
  //  - src/dst analysis, however, does not see the If and infers that liveness is from `A0 = 1` onwards.
  //  - As far as dst-use is concerned, this is a non-issue, because A0 is used way past any assignments to it.
  //    Question is, how to handle?
  //
  // Stopped this for now because it is burning my brain cells.
  //

  // determine first write before src usage (there might be a dummy write before
  assertq(src_range.first() != -1, "oops", true);
  int first_write = -1;
  for (auto dst : use_dst) {
    if (dst >= src_range.first()) break;  // >= because instr can have reg as src as well as dst (eg. add src, src, 1)
    first_write = dst;
  }
  assert(first_write != -1);

  // Live range goes in after found dst
  int first_1 = first_write + 1;
  int last_1 = src_range.last();
  if (last_1 == -1) {                        // Guard for case where var is write only (eg. dummy output)
    last_1 = first_1;
  }
#endif

  //
  // Original way of determining use range
  //
  int first = m_live_range.first();
  //if (!use_dst.empty()) first = use_dst[0];  // Guard for case where var is read only (eg. dummy input)
  if (first == -1) {
    if (!use_dst.empty()) {
      first = use_dst[0] + 1;  // Guard for case where var is read only (eg. dummy input)
    }
  }

  int last = m_live_range.last();
  if (last == -1) {                        // Guard for case where var is write only (eg. dummy output)
    last = first;
  }

#if 0
  assert(first_1 == first);
  assert(last_1 <= last);  // Inequality: live range need not be the same as src usage.
                           // This happens with liveness analysis with conditional loop, where var is used
                           // only within that loop. The liveness of that var goes until the end of the loop,
                           // past last src usage.
                           //
                           // This looks like a bug in liveness, not sure.
                           // But then again, liveness is something of a black magic for me.
#endif

  int ret = (last - first + 1);
  assert(ret > 0);  // really expecting something here
  return ret;
}


int RegUsageItem::first_dst() const {
  assert(!use_dst.empty());
  return use_dst[0];
}


/**
 * Return the first line in which this variable is used (either as src or dst)
 */
int RegUsageItem::first_usage() const {
  assert(src_range.first() == -1 || src_range.first() >= first_dst());
  assert(!use_dst.empty());
  return use_dst[0];
}


/**
 * Get last line number for which variable is used (either as src or dst)
 */
int RegUsageItem::last_usage() const {
  if (only_assigned()) return first_dst();
  return src_range.last();
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
    if (!instrs[i].has_registers()) continue;

    UseDef out(instrs[i]);

    if (out.def.tag != NONE) {
      (*this)[out.def.regId].add_dst(i, instrs[i].isCondAssign());
    }

    for (auto r : out.use) {
      (*this)[r].add_src(i);
    }
  }
}


void RegUsage::set_live(Liveness &live) {
  for (int i = 0; i < live.size(); i++) {
    auto &item = live[i];

    for (auto it : item) {
      (*this)[it].add_live(i);
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

  // Case 'instruction variables which are assigned but never used'
  // is pretty common and not much of an issue (any more).
  // E.g. It occurs if condition flags need to be set and the result of the 
  // operation is discarded.
  //
  // Does not need to be tested.

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
      if (!item.regular_use())   continue;
      if (item.never_assigned()) continue;  // Tested in previous block

      if (item.first_live() <= item.first_dst()) {
        tmp << "  Variable " << i << " is live before first assignment" << "\n";
      }
    }

    if (!tmp.empty()) {
      tmp << "\n  This can happen if the first assignment is in a conditional block (If, When, For etc).\n";
      ret << tmp;

      //debug(dump(true));
      //breakpoint
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
