//
// Liveness analysis
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _QPULIB_LIVENESS_H_
#define _QPULIB_LIVENESS_H_
#include <string>
#include "Common/Seq.h"
#include "Target/Syntax.h"
#include "Target/CFG.h"

namespace QPULib {

// 'use' and 'def' sets:
//   * 'use' set: the variables read by an instruction
//   * 'def' set: the variables modified by an instruction

struct UseDefReg {
  SmallSeq<Reg> use;
  SmallSeq<Reg> def;
};   
     
struct UseDef {
  SmallSeq<RegId> use;
  SmallSeq<RegId> def;
};   

// Compute 'use' and 'def' sets for a given instruction

void useDefReg(Instr instr, UseDefReg* out);
void useDef(Instr instr, UseDef* out);
bool getTwoUses(Instr instr, Reg* r1, Reg* r2);

// A live set containts the variables
// that are live-in to an instruction.
using LiveSet = SmallSeq<RegId>;


/**
 * The result of liveness analysis is a set
 * of live variables for each instruction.
 */
class Liveness {
public:
	void compute(Seq<Instr>* instrs, CFG* cfg);
	void computeLiveOut(CFG* cfg, InstrId i, LiveSet* liveOut);

	void setSize(int size);
	bool insert(int index, RegId item);
	LiveSet &operator[](int index) { return get(index); }

private:
	Seq<LiveSet> m_set;

	std::string dump();
	LiveSet &get(int index) { return m_set.elems[index]; }
};



}  // namespace QPULib

#endif  // _QPULIB_LIVENESS_H_
