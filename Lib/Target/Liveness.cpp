///////////////////////////////////////////////////////////////////////////////
// Liveness analysis
//
///////////////////////////////////////////////////////////////////////////////
#include "Support/basics.h"    // fatal()
#include "Support/Platform.h"  // size_regfile()
#include "Target/Subst.h"
#include "Target/Liveness.h"
#include "Common/CompileData.h"

namespace V3DLib {
namespace {


/**
 * Same as `useDefReg()`, except only yields ids of registers in register file A.
 */
void useDef(Instr const &instr, UseDef* out, bool set_use_where = false) {
  UseDefReg set;
  useDefReg(instr, &set, set_use_where);
  out->use.clear();
  out->def.clear();
  for (int i = 0; i < set.use.size(); i++) {
    Reg r = set.use[i];
    if (r.tag == REG_A) out->use.append(r.regId);
  }
  for (int i = 0; i < set.def.size(); i++) {
    Reg r = set.def[i];
    if (r.tag == REG_A) out->def.append(r.regId);
  }
}


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

    useDef(prev, &useDefPrev);        // Compute vars defined by prev

    if (useDefPrev.def.empty()) continue;

    RegId def = useDefPrev.def[0];

    useDef(instr, &useDefCurrent);    // Compute vars used by instr

    live.computeLiveOut(i, liveOut);  // Compute vars live-out of instr

    bool do_it = (useDefCurrent.use.member(def) && !liveOut.member(def));
    if (!do_it) continue;

