#include "Optimizations.h"
#include <iostream>
#include "../Liveness.h"
#include "Support/Platform.h"
#include "Target/Subst.h"

namespace V3DLib {
namespace {

void replace_acc(Instr::List &instrs, RegUsageItem &item, int var_id, int acc_id) {
  Reg current(REG_A, var_id);
  Reg replace_with(ACC, acc_id);

  if (item.only_assigned()) {
    assert(item.use_range() == 1);
    renameDest(instrs[item.first_use()], current, replace_with);
    return;
  }

  int tmp = renameDest(instrs[item.first_use()], current, replace_with);
  assert(tmp != 0);  // Not expecting this

  // There can possibly be more assignments between first usage and live range
  // Following serves to capture them all - this might just be paranoia
  int tmp_count = 0;
  for (int i = item.first_use() + 1; i <= item.first_live() - 1; i++) {
    tmp_count += renameDest(instrs[item.first_dst()], current, replace_with);
  }

  if (tmp_count > 0) {
    std::string msg = "Detected extra assignments for var ";
    msg << var_id
        << " in range " << (item.first_use() + 1) << "-" << (item.first_live() - 1);

    debug(msg);
  }

  for (int i = item.first_live(); i <= item.last_live(); i++) {
    auto &instr = instrs[i];

    renameDest(instr, current, replace_with);
    //int tmp = renameDest(instr, current, replace_with);
    //assert(tmp == 0);  // Happens sporadically, eg. Hello vc4

    renameUses(instr, current, replace_with);
  }

  // DANGEROUS! Do not use this value downstream.   
  // Currently stored for debug display purposes only! 
  item.reg = replace_with;    
}


/**
 * TODO replace usage of this function with Instr::List::get_free_acc().
 *      Couldn't be bothered right now, nothing to gain here.
 */
Reg replacement_acc(Instr &prev, Instr &instr) {
    int acc_id =1;

    if (!Platform::compiling_for_vc4()) {
      // v3d ROT uses ACC1 (r1) internally, don't use it here
      // TODO better selection of subsitution ACC
      if (prev.isRot() || instr.isRot()) {
        //warning("introduceAccum(): subsituting ACC in ROT operation");
        acc_id = 2;
      }
    }

  return Reg(ACC, acc_id);
}


/**
 * Not as useful as I would have hoped. range_size > 1 in practice happens, but seldom.
 */
int peephole_0(int range_size, Instr::List &instrs, RegUsage &allocated_vars) {
  if (range_size == 0) {
    warning("peephole_0(): range_size == 0 passed in. This does nothing, not bothering");
    return 0;
  }

  int subst_count = 0;

  for (int var_id = 0; var_id < (int) allocated_vars.size(); var_id++) {
    auto &item = allocated_vars[var_id];
    assert(item.unused() || item.use_range() > 0);

    if (item.reg.tag != NONE) continue;
    if (item.unused()) continue;
    if (item.use_range() != range_size) continue;
    assert(range_size != 0 || item.only_assigned());

    // Guard for this special case for the time being.
    // It should actually be possible to load a uniform in an accumulator,
    // not bothering right now.
    if (instrs[item.first_use()].isUniformLoad()) continue;

    //
    // NOTE: There may be a slight issue here:
    //       in line of first use, src acc's may be used for vars which have
    //       last use in this line. I.e. they would be free for usage in this line.
    //
    // This is a small thing, perhaps for later optimization
    //
    int acc_id = instrs.get_free_acc(item.first_use(), item.last_use());

//    if (acc_id > 0) {
    if (range_size > 1) {
      std::string msg;
      msg << "range_size: " << range_size << ", Var " << var_id << ", "
          << "lines " << item.first_use()  << "-" << item.last_use() << ", "
          << "free acc: " << acc_id;
      debug(msg);
    }

    if (acc_id == -1 ) {
      warning("No free acc!");
      continue;
    }

    Reg replace_with(ACC, acc_id);
    allocated_vars.check_overlap_usage(replace_with, item);

    replace_acc(instrs, item, var_id, acc_id);

    subst_count++;
  }


  return subst_count;
}


/**
 * This is a simple peephole optimisation, captured by the following
 * rewrite rule:
 *
 *     i:  x <- f(...)
 *     j:  g(..., x, ...)
 * 
 * ===> if x not live-out of j
 * 
 *     i:  acc <- f(...)
 *     j:  g(..., acc, ...)
 *
 * @param allocated_vars  write param, to register which vars have an accumulator registered
 *
 * @return Number of substitutions performed;
 */
int peephole_1(Liveness &live, Instr::List &instrs, RegUsage &allocated_vars) {
  UseDef  useDefPrev;
  UseDef  useDefCurrent;
  LiveSet liveOut;
  int     subst_count = 0;

  for (int i = 1; i < instrs.size(); i++) {
    Instr prev  = instrs[i-1];
    Instr instr = instrs[i];

    useDefPrev.set_used(prev);        // Compute vars defined by prev
    if (useDefPrev.def.empty()) continue;
    RegId def = useDefPrev.def[0];

    useDefCurrent.set_used(instr);    // Compute vars used by instr
    live.computeLiveOut(i, liveOut);  // Compute vars live-out of instr

    // If 'instr' is not last usage of the found var, skip
    if (!(useDefCurrent.use.member(def) && !liveOut.member(def))) continue;

    // Can't remove this test.
    // Reason: There may be a preceding instruction which sets the var to be replaced.
    //         If 'prev' is conditional, replacing the var with an acc will ignore the previously set value.
    if (!prev.is_always()) {
/*
      std::string msg;
      msg << "peephole_1(): Skipping replacement for line " << i << " because prev.is_always() == false\n"
          << "prev : " << prev.dump()  << "\n"
          << "instr: " << instr.dump() << "\n";

      warning(msg);
*/
      continue;
    }

    Reg current(REG_A, def);
    Reg replace_with = replacement_acc(prev, instr);

    renameDest( prev, current, replace_with);
    renameUses(instr, current, replace_with);
    instrs[i-1] = prev;
    instrs[i]   = instr;

    // DANGEROUS! Do not use this value downstream.   
    // Currently stored for debug display purposes only! 
    allocated_vars[def].reg = replace_with;    

    subst_count++;
  }

  return subst_count;
}


int peephole_2(Liveness &live, Instr::List &instrs, RegUsage &allocated_vars) {
  UseDef  useDefCurrent;
  int     subst_count = 0;

  Instr prev;  // NOP

  for (int i = 1; i < instrs.size(); i++) {
    Instr instr = instrs[i];

    useDefCurrent.set_used(instr);    // Compute vars used by instr
    if (useDefCurrent.def.empty()) continue;
    assert(useDefCurrent.def.size() == 1);
    RegId def = useDefCurrent.def[0];

    if (!allocated_vars[def].only_assigned()) continue;

    Reg current(REG_A, def);
    Reg replace_with = replacement_acc(prev, instr);

    renameDest(instr, current, replace_with);
    instrs[i]   = instr;

    // DANGEROUS! Do not use this value downstream (remember why, old man?).   
    // Currently stored for debug display purposes only! 
    allocated_vars[def].reg = replace_with;    

    subst_count++;
  }

  return subst_count;
}

}  // anon namespace


void combineImmediates(Liveness &live, Instr::List &instrs) {
  for (int i = 0; i < (int) instrs.size(); i++) {
    Instr &instr = instrs[i];

    if (instr.tag == InstrTag::LI) {
      std::cout << "LI at " << i << ": " << instr.dump() << std::endl;
    }
  }
}


/**
 * Optimisation passes that introduce accumulators
 *
 * @param allocated_vars  write param, to register which vars have an accumulator registered
 *
 * @return Number of substitutions performed;
 *
 * ============================================================================
 * NOTES
 * =====
 *
 * * It is possible that a variable gets used multiple times, and the last usage of it
 *   is replaced by an accumulator.
 *
 *   For this reason, it is dangerous to keep track of the substitustion in `allocated_vars`,
 *   and to ignore the variable replacement due to acc usage later on. There may still be instances
 *   of the variable that need replacing.
 */
int introduceAccum(Liveness &live, Instr::List &instrs) {
  RegUsage &allocated_vars = live.reg_usage();

#ifdef DEBUG
  for (int i = 0; i < (int) allocated_vars.size(); i++) {
    assert(allocated_vars[i].reg.tag == NONE);  // Safeguard for the time being
  }
#endif  // DEBUG

  int subst_count = 0;

  //debug(allocated_vars.dump_use_ranges());
  // Picks up a lot usually, but range_size > 1 seldom results in something
  for (int range_size = 1; range_size <= 10; range_size++) {
    int count = peephole_0(range_size, instrs, allocated_vars);

    if (count > 0 && range_size > 1) {
      std::string msg = "peephole_0 for range size ";
      msg << range_size << ", num substs: " << count;
      debug(msg);
    }

    subst_count += count;
  }


  // This peephole still does a lot of useful stuff
  {
    int count = peephole_1(live, instrs, allocated_vars);

/*
    {
      std::string msg;
      msg << "peephole_1, num substs: " << count;
      debug(msg);
    }
*/

    subst_count += count;
  }

  // And some things still get done with this peephole
  {
    int count = peephole_2(live, instrs, allocated_vars);

/*
    {
      std::string msg;
      msg << "peephole_2, num substs: " << count;
      debug(msg);
    }
*/

    subst_count += count;
  }

  return subst_count;
}

}  // namespace V3DLib
