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


void RegUsageItem::add_dst(int n) {
  assertq(use_dst.empty() || use_dst.back() < n, "RegUsageItem::add_dst() failed", true);
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

  int first = m_live_range.first();
  if (!use_dst.empty()) first = use_dst[0];  // Guard for case where var is read only (eg. dummy input)

  int last = m_live_range.last();
  if (last == -1) {                        // Guard for case where var is write only (eg. dummy output)
    last = first;
  }

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
    UseDef out(instrs[i]);

    for (auto r : out.def) {
      (*this)[r].add_dst(i);
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

  //
  // Following is pretty common and not much of an issue (any more).
  // E.g. It occurs if condition flags need to be set and the result of the 
  // operation is discarded.
  //
  // Might need to further specify this, i.e. by removing var's which are known
  // and intended to be used as dummy's (TODO?)
  //
/*
  std::string tmp;
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

/*
RegUsageItem &RegUsageItem::find(RegId id) {
  assert(id >= 0 && id < size());
  assert((*this)[id].tag == NONE); 
  return (*this)[id];
}
*/

}  // namespace V3DLib
