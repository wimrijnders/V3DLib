#ifndef _V3DLIB_TARGET_INSTR_INSTR_H_
#define _V3DLIB_TARGET_INSTR_INSTR_H_
#include "Support/InstructionComment.h"
#include "Common/Seq.h"
#include "Reg.h"
#include "Label.h"
#include "Conditions.h"
#include "ALUOp.h"

namespace V3DLib {

class CmpOp;

// ============================================================================
// Immediates
// ============================================================================

// Different kinds of immediate
enum ImmTag {
    IMM_INT32    // 32-bit word
  , IMM_FLOAT32  // 32-bit float
  , IMM_MASK     // 1 bit per vector element (0 to 0xffff)
};

struct Imm {
  ImmTag tag;

  union {
    int intVal;
    float floatVal;
  };
};

// Different kinds of small immediates
enum SmallImmTag {
    SMALL_IMM  // Small immediate
  , ROT_ACC    // Rotation amount taken from accumulator 5
  , ROT_IMM    // Rotation amount 1..15
};

struct SmallImm {
  // What kind of small immediate is it?
  SmallImmTag tag;
  
  // Immediate value
  int val;

  bool operator==(SmallImm const &rhs) const {
    return tag == rhs.tag && val == rhs.val;
  }

  bool operator!=(SmallImm const &rhs) const {
    return !(*this == rhs);
  }
};

// A register or a small immediate operand?
enum RegOrImmTag { REG, IMM };

struct RegOrImm {
  // Register id or small immediate?
  RegOrImmTag tag;

  union {
    // A register
    Reg reg;
    
    // A small immediate
    SmallImm smallImm;
  };

  bool operator==(RegOrImm const &rhs) const {
    if (tag != rhs.tag) return false;

    if (tag == REG) {
      return reg == rhs.reg;
    } else {
      return smallImm == rhs.smallImm;
    }
  }

  bool operator!=(RegOrImm const &rhs) const {
    return !(*this == rhs);
  }
};


// ============================================================================
// Class BranchTarget
// ============================================================================

struct BranchTarget {
  bool relative;      // Branch is absolute or relative to PC+4

  bool useRegOffset;  // Plus value from register file A (optional)
  RegId regOffset;

  int immOffset;      // Plus 32-bit immediate value

  std::string to_string() const;
};


// ============================================================================
// QPU instruction tags
// ============================================================================

enum InstrTag {
  LI,             // Load immediate
  ALU,            // ALU operation
  BR,             // Conditional branch to target
  END,            // Program end (halt)

  // ==================================================
  // Intermediate-language constructs
  // ==================================================

  BRL,            // Conditional branch to label
  LAB,            // Label
  NO_OP,          // No-op

  VC4_ONLY,

  DMA_LOAD_WAIT = VC4_ONLY, // Wait for DMA load to complete
  DMA_STORE_WAIT, // Wait for DMA store to complete
  SINC,           // Increment semaphore
  SDEC,           // Decrement semaphore
  IRQ,            // Send IRQ to host

  // Print instructions
  PRS,            // Print string
  PRI,            // Print integer
  PRF,            // Print float

  VPM_STALL,      // Marker for VPM read setup

  END_VC4_ONLY,

  // Load receive via TMU
  RECV = END_VC4_ONLY,
  TMU0_TO_ACC4,

  // Init program block (Currently filled only for v3d)
  INIT_BEGIN,     // Marker for start of init block
  INIT_END,       // Marker for end of init block

  // ==================================================
  // v3d-only instructions
  // ==================================================
  V3D_ONLY,

  TMUWT = V3D_ONLY,
  // TODO Add as required here

  END_V3D_ONLY
};


void check_instruction_tag_for_platform(InstrTag tag, bool for_vc4);


// ============================================================================
// Class Instr - QPU instructions
// ============================================================================

struct Instr : public InstructionComment {

