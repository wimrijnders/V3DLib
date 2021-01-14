#ifndef _V3DLIB_TARGET_SYNTAX_H_
#define _V3DLIB_TARGET_SYNTAX_H_
#include <stdint.h>
#include <string>
#include "Common/Seq.h"  // for check_zeroes()
#include "Support/InstructionComment.h"
#include "Support/debug.h"
#include "Source/Var.h"
#include "Reg.h"

namespace V3DLib {

// Syntax of the QPU target language.

// This abstract syntax is a balance between a strict and relaxed
// definition of the target language:
// 
//   a "strict" definition would allow only instructions that can run on
//   the target machine to be expressed, whereas a "relaxed" one allows
//   instructions that have no direct mapping to machine instructions.
// 
// A relaxed definition allows the compilation process to be incremental:
// after each pass, the target code gets closer to being executable, by
// transforming away constructs that do not have a direct mapping to
// hardware.  However, we do not want to be too relaxed, otherwise we
// loose scope for the type checker to help us.
// 
// For example, the definition below allows an instruction to read two
// operands from the *same* register file.  In fact, two operands must be
// taken from different register files in the target language.  It is the
// job of a compiler pass to enforce such a constraint.

// ============================================================================
// Sub-word selectors
// ============================================================================

// A sub-word selector allows a 32, 16, or 8-bit portion of each vector
// word to be selected.

enum SubWord {
    A8     // Bits 7..0
  , B8     // Bits 15..8
  , C8     // Bits 23..16
  , D8     // Bits 31..24
  , A16    // Bits 15..0
  , B16    // Bits 31..16
  , A32    // Bits 31..0
};

// ============================================================================
// Conditions
// ============================================================================

enum Flag {
    ZS              // Zero set
  , ZC              // Zero clear
  , NS              // Negative set
  , NC              // Negative clear
};

// Branch conditions

enum BranchCondTag {
    COND_ALL         // Reduce vector of bits to a single
  , COND_ANY         // bit using AND/OR reduction
  , COND_ALWAYS
  , COND_NEVER
};


struct BranchCond {
  BranchCondTag tag;  // ALL or ANY reduction?
  Flag flag;          // Condition flag

	BranchCond negate() const;
	std::string to_string() const;
};

struct CmpOp;  // Forward declaration

// v3d only
struct SetCond {
	enum Tag {
		NO_COND,
		Z,
		N,
		C
	};

	SetCond() : m_tag(NO_COND) {}

	bool flags_set() const { return m_tag != NO_COND; }
	void tag(Tag tag) { m_tag = tag; }
	Tag tag() const { return m_tag; }
	void clear() { tag(NO_COND); }
	std::string pretty() const;
	void setFlag(Flag flag);
	void setOp(CmpOp const &cmp_op);

private:
	Tag m_tag = NO_COND;

	SetCond(CmpOp const &cmp_op);
	const char *to_string() const;
};



/**
 * Assignment conditions
 */
struct AssignCond {

	enum Tag {
		NEVER,
		ALWAYS,
		FLAG
	};

  Tag tag;    // Kind of assignment condition
  Flag flag;  // Condition flag

	AssignCond() = default;
	AssignCond(CmpOp const &cmp_op);
	AssignCond(Tag in_tag) : tag(in_tag) {}

	bool is_always() const { return tag == ALWAYS; }
	bool is_never()  const { return tag == NEVER; }
	AssignCond negate() const;

	std::string to_string() const;
	BranchCond to_branch_cond(bool do_all) const;
};

extern AssignCond always;  // Is a global to reduce eyestrain in gdb
extern AssignCond never;   // idem

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
// ALU operations
// ============================================================================

// Add operators
enum ALUOp {
	NOP,            // No op

  // Opcodes for the 'add' ALU
	A_FADD,         // Floating-point add
	A_FSUB,         // Floating-point subtract
	A_FMIN,         // Floating-point min
	A_FMAX,         // Floating-point max
	A_FMINABS,      // Floating-point min of absolute values
	A_FMAXABS,      // Floating-point max of absolute values
	A_FtoI,         // Float to signed integer
	A_ItoF,         // Signed integer to float
	A_ADD,          // Integer add
	A_SUB,          // Integer subtract
	A_SHR,          // Integer shift right
	A_ASR,          // Integer arithmetic shift right
	A_ROR,          // Integer rotate right
	A_SHL,          // Integer shift left
	A_MIN,          // Integer min
	A_MAX,          // Integer max
	A_BAND,         // Bitwise and
	A_BOR,          // Bitwise or
	A_BXOR,         // Bitwise xor
	A_BNOT,         // Bitwise not
	A_CLZ,          // Count leading zeros
	A_V8ADDS,       // Add with saturation per 8-bit element
	A_V8SUBS,       // Subtract with saturation per 8-bit element

