///////////////////////////////////////////////////////////////////////////////
// Liveness analysis
//
///////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "Support/basics.h"
#include "Support/Platform.h"
#include "Target/Subst.h"
#include "Target/Liveness.h"
#include "Common/CompileData.h"
#include "liveness/Optimizations.h"

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

  if (count > 0) {
    std::string msg;
    msg << "remove_replaced_instructions() removed " << count << " SKIPs";
    debug(msg);
  }
}

}  // anon namespace


///////////////////////////////////////////////////////////////////////////////
// Class LiveSet
///////////////////////////////////////////////////////////////////////////////

void LiveSet::add_not_used(LiveSet const &set, UseDef const &use ) {
  clear();
  for (int j = 0; j < set.size(); j++) {
    if (!use.def.member(set[j]))
      Parent::insert(set[j]);
  }
}


std::string LiveSet::dump() const {
  std::string ret;

  ret << "(";
  for (int j = 0; j < size(); j++) {
    ret << (*this)[j] << ", ";
  }
  ret << ")";

  return ret;
}


LiveSets::LiveSets(int size) : m_size(size) {
  assert(size > 0);
  m_sets = new LiveSet [size];  // confirmed: default ctor LiveSet called here (notably ctor SmallSeq)
}


LiveSets::~LiveSets() {
  delete [] m_sets;
}


void LiveSets::init(Instr::List &instrs, Liveness &live) {
  LiveSet liveOut;

  for (int i = 0; i < instrs.size(); i++) {
    live.computeLiveOut(i, liveOut);
    useDefSet.set_used(instrs[i]);

    for (int j = 0; j < liveOut.size(); j++) {
      RegId rx = liveOut[j];

      for (int k = 0; k < liveOut.size(); k++) {
        RegId ry = liveOut[k];
        if (rx != ry) (*this)[rx].insert(ry);
      }

      for (int k = 0; k < useDefSet.def.size(); k++) {
        RegId rd = useDefSet.def[k];
        if (rd != rx) {
          (*this)[rx].insert(rd);
          (*this)[rd].insert(rx);
        }
      }
    }
  }
}


LiveSet &LiveSets::operator[](int index) {
  assert(index >=0 && index < m_size);
  return m_sets[index];
}


/**
 * Determine the available register in the register file, to use for variable 'index'.
 *
 * @param index  index of variable
 */
std::vector<bool> LiveSets::possible_registers(int index, RegUsage &alloc, RegTag reg_tag) {
  assert(reg_tag == REG_A || reg_tag == REG_B);

  const int NUM_REGS = Platform::size_regfile();
  std::vector<bool> possible(NUM_REGS);

  for (int j = 0; j < NUM_REGS; j++)
    possible[j] = true;

  LiveSet &set = (*this)[index];

  // Eliminate impossible choices of register for this variable
  for (int j = 0; j < set.size(); j++) {
    Reg neighbour = alloc[set[j]].reg;
    if (neighbour.tag == reg_tag) possible[neighbour.regId] = false;
  }

  return possible;
}


/**
 * Debug function to output the contents of the possible-vector
 *
 * Returns a string of 0's and 1's for each slot in the possible-vector.
 * - '0' - in use, not available for assignment for variable with index 'index'.
 * - '1' - not in use, available for assignment
 *
 * This falls under the category "You probably don't need it, but when you need it, you need it bad".
 *
 * @param index - index value of current variable displayed. If `-1`, don't show. For display purposes only.
 */
void LiveSets::dump_possible(std::vector<bool> &possible, int index) {
  std::string buf = "possible: ";

  if (index >= 0) {
    if (index < 10) buf << "  ";
    else if (index < 100) buf << " ";

    buf << index;
  }
  buf << ": ";

  for (int j = 0; j < (int) possible.size(); j++) {
    buf << (possible[j]?"1":"0");
  }
  debug(buf.c_str());
}


/**
 * Find possible register in each register file
 */
RegId LiveSets::choose_register(std::vector<bool> &possible, bool check_limit) {
  assert(!possible.empty());
  RegId chosenA = -1;

  for (int j = 0; j < (int) possible.size(); j++)
    if (possible[j]) { chosenA = j; break; }

  if (check_limit && chosenA < 0) {
    error("LiveSets::choose_register(): register allocation failed, insufficient capacity", true);
  }

  return chosenA;
}  


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
    msg << " Liveness table:\n" << dump();
    debug(msg);
  }
*/

  m_reg_usage.set_live(*this);
  //debug(m_reg_usage.dump(true));

  // Adjust first usage in liveness, if necessary 
  for (int i = 0; i < (int) m_reg_usage.size(); ++i) {
    auto &item = m_reg_usage[i];

    if (item.unused() || item.only_assigned())     continue;  // skip special cases
    if (item.first_dst() + 1 == item.first_live()) continue;  // all is well

    {
      std::string msg;
      msg << "m_reg_usage[" << i << "]: discrepancy between first assign and liveness: " << item.dump();
      warning(msg);
    }

    // Remove all liveness of given variable `i` before and including first assign
    assert(item.first_dst() != -1);
    assert(item.first_live() != -1);
    int remove_count = 0;
    for (int j = item.first_live(); j <= item.first_dst(); ++j) {
      int num = m_set[j].remove(i);
      if (num == 1) remove_count++;
    }

    if (remove_count == 0) {
      std::string msg = "Liveness::compute(): ";
      msg << "failed to remove liveness for var " << i 
          << " in range (" << item.first_dst() << ", " << item.first_live() << ")\n"
          << " Usage item: " << item.dump() << "\n";
/*
          << " Liveness table:\n" << dump() << "\n"
          << " Reg usage:\n" << m_reg_usage.dump(true) << "\n"
          << " Code:\n" << instrs.dump(true) << "\n";
*/

      warning(msg);
    }
  }

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

  for (int j = 0; j < set.size(); j++) {
    if (m_set[index].insert(set[j])) {
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
    for (int j = 0; j < item.size(); j++) {
      if (did_first) {
        ret += ", ";
      } else {
        did_first = true;
      }
      ret += std::to_string(item[j]);
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

  Liveness live(numVars);
  live.compute(instrs);
  //live.dump();

  combineImmediates(live, instrs);
  //remove_replaced_instructions(instrs); - bad idea here, CFG  doesn't change

  live.compute(instrs);  // instructions may have changed in previous step, redo liveness

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
