// Liveness analysis
//
// This follows the following source pretty closely:
//
//    https://lambda.uta.edu/cse5317/spring01/notes/node37.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Target/Liveness.h"

namespace QPULib {

// ============================================================================
// Compute 'use' and 'def' sets
// ============================================================================

// 'use' set: the variables read by an instruction
// 'def' set: the variables modified by an instruction

// Compute 'use' and 'def' sets for a given instruction

void useDefReg(Instr instr, UseDefReg* useDef)
{
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

void useSetOfSuccs(Seq<Instr>* instrs, CFG* cfg,
                   InstrId i, SmallSeq<RegId>* use)
{
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
void liveness(Seq<Instr>* instrs, CFG* cfg, Liveness &live) {
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
      live.computeLiveOut(cfg, i, &liveOut);

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


void Liveness::compute(Seq<Instr>* instrs, CFG* cfg) {
	liveness(instrs, cfg, *this);
	printf("%s", dump().c_str());
}


/**
 * Compute live sets for each instruction
 *
 * Compute the live-out variables of an instruction, given the live-in
 * variables of all instructions and the CFG.
 */
void Liveness::computeLiveOut(CFG* cfg, InstrId i, LiveSet* liveOut) {
  liveOut->clear();
  Succs* s = &cfg->elems[i];

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