  class List : public Seq<Instr> {
    using Parent = Seq<Instr>;
  public:
    List() = default;
    List(int size) : Parent(size) {}

    std::string dump() const;
    std::string mnemonics(bool with_comments = false) const;
    int lastUniformOffset();
    int tag_index(InstrTag tag, bool ensure_one = true);
    int tag_count(InstrTag tag);
  };

  InstrTag tag;

  union {
    // Load immediate
    struct {
      SetCond    m_setCond;
      AssignCond cond;
      Reg        dest;
      Imm        imm;
    } LI;

    // ALU operation
    struct {
      SetCond    m_setCond;
      AssignCond cond;
      Reg        dest;
      RegOrImm   srcA;
      ALUOp      op;
      RegOrImm   srcB;
    } ALU;

    // Conditional branch (to target)
    struct {
      BranchCond cond;
      BranchTarget target;
    } BR;

    // ==================================================
    // Intermediate-language constructs
    // ==================================================

    // Conditional branch (to label)
    struct {
      BranchCond cond;
      Label label;
    } BRL;

    // Labels, denoting branch targets
    Label m_label;  // Renamed during debugging
                    // TODO perhaps revert

    // Semaphores
    int semaId;                 // Semaphore id (range 0..15)

    // Load receive via TMU
    struct { Reg dest; } RECV;  // Destination register for load receive

    // Print instructions
    const char* PRS;            // Print string
    Reg PRI;                    // Print integer
    Reg PRF;                    // Print float
  };


  Instr() : tag(NO_OP) {} 
  Instr(InstrTag in_tag);

  std::string header()  const  { return InstructionComment::header();  }  // grumbl hating that this is needed
  std::string comment() const  { return InstructionComment::comment(); }  // idem
  Instr &header(std::string const &msg) { InstructionComment::header(msg);  return *this; }
  Instr &comment(std::string msg)       { InstructionComment::comment(msg); return *this; }

  void break_point() { m_break_point = true; }
  bool break_point() const { return m_break_point; }

  // ==================================================
  // Helper methods
  // ==================================================
  Instr &setCondFlag(Flag flag);
  Instr &setCondOp(CmpOp const &cmp_op);
  Instr &cond(AssignCond in_cond);
  bool isCondAssign() const;
  bool hasImm() const { return ALU.srcA.tag == IMM || ALU.srcB.tag == IMM; }
  bool isUniformLoad() const;
  bool isUniformPtrLoad() const;
  bool isTMUAWrite(bool fetch_only = false) const;
  bool isZero() const;
  bool isLast() const;

  SetCond const &setCond() const;
  std::string mnemonic(bool with_comments = false, std::string const &pref = "") const;
  std::string dump() const;

  bool operator==(Instr const &rhs) const {
    // Cheat by comparing the string representation,
    // to avoid having to check the union members separately, and to skip unused fields
    return this->mnemonic() == rhs.mnemonic();
  }


  static Instr nop();

  /////////////////////////////////////
  // Label support
  /////////////////////////////////////

  bool is_label() const { return tag == InstrTag::LAB; }
  bool is_branch_label() const { return tag == InstrTag::BRL; }

  Label branch_label() const {
    assert(tag == InstrTag::BRL);
    return BRL.label;
  }

  void label_to_target(int offset);
    
  void label(Label val) {
    assert(tag == InstrTag::LAB);
    m_label = val;
  }

  Label label() const {
    assert(tag == InstrTag::LAB);
    return m_label;
  }

  // ==================================================
  // v3d-specific  methods
  // ==================================================
  Instr &pushz();

  Instr &allzc() {
    assert(tag == InstrTag::BRL);
    BRL.cond.tag  = COND_ALL;
    BRL.cond.flag = Flag::ZC;
    return *this;
  }

private:
  SetCond &setCond();

  bool m_break_point = false;
};


void check_zeroes(Seq<Instr> const &instrs);

}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_INSTR_INSTRUCTIONS_H_