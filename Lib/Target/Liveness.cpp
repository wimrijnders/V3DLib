// Liveness analysis
//
// This follows the following source pretty closely:
//
//    https://lambda.uta.edu/cse5317/spring01/notes/node37.html
//
///////////////////////////////////////////////////////////////////////////////
#include "Support/basics.h"   // fatal()
#include "Target/Subst.h"
#include "Target/Liveness.h"

namespace QPULib {

namespace {

// ============================================================================
// Accumulator allocation
// ============================================================================

// This is a simple peephole optimisation, captured by the following
// rewrite rule:
//
//   i:  x <- f(...)
//   j:  g(..., x, ...)
// 
// ===> if x not live-out of j
// 
//   i:  acc <- f(...)
//   j:  g(..., acc, ...)

void introduceAccum(Liveness &live, Seq<Instr>* instrs)
{
  UseDef useDefPrev, useDefCurrent;
  LiveSet liveOut;

  Reg acc;
  acc.tag = ACC;
  acc.regId = 1;

  for (int i = 1; i < instrs->numElems; i++) {
    Instr prev  = instrs->elems[i-1];
    Instr instr = instrs->elems[i];

    // Compute vars defined by prev
    useDef(prev, &useDefPrev);

    if (useDefPrev.def.numElems > 0) {
      RegId def = useDefPrev.def.elems[0];

      // Compute vars used by instr
      useDef(instr, &useDefCurrent);

      // Compute vars live-out of instr
      live.computeLiveOut(i, &liveOut);

      // Check that write is non-conditional
			auto ALWAYS = AssignCond::Tag::ALWAYS;
      bool always = (prev.tag == LI && prev.LI.cond.tag == ALWAYS)
                 || (prev.tag == ALU && prev.ALU.cond.tag == ALWAYS);

      if (always && useDefCurrent.use.member(def) && !liveOut.member(def)) {
        renameDest(&prev, REG_A, def, ACC, 1);
        renameUses(&instr, REG_A, def, ACC, 1);
        instrs->elems[i-1] = prev;
        instrs->elems[i]   = instr;
      }
    }
  }
}

}  // anon namespace

// ============================================================================
// Compute 'use' and 'def' sets
// ============================================================================

// 'use' set: the variables read by an instruction
// 'def' set: the variables modified by an instruction

/**
 * Compute 'use' and 'def' sets for a given instruction
 */
void useDefReg(Instr instr, UseDefReg* useDef) {
	auto ALWAYS = AssignCond::Tag::ALWAYS;

  // Make the 'use' and 'def' sets empty
  useDef->use.clear();
  useDef->def.clear();

  switch (instr.tag) {
    // Load immediate
    case LI:
      // Add destination reg to 'def' set
      useDef->def.insert(instr.LI.dest);

      // Add destination reg to 'use' set if conditional assigment
      if (instr.LI.cond.tag != ALWAYS)
        useDef->use.insert(instr.LI.dest);
      return;

    // ALU operation
    case ALU:
      // Add destination reg to 'def' set
      useDef->def.insert(instr.ALU.dest);

      // Add destination reg to 'use' set if conditional assigment
      if (instr.ALU.cond.tag != ALWAYS)
        useDef->use.insert(instr.ALU.dest);

      // Add source reg A to 'use' set
      if (instr.ALU.srcA.tag == REG)
        useDef->use.insert(instr.ALU.srcA.reg);

      // Add source reg B to 'use' set
      if (instr.ALU.srcB.tag == REG)
        useDef->use.insert(instr.ALU.srcB.reg);
      return;

    // Print integer instruction
    case PRI:
      // Add source reg to 'use' set
      useDef->use.insert(instr.PRI);
      return;

    // Print float instruction
    case PRF:
      // Add source reg to 'use' set
      useDef->use.insert(instr.PRF);
      return;

    // Load receive instruction
    case RECV:
      // Add dest reg to 'def' set
      useDef->def.insert(instr.RECV.dest);
      return;
  }
}


/**
 * Same as `useDefReg()`, except only yields ids of registers in register file A.
 */
void useDef(Instr instr, UseDef* out) {
  UseDefReg set;
  useDefReg(instr, &set);
  out->use.clear();
  out->def.clear();
  for (int i = 0; i < set.use.numElems; i++) {
    Reg r = set.use.elems[i];
    if (r.tag == REG_A) out->use.append(r.regId);
  }
  for (int i = 0; i < set.def.numElems; i++) {
    Reg r = set.def.elems[i];
    if (r.tag == REG_A) out->def.append(r.regId);
  }
}

// Compute the union of the 'use' sets of the successors of a given
// instruction.

void useSetOfSuccs(Seq<Instr>* instrs, CFG* cfg, InstrId i, SmallSeq<RegId>* use) {
  use->clear();
  Succs* s = &cfg->elems[i];
  for (int j = 0; j < s->numElems; j++) {
    UseDef set;
    useDef(instrs->elems[s->elems[j]], &set);
    for (int k = 0; k < set.use.numElems; k++)
      use->insert(set.use.elems[k]);
  }
}

// Return true if given instruction has two register operands.

bool getTwoUses(Instr instr, Reg* r1, Reg* r2)
{
  if (instr.tag == ALU && instr.ALU.srcA.tag == REG
                       && instr.ALU.srcB.tag == REG) {
    *r1 = instr.ALU.srcA.reg;
    *r2 = instr.ALU.srcB.reg;
    return true;
  }
  return false;
}


namespace {

/**
 * Determine the liveness sets for each instruction.
 */
void liveness(Seq<Instr>* instrs, Liveness &live) {
  // Initialise live mapping to have one entry per instruction
	live.setSize(instrs->numElems);

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
    for (int i = instrs->numElems-1; i >= 0; i--) {
      // Compute 'use' and 'def' sets
      Instr instr = instrs->elems[i];
      useDef(instr, &useDefSets);

      // Compute live-out variables
      live.computeLiveOut(i, &liveOut);

      // Remove the 'def' set from the live-out set to give live-in set
      liveIn.clear();
      for (int j = 0; j < liveOut.size(); j++) {
        //if (! useDefSets.def.member(liveOut[j]))
        //  liveIn.insert(liveOut[j]);
        if (! useDefSets.def.member(liveOut.elems[j]))
          liveIn.insert(liveOut.elems[j]);
      }

      // Add the 'use' set to the live-in set
      for (int j = 0; j < useDefSets.use.numElems; j++)
        liveIn.insert(useDefSets.use.elems[j]);

      // Insert the live-in variables into the map
      for (int j = 0; j < liveIn.size(); j++) {
        bool inserted = live.insert(i, liveIn.elems[j]);
        changed = changed || inserted;
      }
    }
  }
}


}  // anon namespace

