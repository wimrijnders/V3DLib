//
// Liveness analysis
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _V3DLIB_LIVENESS_H_
#define _V3DLIB_LIVENESS_H_
#include <string>
#include <vector>
#include "CFG.h"

namespace V3DLib {

struct RegUsageItem {
 Reg reg;

 struct {
   int dst_use = 0;
   int src_use = 0;
   int dst_first = -1;
   int src_first = -1;
 } use;

 struct {
   int first = -1;
   int last = -1;
   int count = 0;
 } live;

 void add_live(int n);
 bool unused() const;
 bool only_assigned() const { return use.dst_use != 0 && use.src_use == 0; }
 std::string dump() const;
};

class Liveness;

struct RegUsage : public std::vector<RegUsageItem> {
  using Parent = std::vector<RegUsageItem>;

  RegUsage(int numVars);

  void set_used(Instr::List &instrs);
  void set_live(Liveness &live);
  std::string allocated_registers_dump() const;
  std::string dump() const;
};


// 'use' and 'def' sets:
//   * 'use' set: the variables read by an instruction
//   * 'def' set: the variables modified by an instruction

struct UseDefReg {
  SmallSeq<Reg> use;
  SmallSeq<Reg> def;

  std::string dump() const;
};   
     
struct UseDef {
  SmallSeq<RegId> use;
  SmallSeq<RegId> def;
};   

// Compute 'use' and 'def' sets for a given instruction

void useDefReg(Instr instr, UseDefReg* out);
void useDef(Instr const &instr, UseDef* out);

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

  void compute(Instr::List &instrs);
  void computeLiveOut(InstrId i, LiveSet &liveOut);

  void setSize(int size);
  int size() const { return m_set.size(); }
  bool insert(int index, RegId item);
  LiveSet &operator[](int index) { return get(index); }
  std::string dump();

private:
  CFG &m_cfg;
  Seq<LiveSet> m_set;

  LiveSet &get(int index) { return m_set[index]; }
};


class LiveSets {
public:
  UseDef useDefSet;

  LiveSets(int size);
  ~LiveSets();

  void init(Instr::List &instrs, Liveness &live);
  LiveSet &operator[](int index);
  std::vector<bool> possible_registers(int index, RegUsage &alloc, RegTag reg_tag = REG_A);

  static RegId choose_register(std::vector<bool> &possible, bool check_limit = true);  
  static void  dump_possible(std::vector<bool> &possible, int index = -1);

private:
  int m_size = 0;
  LiveSet *m_sets = nullptr;
};


int introduceAccum(Liveness &live, Instr::List &instrs, RegUsage &allocated_vars);

}  // namespace V3DLib

#endif  // _V3DLIB_LIVENESS_H_
