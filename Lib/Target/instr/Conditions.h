#ifndef _V3DLIB_TARGET_SYNTAX_INSTR_CONDITIONS_H_
#define _V3DLIB_TARGET_SYNTAX_INSTR_CONDITIONS_H_
#include <string>

namespace V3DLib {

class CmpOp;  // Forward declaration

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

private:
	Tag m_tag = NO_COND;

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

}  // namespace V3DLib

#endif  // define _V3DLIB_TARGET_SYNTAX_INSTR_CONDITIONS_H_
