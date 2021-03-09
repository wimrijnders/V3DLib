// Liveness analysis
//
// This follows the following source pretty closely:
//
//    https://lambda.uta.edu/cse5317/spring01/notes/node37.html
//
///////////////////////////////////////////////////////////////////////////////
#include "Support/basics.h"    // fatal()
#include "Support/Platform.h"  // size_regfile()
#include "Target/Subst.h"
#include "Target/Liveness.h"
#include "Common/CompileData.h"

namespace V3DLib {

bool RegUsageItem::unused() const {
  bool ret = (dst_use == 0 && src_use == 0);

  if (ret) {
    assert(reg.tag == NONE);
  }

  return ret;
}


std::string RegUsageItem::dump() const {
  std::string ret;

  ret << reg.dump() << "; dst count: " << dst_use << "; src count: " << src_use;

  return ret;
}

RegUsage::RegUsage(int numVars) : Parent(numVars) {
  for (int i = 0; i < numVars; i++) (*this)[i].reg.tag = NONE;
}


void RegUsage::set_used(Instr::List &instrs) {
  for (int i = 0; i < instrs.size(); i++) {
    UseDef out;
    useDef(instrs[i], &out);

    for (int j = 0; j < out.def.size(); j++) {
      (*this)[out.def[j]].dst_use++;
    }
    for (int j = 0; j < out.use.size(); j++) {
      (*this)[out.use[j]].src_use++;
    }
  }
}


std::string RegUsage::allocated_registers_dump() const {
  std::string ret;

  for (int i = 0; i < (int) size(); i++) {
    ret << i << ": " << (*this)[i].reg.dump() << "\n";
  }

  return ret;
}


std::string RegUsage::dump() const {
  std::string ret;

  for (int i = 0; i < (int) size(); i++) {
    ret << i << ": " << (*this)[i].dump() << "\n";
  }

  std::string unused;

  for (int i = 0; i < (int) size(); i++) {
    if ((*this)[i].unused()) {
      unused << i << ",";
    }
  }

  if (!unused.empty()) {
    ret << "\nNot used: " << unused;
  }

  return ret;
}

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


/**
 * Optimisation pass that introduces accumulators
 *
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
 * @return Number of substitustions performed;
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

    bool do_it = (prev.is_always() && useDefCurrent.use.member(def) && !liveOut.member(def));
    if (!do_it) continue;

    int acc_id =1;

    if (!Platform::compiling_for_vc4()) {
      // v3d ROT uses ACC1 (r1) internally, don't use it here
      // TODO better selection of subsitution ACC
      if (prev.isRot() || instr.isRot()) {
        //warning("introduceAccum(): subsituting ACC in ROT operation");
        acc_id = 2;
      }
    }

    Reg current(REG_A, def);
    Reg replace_with(ACC, acc_id);

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


///////////////////////////////////////////////////////////////////////////////
// Compute 'use' and 'def' sets
///////////////////////////////////////////////////////////////////////////////

// 'use' set: the variables read by an instruction
// 'def' set: the variables modified by an instruction

/**
 * Compute 'use' and 'def' sets for a given instruction
 */
void useDefReg(Instr instr, UseDefReg* useDef) {
  auto ALWAYS = AssignCond::Tag::ALWAYS;

  useDef->use.clear();
  useDef->def.clear();

  switch (instr.tag) {
    case LI:                                     // Load immediate
      useDef->def.insert(instr.LI.dest);         // Add destination reg to 'def' set

      if (instr.LI.cond.tag != ALWAYS)           // Add destination reg to 'use' set if conditional assigment
        useDef->use.insert(instr.LI.dest);
      return;

    case ALU:  // ALU operation
      useDef->def.insert(instr.ALU.dest);        // Add destination reg to 'def' set

      if (instr.ALU.cond.tag != ALWAYS)          // Add destination reg to 'use' set if conditional assigment
        useDef->use.insert(instr.ALU.dest);

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


/**
 * Same as `useDefReg()`, except only yields ids of registers in register file A.
 */
void useDef(Instr const &instr, UseDef* out) {
  UseDefReg set;
  useDefReg(instr, &set);
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


/*
// Compute the union of the 'use' sets of the successors of a given
// instruction.

void useSetOfSuccs(Instr::List* instrs, CFG* cfg, InstrId i, SmallSeq<RegId>* use) {
  use->clear();
  Succs* s = &cfg->elems[i];
  for (int j = 0; j < s->size(); j++) {
    UseDef set;
    useDef(instrs->elems[s->elems[j]], &set);
    for (int k = 0; k < set.use.size(); k++)
      use->insert(set.use[k]);
  }
}
*/


namespace {

/**
 * Determine the liveness sets for each instruction.
 */
void liveness(Instr::List &instrs, Liveness &live) {
  // Initialise live mapping to have one entry per instruction
  live.setSize(instrs.size());

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
      useDef(instr, &useDefSets);

      // Compute live-out variables
      live.computeLiveOut(i, liveOut);

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
        bool inserted = live.insert(i, liveIn[j]);
        changed = changed || inserted;
      }
    }
  }
}


}  // anon namespace


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


void Liveness::compute(Instr::List &instrs) {
  liveness(instrs, *this);

  //std::cout << live.dump() << std::endl;
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

}  // namespace V3DLib
