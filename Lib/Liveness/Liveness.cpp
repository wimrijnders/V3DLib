///////////////////////////////////////////////////////////////////////////////
// Liveness Analysis
//
///////////////////////////////////////////////////////////////////////////////
#include "Liveness.h"
#include <iostream>
#include "Support/basics.h"
#include "Support/Platform.h"
#include "Target/Subst.h"
#include "Common/CompileData.h"
#include "Optimizations.h"

namespace V3DLib {
namespace {

int count_skips(Instr::List &instrs) {
  int ret = 0;

  for (int i = 0; i < (int) instrs.size(); i++) {
    if (instrs[i].tag == InstrTag::SKIP) {
      ret++;
    }
  }

  return ret;
}


/**
 * Replace the variables with the assigned registers for the given instruction
 *
 * This functions assigns real registers to the 'variable registers' of the instruction.
 *
 * The incoming instructions all have REG_A as registers but signify variables.
 * The reg id's indicate the variable at this stage.
 *
 * Allocation is first done with reg types TMP_A/TMP_B,
 * to avoid accidental replacements of registers with same id.
 * This has happened IRL.
 */
void allocate_registers(Instr &instr, RegUsage const &alloc) {

  auto check_regfile_register = [&instr] (Reg const &replace_with, RegId r) -> bool {
    if (replace_with.tag == REG_A) return true;
    if (Platform::compiling_for_vc4() && replace_with.tag == REG_B) return true;

    UseDefReg out;
    out.set_used(instr);

    std::string msg = "regAlloc(): allocated register must be in register file.";
    msg << "\n"
        << "Instruction: " << instr.dump() << ", "
        << "Registers: " << out.dump() << ", "
        << "Reg id : " << r << ", alloc value: " << replace_with.dump();

    error(msg, true);  // true: throw if there is an error

    return false;
  };

  UseDef useDefSet;
  useDefSet.set_used(instr);  // Registers only usage REG_A

  for (int j = 0; j < useDefSet.def.size(); j++) {
    RegId r = useDefSet.def[j];
    assert(!alloc[r].unused());
    Reg replace_with = alloc[r].reg;

    if (!check_regfile_register(replace_with, r)) continue;
    replace_with.tag = (replace_with.tag == REG_A)?TMP_A:TMP_B;

    renameDest(instr, Reg(REG_A, r), replace_with);
  }

  for (int j = 0; j < useDefSet.use.size(); j++) {
    RegId r = useDefSet.use[j];
    assert(!alloc[r].unused());
    Reg replace_with = alloc[r].reg;

    if (!check_regfile_register(replace_with, r)) continue;
    replace_with.tag = (replace_with.tag == REG_A)?TMP_A:TMP_B;

    renameUses(instr, Reg(REG_A, r), replace_with);
  }

  substRegTag(&instr, TMP_A, REG_A);
  substRegTag(&instr, TMP_B, REG_B);
}


/**
 * Removes all SKIP instructions from the list
 */
void remove_replaced_instructions(Instr::List &instrs) {
  int cur   = 0;
  int count = 0;

  while (cur < instrs.size()) {
    if (instrs[cur].tag == InstrTag::SKIP) {
      instrs.remove(cur);
      count++;
    } else {
      cur++;
    }
  }

/*
  if (count > 0) {
    std::string msg;
    msg << "remove_replaced_instructions() removed " << count << " SKIPs";
    debug(msg);
  }
*/
}

}  // anon namespace


///////////////////////////////////////////////////////////////////////////////
// Class Liveness
///////////////////////////////////////////////////////////////////////////////

/**
 * Determine the liveness sets for each instruction.
 */
void Liveness::compute_liveness(Instr::List &instrs) {
  // Initialise live mapping to have one entry per instruction
  setSize(instrs.size());

  UseDef useDef;

  // For temporarily storing live-in and live-out variables
  LiveSet liveIn;
  LiveSet liveOut;

  bool changed = true;
  int count = 0;

/*
  {
    std::string msg;
    msg << "compute_liveness CFG:\n"
        << m_cfg.dump();

    debug(msg);
}
*/

  // Iterate until no change, i.e. fixed point
  while (changed) {
    changed = false;

    // Propagate live variables backwards
    for (int i = instrs.size() - 1; i >= 0; i--) {
      auto &instr = instrs[i];

      bool also_set_used = false;
      if (instr.isCondAssign()) {
        UseDef useDef;
        useDef.set_used(instr);
        if (!useDef.def.empty()) {
          auto &item = m_reg_usage[useDef.def[0]];

          // If the dst variable is not used before, it should not be set as used as well
          assert(item.first_dst() <= i);
          also_set_used = (item.first_dst() < i);

          if (!also_set_used) {
            // Sanity check: in this case, we expect the variable to be in the condition assign block only
            AssignCond assign_cond = instr.assign_cond();
            for (int j = item.first_usage(); j <= item.last_usage(); j++) {
              assert((assign_cond == instrs[j].assign_cond())            // expected usage
                   || (instrs[j].is_always() && !instrs[j].is_branch())  // Interim basic usage allowed (happens)
              );
            }
          }
        }
      }

      // Compute 'use' and 'def' sets
      useDef.set_used(instr, also_set_used);
      computeLiveOut(i, liveOut);
      liveIn.add_not_used(liveOut, useDef);  // Remove the 'def' set from the live-out set to give live-in set
      liveIn.add(useDef.use);

      if (insert(i, liveIn)) {
        changed = true;
      }
    }

    count++;
  }

/*
  std::string msg;
  msg << "compute_liveness count: " << count;
  debug(msg);
*/
}


void Liveness::clear() {
  m_cfg.clear();
  m_set.clear();
  m_reg_usage.reset();

/*
  std::string msg;
  msg << "Liveness after Liveness::clear(): " << this->dump();
  debug(msg);
*/
}


void Liveness::compute(Instr::List &instrs) {
  clear();

  m_cfg.build(instrs);
  m_reg_usage.set_used(instrs);

  compute_liveness(instrs);
  assert(instrs.size() == size());

/*
  {
    std::string msg;
    msg << " CFG table:\n" << cfg().dump();
    debug(msg);
}
*/

  m_reg_usage.set_live(*this);
/*
  if (!Platform::compiling_for_vc4()) {
    debug(m_reg_usage.dump(true));
    breakpoint
  }
*/
  compile_data.reg_usage_dump = m_reg_usage.dump(true);
  compile_data.liveness_dump = dump();

  m_reg_usage.check();
}


/**
 * Compute live sets for each instruction
 *
 * Compute the live-out variables of an instruction, given the live-in
 * variables of all instructions and the CFG.
 */
void Liveness::computeLiveOut(InstrId i, LiveSet &liveOut) {
  liveOut.clear();
  Succs &s = m_cfg[i];

  for (int j = 0; j < s.size(); j++) {
    LiveSet &set = get(s[j]);
    liveOut.add(set);
  }
}


void Liveness::setSize(int size) {
  m_set.set_size(size);
}


bool Liveness::insert(int index, LiveSet const &set) {
  bool changed = false;

  for (auto it : set) {
    bool present = m_set[index].member(it);

    m_set[index].insert(it);
    assert(m_set[index].member(it));

    if (!present) {
      changed = true;
    }
  }

  return changed;
}


std::string Liveness::dump() {
  std::string ret;

  for (int i = 0; i < m_set.size(); ++i) {
    ret += std::to_string(i) + ": ";

    auto &item = m_set[i];
    bool did_first = false;

    for (auto it : item) {
      if (did_first) {
        ret += ", ";
      } else {
        did_first = true;
      }
      ret += std::to_string(it);
    }

    ret += "\n";
  }

  if (ret.empty()) ret += "<Empty>";

  ret += "\n";

  return ret;
}


/**
 * Introduce optimizations where possible in the instruction list
 *
 * This is done before the actual liveness analysis.
 * The idea is to minimize beforehand the number of variables considered
 * in the liveness analysis.
 */
void Liveness::optimize(Instr::List &instrs, int numVars) {
  assertq(count_skips(instrs) == 0, "optimize(): SKIPs detected in instruction list");

  compile_data.target_code_before_optimization = instrs.dump();

  Liveness live(numVars);
  live.compute(instrs);
  //live.dump();

  if (combineImmediates(live, instrs)) {
    live.compute(instrs);  // instructions have changed, redo liveness
  }

  int prev_count_skips = count_skips(instrs);
  compile_data.num_accs_introduced = introduceAccum(live, instrs);
  assertq(prev_count_skips == count_skips(instrs), "SKIP count changed after introduceAccum()");

  remove_replaced_instructions(instrs);
  assertq(count_skips(instrs) == 0, "optimize(): SKIPs detected in instruction list after cleanup");

  //std::cout << count_reg_types(instrs).dump() << std::endl;
  compile_data.target_code_before_liveness = instrs.dump();
}


void allocate_registers(Instr::List &instrs, RegUsage const &alloc) {
  for (int i = 0; i < instrs.size(); i++) {
    allocate_registers(instrs.get(i), alloc);
  }
}

}  // namespace V3DLib
