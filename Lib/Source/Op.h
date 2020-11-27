#ifndef _V3DLIB_SOURCE_OP_H_
#define _V3DLIB_SOURCE_OP_H_

namespace V3DLib {

// ============================================================================
// Operators
// ============================================================================

// Every operator has a type associated with it.
// Also used in other operator/comparison classes.
enum BaseType { UINT8, INT16, INT32, FLOAT };

// Order of operators is important to the random generator, see `Gen.cpp`.
// This applies to the range ROTATE...ROR.
enum OpId {
	// Int & Float operators:
	ROTATE, ADD, SUB, MUL, MIN, MAX,

	// Int only operators:
	SHL, SHR, USHR, BOR, BAND, BXOR, BNOT, ROR,

	// Conversion operators:
	ItoF, FtoI,

	// v3d only
	TIDX,
	EIDX
};


/**
 * Pair containing operator and base type
 */
struct Op {
	OpId op;
	BaseType type;

	Op(OpId in_op, BaseType in_type) : op(in_op), type(in_type) {}

	const char *to_string() const;
	bool noParams() const;  // Yes, I know, doesn't make sense. Happens anyway
	bool isUnary() const;
	bool isCommutative() const;
};

}  // namespace V3DLib


#endif  //  _V3DLIB_SOURCE_OP_H_
