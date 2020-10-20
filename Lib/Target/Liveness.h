//
// Liveness analysis
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _QPULIB_LIVENESS_H_
#define _QPULIB_LIVENESS_H_
#include <string>
#include <vector>
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
	Liveness(CFG &cfg) : m_cfg(cfg) {}

	void compute(Seq<Instr> &instrs);
	void computeLiveOut(InstrId i, LiveSet &liveOut);

	void setSize(int size);
	int size() const { return m_set.size(); }
	bool insert(int index, RegId item);
	LiveSet &operator[](int index) { return get(index); }

private:
	CFG &m_cfg;
	Seq<LiveSet> m_set;

	std::string dump();
	LiveSet &get(int index) { return m_set[index]; }
};


class LiveSets {
public:
  UseDef useDefSet;

	LiveSets(int size) :m_size(size) {
  	m_sets = new LiveSet [size];
	}

	~LiveSets() {
		delete [] m_sets;
	}

	void init(Seq<Instr> &instrs, Liveness &live);
	LiveSet &operator[](int index) { return m_sets[index]; }
	std::vector<bool> possible_registers(int index, std::vector<Reg> &alloc, RegTag reg_tag = REG_A);

	static RegId choose_register(std::vector<bool> &possible, bool check_limit = true);	
	static void  dump_possible(std::vector<bool> &possible, int index = -1);

private:
	int m_size = 0;
	LiveSet *m_sets = nullptr;
};

}  // namespace QPULib

#endif  // _QPULIB_LIVENESS_H_
