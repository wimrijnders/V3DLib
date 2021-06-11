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
#include "Support/Timer.h"
#include "UseDef.h"

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

    UseDefReg out(instr);

    std::string msg = "regAlloc(): allocated register must be in register file.";
    msg << "\n"
        << "Instruction: " << instr.dump() << ", "
        << "Registers: " << out.dump() << ", "
        << "Reg id : " << r << ", alloc value: " << replace_with.dump();

    error(msg, true);  // true: throw if there is an error

    return false;
  };

  UseDef useDefSet(instr);  // Registers only usage REG_A

  if (useDefSet.def.tag != NONE) {
    RegId r = useDefSet.def.regId; 
    assert(!alloc[r].unused());
    Reg replace_with = alloc[r].reg;

    if (check_regfile_register(replace_with, r)) {
      replace_with.tag = (replace_with.tag == REG_A)?TMP_A:TMP_B;
      renameDest(instr, Reg(REG_A, r), replace_with);
    }
  }

  for (auto r: useDefSet.use) {
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
 *
 * Creating a new list is MUCH faster than inline removal using Instr::remove().
 * i.e.  Remove 1857 SKIPs from kernel final size 140828
 *        - remove() -> 28.557023s
 *        - new list -> 0.170661s
 */
Instr::List  remove_replaced_instructions(Instr::List &instrs) {
  Instr::List ret;

  int cur   = 0;
  int count = 0;

  while (cur < instrs.size()) {
    if (instrs[cur].tag == InstrTag::SKIP) {
      count++;
    } else {
      ret << instrs[cur];
    }
    cur++;
  }

/*
  if (count > 0) {
    std::string msg;
    msg << "remove_replaced_instructions() removed " << count << " SKIPs";
    debug(msg);
  }
*/

  return ret;
}

}  // anon namespace


///////////////////////////////////////////////////////////////////////////////
// Class Liveness
///////////////////////////////////////////////////////////////////////////////

/**
 * Determine the liveness sets for each instruction.
 */
void Liveness::compute_liveness(Instr::List &instrs) {
  //Timer t("compute_liveness", true);

  // Initialise live mapping to have one entry per instruction
  setSize(instrs.size());
/*
  // Went wrong with Seq<>, you never know later on
  for (int i = 0; i < (int) m_set.size(); i++) {
    assert(m_set[i].empty());
  }
*/

  // For temporarily storing live-in and live-out variables
  RegIdSet liveIn;
  RegIdSet liveOut;

  bool changed = true;
  int count = 0;

  //Timer t2("compute_liveness loop intern");
  //Timer t3("compute liveOut");
  //Timer t4("compute use/def rest");
  //Timer t5("compute insert");

  // Iterate until no change, i.e. fixed point
  while (changed) {
    changed = false;

    // Propagate live variables backwards
    for (int i = instrs.size() - 1; i >= 0; i--) {
      auto &instr = instrs[i];

      bool also_set_used = false;

      if (instr.isCondAssign()) {  // no performance impact ~ 1.5%
        Reg dst = instr.dst_a_reg();

        if (dst.tag != NONE) {
          auto &item = m_reg_usage[dst.regId];

          // If the dst variable is not used before, it should not be set as used as well
          assert(item.first_dst() <= i);
          also_set_used = (item.first_dst() < i);

          if (!also_set_used) {
            // Sanity check: in this case, we expect the variable to be in the condition assign block only
            AssignCond assign_cond = instr.assign_cond();
            for (int j = item.first_usage(); j <= item.last_usage(); j++) {
              assertq((assign_cond == instrs[j].assign_cond())            // expected usage
                   || (instrs[j].is_always() && !instrs[j].is_branch()),  // Interim basic usage allowed (happens)
                ""
              );
            }
          }
        }
      }

      //t2.start();
      // Compute 'use' and 'def' sets
      UseDef useDef(instr, also_set_used);

      //t3.start();
      computeLiveOut(i, liveOut);
      //t3.stop();

      //t4.start();
      liveIn = liveOut;
      if (useDef.def.tag != NONE) {
        liveIn.remove(useDef.def.regId);  // Remove the 'def' set from the live-out set to give live-in set
      }
      liveIn.add(useDef.use);
      //t4.stop();

      //t5.start();
      if (insert(i, liveIn)) {
        changed = true;
      }
      //t5.stop();
      //t2.stop();
    }

    count++;
  }

  //t2.end();
  //t3.end();
  //t4.end();
  //t5.end();

/*
  std::string msg;
  msg << "compute_liveness num iterations: " << count;
  debug(msg);
*/
}


void Liveness::clear() {
  m_cfg.clear();
  m_set.clear();
  m_reg_usage.reset();
}


void Liveness::compute(Instr::List &instrs) {
  clear();

  m_cfg.build(instrs);
  m_reg_usage.set_used(instrs);

  //Timer t3("compute liveness", false);
  compute_liveness(instrs); // performance hog 23/28s
  //t3.end();
  assert(instrs.size() == size());

  m_reg_usage.set_live(*this);

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
void Liveness::computeLiveOut(InstrId i, RegIdSet &liveOut) {
  liveOut.clear();

  for (auto val : m_cfg[i]) {
    RegIdSet &set = get(val);
    liveOut.add(set);
  }
}


void Liveness::setSize(int size) {
  m_set.resize(size);
}


/**
 * @return true if something inserted, false otherwise
 */
bool Liveness::insert(int index, RegIdSet const &set) {
  auto &item = m_set[index];

  int prev_size = (int) item.size();
  item.add(set);
  return ((int) item.size() != prev_size);
}


std::string Liveness::dump() {
  std::string ret;

  for (int i = 0; i < (int) m_set.size(); ++i) {
    std::string line;
    line += std::to_string(i) + ": ";

    auto &item = m_set[i];
    bool did_first = false;

    for (auto it : item) {
      if (did_first) {
        line += ", ";
      } else {
        did_first = true;
      }
      line += std::to_string(it);
    }

    ret += line;
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

  //Timer t1("live compute");
  Liveness live(numVars);
  live.compute(instrs);
  //std::cout << live.dump() << std::endl;
  //t1.end();

  if (combineImmediates(live, instrs)) {
    //std::cout << "After combineImmediates:\n"; 
    //std::cout << instrs.dump(true) << std::endl;  // Useful sometimes for debug

    //Timer t3("combine immediates compute", true);
    live.compute(instrs);  // instructions have changed, redo liveness
    //std::cout << live.dump() << std::endl;
  }

  //Timer t3("introduceAccum");
  int prev_count_skips = count_skips(instrs);
  compile_data.num_accs_introduced = introduceAccum(live, instrs);
  assertq(prev_count_skips == count_skips(instrs), "SKIP count changed after introduceAccum()");
	//t3.end();

  // Times for following (now) insignificant

  instrs = remove_replaced_instructions(instrs);
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