void LiveSets::init(Seq<Instr>* instrs, Liveness &live) {
  LiveSet liveOut;

  for (int i = 0; i < instrs->numElems; i++) {
    live.computeLiveOut(i, &liveOut);
    useDef(instrs->elems[i], &useDefSet);

    for (int j = 0; j < liveOut.size(); j++) {
      RegId rx = liveOut.elems[j];

      for (int k = 0; k < liveOut.size(); k++) {
        RegId ry = liveOut.elems[k];
        if (rx != ry) m_sets[rx].insert(ry);
      }

      for (int k = 0; k < useDefSet.def.numElems; k++) {
        RegId rd = useDefSet.def.elems[k];
        if (rd != rx) {
          m_sets[rx].insert(rd);
          m_sets[rd].insert(rx);
        }
      }
    }
  }
}


/**
 * Determine the available register in the register file, to use for variable 'index'.
 *
 * @param index  index of variable
 */
std::vector<bool> LiveSets::possible_registers(int index, std::vector<Reg> &alloc, RegTag reg_tag) {
  const int NUM_REGS = 32;
  std::vector<bool> possible(NUM_REGS);

	for (int j = 0; j < NUM_REGS; j++)
		possible[j] = true;

    LiveSet &set = m_sets[index];

    // Eliminate impossible choices of register for this variable
    for (int j = 0; j < set.size(); j++) {
      Reg neighbour = alloc[set[j]];
      if (neighbour.tag == reg_tag) possible[neighbour.regId] = false;
    }

	return possible;
}


/**
 * Debug function to output the contents of the possible-vector
 */
void LiveSets::dump_possible(std::vector<bool> &possible, int index) {
	std::string buf = "possible: ";

	if (index >=0) {
		if (index < 10) buf << "  ";
		else if (index < 100) buf << " ";

		buf << index;
	}
	buf << ": ";

	for (int j = possible.size() - 1; j >= 0; j--) {
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

	for (int j = 0; j < possible.size(); j++)
		if (possible[j]) { chosenA = j; break; }

	if (check_limit && chosenA < 0) {
		fatal("LiveSets::choose_register(): register allocation failed, insufficient capacity");
	}

	return chosenA;
}	


void Liveness::compute(Seq<Instr>* instrs) {
	liveness(instrs, *this);
	//printf("%s", dump().c_str());

  // Optimisation pass that introduces accumulators
  introduceAccum(*this, instrs);
}


/**
 * Compute live sets for each instruction
 *
 * Compute the live-out variables of an instruction, given the live-in
 * variables of all instructions and the CFG.
 */
void Liveness::computeLiveOut(InstrId i, LiveSet* liveOut) {
  liveOut->clear();
  Succs* s = &m_cfg.elems[i];

  for (int j = 0; j < s->numElems; j++) {
    LiveSet &set = get(s->elems[j]);

    for (int k = 0; k < set.size(); k++)
      //liveOut->insert(set[k]);
      liveOut->insert(set.elems[k]);
  }
}


void Liveness::setSize(int size) {
  m_set.setCapacity(size);
  m_set.numElems = size;
}


bool Liveness::insert(int index, RegId item) {
	return m_set.elems[index].insert(item);
}


std::string Liveness::dump() {
	std::string ret;

	ret += "Liveness dump:\n";

	for (int i = 0; i < m_set.size(); ++i) {
		ret += std::to_string(i) + ": ";

		auto &item = m_set.elems[i];
		bool did_first = false;
		for (int j = 0; j < item.size(); j++) {
			if (did_first) {
				ret += ", ";
			} else {
				did_first = true;
			}
			//ret += std::to_string(item[i]);
			ret += std::to_string(item.elems[j]);
		}
		ret += "\n";
	}

	ret += "\n";

	return ret;
}

}  // namespace QPULib