    // 20210312 This test still required
    // TODO see if it can be fixed
    if (!prev.is_always()) {
      std::string msg;
      msg << "peephole_1(): Skipping replacement for line " << i << " because prev.is_always() == false\n"
          << "prev : " << prev.dump()  << "\n"
          << "instr: " << instr.dump() << "\n";

      warning(msg);
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

    useDef(instr, &useDefCurrent);    // Compute vars used by instr
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
int introduceAccum(Liveness &live, Instr::List &instrs, RegUsage &allocated_vars) {
#ifdef DEBUG
  for (int i = 0; i < (int) allocated_vars.size(); i++) {
    assert(allocated_vars[i].reg.tag == NONE);  // Safeguard for the time being
  }
#endif  // DEBUG

  int subst_count = peephole_1(live, instrs, allocated_vars);
  subst_count += peephole_2(live, instrs, allocated_vars);

  return subst_count;
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
    useDefReg(instr, &out);

    std::string msg = "regAlloc(): allocated register must be in register file.";
    msg << "\n"
        << "Instruction: " << instr.dump() << ", "
        << "Registers: " << out.dump() << ", "
        << "Reg id : " << r << ", alloc value: " << replace_with.dump();

    error(msg, true);  // true: throw if there is an error

    return false;
  };

  UseDef useDefSet;
  useDef(instr, &useDefSet);  // Registers only usage REG_A

  for (int j = 0; j < useDefSet.def.size(); j++) {
    assert(!alloc[j].unused());
    RegId r = useDefSet.def[j];

    Reg replace_with = alloc[r].reg;

    if (!check_regfile_register(replace_with, r)) {
      continue;
    }

    replace_with.tag = (replace_with.tag == REG_A)?TMP_A:TMP_B;

    renameDest(instr, Reg(REG_A, r), replace_with);
  }

  for (int j = 0; j < useDefSet.use.size(); j++) {
    assert(!alloc[j].unused());
    RegId r = useDefSet.use[j];

    Reg replace_with = alloc[r].reg;

    if (!check_regfile_register(replace_with, r)) {
      continue;
    }

    replace_with.tag = (replace_with.tag == REG_A)?TMP_A:TMP_B;

    renameUses(instr, Reg(REG_A, r), replace_with);
  }

  substRegTag(&instr, TMP_A, REG_A);
  substRegTag(&instr, TMP_B, REG_B);
}


std::string get_unused_list(RegUsage const &alloc_list) {
  std::string ret;

  for (int i = 0; i < (int) alloc_list.size(); i++) {
    if (alloc_list[i].unused()) {
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


std::string get_never_assigned_list(RegUsage const &alloc_list) {
  std::string ret;

  for (int i = 0; i < (int) alloc_list.size(); i++) {
    if (alloc_list[i].never_assigned()) {
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
  return (use.dst_use == 0 && use.src_use == 0);
}


std::string RegUsageItem::dump() const {
  std::string ret;
  ret << reg.dump() << "; ";

  if (unused()) {
    ret << "Not used";
    return ret;
  }

  ret << "use(dst_first, src_first, dst_count,src_count): ("
      << use.dst_first << ", " << use.src_first << ", "
      << use.dst_use << ", " << use.src_use << "), "
      << "; live(first, last, count): (" << live.first << ", " << live.last << ", " << live.count << ")";
  return ret;
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


///////////////////////////////////////////////////////////////////////////////
// Class RegUsageItem
///////////////////////////////////////////////////////////////////////////////

RegUsage::RegUsage(int numVars) : Parent(numVars) {
  for (int i = 0; i < numVars; i++) (*this)[i].reg.tag = NONE;
}


void RegUsage::set_used(Instr::List &instrs) {
  for (int i = 0; i < instrs.size(); i++) {
    UseDef out;
    useDef(instrs[i], &out);


    for (int j = 0; j < out.def.size(); j++) {
      auto &use = (*this)[out.def[j]].use;

      if (use.dst_first == -1 || use.dst_first > i) {
        use.dst_first = i;
      }

      use.dst_use++;
    }

    for (int j = 0; j < out.use.size(); j++) {
      auto &use = (*this)[out.use[j]].use;
      use.src_use++;

      if (use.src_first == -1 || use.src_first > i) {
        use.src_first = i;
      }
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


void RegUsage::check() const {
  std::string prefix = "RegUsage in regAlloc() ";
  if (Platform::compiling_for_vc4()) {
    prefix << "vc4";
  } else {
    prefix << "v3d";
  }
  prefix << ": ";

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

  tmp = get_never_assigned_list(*this);
  if (!tmp.empty()) {
    std::string msg = prefix;
    msg << "There are internal instruction variables which are used but never assigned.\n"
        << "Variables: " << tmp << "\n";
    error(msg, true);
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


///////////////////////////////////////////////////////////////////////////////
// Class UseDefReg
///////////////////////////////////////////////////////////////////////////////

std::string UseDefReg::dump() const {
  std::string ret;

  ret << "(def: ";
  for (int j = 0; j < def.size(); j++) {
    ret << def[j].dump();
  }
  ret << "; ";

  ret << "use: ";
  for (int j = 0; j < use.size(); j++) {
    ret << use[j].dump();
  }
  ret << ") ";

  return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Compute 'use' and 'def' sets
///////////////////////////////////////////////////////////////////////////////

// 'use' set: the variables read by an instruction
// 'def' set: the variables modified by an instruction

/**
 * Compute 'use' and 'def' sets for a given instruction
 *
 * Param 'set_use_where' need only be true during liveness analysis.
 *
 * @param set_use_where  if true, regard assignments in conditional 'where'
 *                       instructions as usage.
 *
 * ============================================================================
 * NOTES
 * =====
 *
 * * 'set_use_where' needs to be true for the following case (target language):
 *
 *    LI A5 <- 0                  # assignment
 *    ...
 *    where ZC: LI A6 <- 1
 *    where ZC: A5 <- or(A6, A6)  # Conditional assignment
 *    ...
 *    S[VPM_WRITE] <- shl(A5, 0)  # last use
 *
 *   If the condition is ignored (`set_use_where == false`), the conditional
 *   assignment is regarded as an overwrite of the previous one. The variable
 *   is then considered live from the conditional assignment onward.
 *   This is wrong, the value of the first assignment may be significant due
 *   to the condition. The usage of `A5` runs the risk of being assigned different
 *   registers for the different assignments, which will lead to wrong code execution.
 *
 * * However, always using `set_use_where == true` leads to variables being live
 *   for unnecessarily long. If this is the *only* usage of `A6`:
 *
 *    where ZC: LI A6 <- 1
 *    where ZC: A5 <- or(A6, A6)  # Conditional assignment
 *
 *   ... `A6` would be considered live from the start of the program
 *   onward till the last usage.
 *   This unnecessarily ties up a register for a long duration, complicating the
 *   allocation by creating a false shortage of registers.
 *   This case can not be handled by the liveness analysis as implemented here.
 *   It is corrected afterwards in methode `Liveness::compute()`.
 */
void useDefReg(Instr instr, UseDefReg* useDef, bool set_use_where) {
  auto ALWAYS = AssignCond::Tag::ALWAYS;

  useDef->use.clear();
  useDef->def.clear();

  switch (instr.tag) {
    case LI:                                     // Load immediate
      useDef->def.insert(instr.LI.dest);         // Add destination reg to 'def' set

      if (set_use_where) {
        if (instr.LI.cond.tag != ALWAYS)         // Add destination reg to 'use' set if conditional assigment
          useDef->use.insert(instr.LI.dest);
      }
      return;

    case ALU:                                    // ALU operation
      useDef->def.insert(instr.ALU.dest);        // Add destination reg to 'def' set

      if (set_use_where) {
        if (instr.ALU.cond.tag != ALWAYS)        // Add destination reg to 'use' set if conditional assigment
          useDef->use.insert(instr.ALU.dest);
      }

      if (instr.ALU.srcA.is_reg())               // Add source reg A to 'use' set
        useDef->use.insert(instr.ALU.srcA.reg);

      if (instr.ALU.srcB.is_reg())               // Add source reg B to 'use' set
        useDef->use.insert(instr.ALU.srcB.reg);
      return;

    case RECV:                                   // Load receive instruction
      useDef->def.insert(instr.RECV.dest);       // Add dest reg to 'def' set
      return;
    default:
      return;
  }  
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
    useDef(instrs[i], &useDefSet);

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
    fatal("LiveSets::choose_register(): register allocation failed, insufficient capacity");
  }

  return chosenA;
}  


/**
 * Determine the liveness sets for each instruction.
 */
void Liveness::compute_liveness(Instr::List &instrs) {
  // Initialise live mapping to have one entry per instruction
  setSize(instrs.size());

  // For storing the 'use' and 'def' sets of each instruction
  UseDef useDefSets;

  // For temporarily storing live-in and live-out variables
  LiveSet liveIn;
  LiveSet liveOut;

  // Has a change been made to the liveness mapping?
  bool changed = true;

  // Iterate until no change, i.e. fixed point
  while (changed) {
    changed = false;

    // Propagate live variables backwards
    for (int i = instrs.size()-1; i >= 0; i--) {
      // Compute 'use' and 'def' sets
      Instr instr = instrs[i];
      useDef(instr, &useDefSets, true);

      // Compute live-out variables
      computeLiveOut(i, liveOut);

      // Remove the 'def' set from the live-out set to give live-in set
      liveIn.clear();
      for (int j = 0; j < liveOut.size(); j++) {
        if (!useDefSets.def.member(liveOut[j]))
          liveIn.insert(liveOut[j]);
      }

      // Add the 'use' set to the live-in set
      for (int j = 0; j < useDefSets.use.size(); j++)
        liveIn.insert(useDefSets.use[j]);

      // Insert the live-in variables into the map
      for (int j = 0; j < liveIn.size(); j++) {
        bool inserted = insert(i, liveIn[j]);
        changed = changed || inserted;
      }
    }
  }
}


void Liveness::compute(Instr::List &instrs) {
  reg_usage.set_used(instrs);

  compute_liveness(instrs);
  assert(instrs.size() == size());

  reg_usage.set_live(*this);

  // Adjust first usage in liveness, if necessary 
  for (int i = 0; i < (int) reg_usage.size(); ++i) {
    auto &item = reg_usage[i];

    if (item.unused() || item.only_assigned())     continue;  // skip special cases
    if (item.use.dst_first + 1 == item.live.first) continue;  // all is well

/*
    {
      std::string msg;
      msg << "reg_usage[" << i << "]: discrepancy between first assign and liveness: " << item.dump();
      warning(msg);
    }
*/

    // Remove all liveness of given variable `i` before and including first assign
    assert(item.use.dst_first != -1);
    assert(item.live.first != -1);
    for (int j = item.live.first; j <= item.use.dst_first; ++j) {
      int num = m_set[j].remove_set(i);
      assert(num == 1);
    }
  }

  compile_data.liveness_dump = dump();
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

    for (int k = 0; k < set.size(); k++)
      liveOut.insert(set[k]);
  }
}


void Liveness::setSize(int size) {
  m_set.set_size(size);
}


bool Liveness::insert(int index, RegId item) {
  return m_set[index].insert(item);
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

  ret += "\n";

  return ret;
}


/**
 * Introduce accumulators where possible
 *
 * This is done before the actual liveness analysis.
 * The idea is to minimize beforehand the number of variables considered
 * in the liveness analysis.
 */
void introduceAccum(CFG &cfg, Instr::List &instrs, int numVars) {
  Liveness live(cfg, numVars);
  live.compute(instrs);

  compile_data.num_accs_introduced = introduceAccum(live, instrs, live.alloc());
  //std::cout << count_reg_types(instrs).dump() << std::endl;
}


void allocate_registers(Instr::List &instrs, RegUsage const &alloc) {
  for (int i = 0; i < instrs.size(); i++) {
    allocate_registers(instrs.get(i), alloc);
  }
}

}  // namespace V3DLib
