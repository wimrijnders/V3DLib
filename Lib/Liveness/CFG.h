#ifndef _V3DLIB_LIVENESS_CFG_H_
#define _V3DLIB_LIVENESS_CFG_H_
#include "Common/Set.h"
#include "Target/instr/Instr.h"
#include "Range.h"

namespace V3DLib {

typedef int InstrId;                           // Index of instruction in instruction list
using Succs = SmallSet<InstrId>;               // Set of successors.


/**
 * Control Flow Graph (CFG)
 *
 * Set of successors for each instruction.
 */
class CFG : public Set<Succs> {
  using Parent = Set<Succs>;

public:
  void build(Instr::List &instrs);
  int  block_at(InstrId line_num) const;
  int  block_end(InstrId line_num) const { return blocks.end(line_num);}
  bool is_parent_block(InstrId line_num, int block) const;
  void clear();

  std::string dump() const;

private:

  /**
   * Blocks are numbered uniquely and consecutively as encountered.
   *
   * An child (embedded) block always has a higher number than its parent block.
   * However, a consecutive block can also be numbered lower.
   */
  struct Blocks {
    void build(CFG const &cfg);
    void clear();
    std::string dump() const;
    int end(InstrId line_num) const;
    bool block_in(int cur_block, int parent_block) const;

    std::vector<int>   list;
    std::vector<Range> ranges;

  private:
    void add_ranges();
    int max_block_num() const;
  } blocks;

  bool is_regular(InstrId i) const;
};


}  // namespace V3DLib

#endif  // _V3DLIB_LIVENESS_CFG_H_