  // Opcodes for the 'mul' ALU
	M_FMUL,        // Floating-point multiply
	M_MUL24,       // 24-bit integer multiply
	M_V8MUL,       // Multiply per 8-bit element
	M_V8MIN,       // Min per 8-bit element
	M_V8MAX,       // Max per 8-bit element
	M_V8ADDS,      // Add with saturation per 8-bit element
	M_V8SUBS,      // Subtract with saturation per 8-bit element
	M_ROTATE,      // Rotation (intermediate op-code)

	// v3d only
	A_TIDX,
	A_EIDX
};

// ============================================================================
// Branch targets
// ============================================================================

struct BranchTarget {
  bool relative;      // Branch is absolute or relative to PC+4

  bool useRegOffset;  // Plus value from register file A (optional)
  RegId regOffset;

  int immOffset;      // Plus 32-bit immediate value

	std::string to_string() const;
};


// We allow labels for branching, represented by integer identifiers.  These
// will be translated to actual branch targets in a linking phase.

typedef int Label;



// ======================
// Fresh label generation
// ======================

// Obtain a fresh label
Label freshLabel();

// Number of fresh labels used
int getFreshLabelCount();

// Reset fresh label generator
void resetFreshLabelGen();
void resetFreshLabelGen(int val);

// ============================================================================
// Instructions
// ============================================================================

// QPU instruction tags
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

// QPU instructions
struct Instr : public InstructionComment {
  // What kind of instruction is it?
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


	// ==================================================
	// Helper methods
	// ==================================================
	Instr &setCondFlag(Flag flag);
	Instr &setCondOp(CmpOp const &cmp_op);
	Instr &cond(AssignCond in_cond);
	bool isCondAssign() const;
	bool hasImm() const { return ALU.srcA.tag == IMM || ALU.srcB.tag == IMM; }
	bool isRot() const { return ALU.op == M_ROTATE; }
	bool isMul() const;
	bool isUniformLoad() const;
	bool isTMUAWrite() const;
	bool isZero() const;
	bool isLast() const;

	SetCond const &setCond() const;
	std::string mnemonic(bool with_comments = false) const;

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
};


std::string mnemonics(Seq<Instr> const &code, bool with_comments = false);


// Instruction id: also the index of an instruction
// in the main instruction sequence
typedef int InstrId;


// ============================================================================
// Handy functions
// ============================================================================

void check_zeroes(Seq<Instr> const &instrs);

namespace Target {
namespace instr {

extern Reg const None;
extern Reg const ACC0;
extern Reg const ACC1;
extern Reg const ACC4;
extern Reg const QPU_ID;
extern Reg const ELEM_ID;
extern Reg const TMU0_S;
extern Reg const VPM_WRITE;
extern Reg const VPM_READ;
extern Reg const WR_SETUP;
extern Reg const RD_SETUP;
extern Reg const DMA_LD_WAIT;
extern Reg const DMA_ST_WAIT;
extern Reg const DMA_LD_ADDR;
extern Reg const DMA_ST_ADDR;
extern Reg const SFU_EXP;

// Following registers are synonyms for v3d code generation,
// to better indicate the intent. Definitions of vc4 concepts
// are reused here, in order to prevent the code getting into a mess.
extern Reg const TMUD;
extern Reg const TMUA;

Reg rf(uint8_t index);

Instr bor(Reg dst, Reg srcA, Reg srcB);
Instr band(Reg dst, Reg srcA, Reg srcB);
Instr band(Var dst, Var srcA, Var srcB);
Instr band(Reg dst, Reg srcA, int n);
Instr bxor(Var dst, Var srcA, int n);
Instr mov(Var dst, Var src);
Instr mov(Var dst, Reg src);
Instr mov(Var dst, int n);
Instr mov(Reg dst, Var src);
Instr mov(Reg dst, int n);
Instr mov(Reg dst, Reg src);
Instr shl(Reg dst, Reg srcA, int val);
Instr add(Reg dst, Reg srcA, Reg srcB);
Instr add(Reg dst, Reg srcA, int n);
Instr sub(Reg dst, Reg srcA, int n);
Instr shr(Reg dst, Reg srcA, int n);
Instr li(Reg dst, int i);
Instr li(Var v, int i);
Instr li(Var v, float f);
Instr branch(Label label);
Instr branch(BranchCond cond, Label label);
Instr label(Label in_label);

// SFU functions
Seq<Instr> bexp(Var dst, Var srcA);

// v3d only
Instr tmuwt();

}  // namespace instr
}  // namespace Target

}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_SYNTAX_H_
