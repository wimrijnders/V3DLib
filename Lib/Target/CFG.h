//
// Control-flow graphs (CFGs)
//
// ============================================================================
#ifndef _V3DLIB_CFG_H_
#define _V3DLIB_CFG_H_
#include "Common/Set.h"
#include "Target/instr/Instr.h"

namespace V3DLib {

typedef int InstrId;                           // Index of instruction in instruction list
using Succs = SmallSet<InstrId>;               // Set of successors.


/**
 * Control Flow Graph
 *
 * Set of successors for each instruction.
 */
class CFG : public Set<Succs> {
public:
  void build(Instr::List &instrs);
  int  block_at(InstrId line_num) const;
  int  block_end(InstrId line_num) const;
  bool is_parent_block(InstrId line_num, int block) const;

  std::string dump() const;
  std::string dump_blocks() const;

private:
  std::vector<int> blocks;

  bool is_regular(InstrId i) const;
  void build_blocks();
};


}  // namespace V3DLib

#endif  // _V3DLIB_CFG_H_
