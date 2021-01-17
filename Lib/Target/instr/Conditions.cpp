#include "Conditions.h"
#include "Support/basics.h"
#include "Source/BExpr.h"

namespace V3DLib {
namespace {

/**
 * Negate a condition flag
 */
Flag negFlag(Flag flag) {
  switch(flag) {
    case ZS: return ZC;
    case ZC: return ZS;
    case NS: return NC;
    case NC: return NS;
  }

  // Not reachable
  assert(false);
  return ZS;
}


const char *pretty(Flag flag) {
  switch (flag) {
    case ZS: return "ZS";
    case ZC: return "ZC";
    case NS: return "NS";
    case NC: return "NC";
  	default: assert(false); return "";
  }
}

} // anon namespace

///////////////////////////////////////////////////////////////////////////////
// Class BranchCond
///////////////////////////////////////////////////////////////////////////////

BranchCond BranchCond::negate() const {
  BranchCond ret;

  switch (tag) {
    case COND_NEVER:  ret.tag  = COND_ALWAYS; break;
    case COND_ALWAYS: ret.tag  = COND_NEVER;  break;
    case COND_ANY:    ret.tag  = COND_ALL; ret.flag = negFlag(flag); break;
    case COND_ALL:    ret.tag  = COND_ANY; ret.flag = negFlag(flag); break;
  	default:
  		assert(false);
  		break;
  }

  return ret;
}


std::string BranchCond::to_string() const {
  std::string ret;

  switch (tag) {
    case COND_ALL:
      ret << "all(" << pretty(flag) << ")";
      break;
    case COND_ANY:
      ret << "any(" << pretty(flag) << ")";
      break;
    case COND_ALWAYS:
      ret = "always";
      break;
    case COND_NEVER:
      ret = "never";
      break;
  }

  return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Class SetCond
///////////////////////////////////////////////////////////////////////////////

/*
SetCond::SetCond(CmpOp const &cmp_op) {
  setOp(cmp_op);
}
*/


const char *SetCond::to_string() const {
  switch (m_tag) {
  	case NO_COND: return "None";
  	case Z:       return "Z";
  	case N:       return "N";
  	case C:       return "C";
  	default:
  		assert(false);
  		return "<UNKNOWN>";
  }
}


std::string SetCond::pretty() const {
  std::string ret;
  if (flags_set()) {
  	ret << "{sf-" << to_string() << "}";
  }
  return ret;
}


void SetCond::setFlag(Flag flag) {
  Tag set_tag = NO_COND;

  switch (flag) {
  	case ZS: 
  	case ZC: 
  		set_tag = SetCond::Z;
  		break;
  	case NS: 
  	case NC: 
  		set_tag = SetCond::N;
  		break;
  	default:
  		assert(false);  // Not expecting anything else right now
  		break;
  }

  tag(set_tag);
}


///////////////////////////////////////////////////////////////////////////////
// Class AssignCond
///////////////////////////////////////////////////////////////////////////////

namespace {

std::string pretty(AssignCond cond) {
  using Tag = AssignCond::Tag;

  switch (cond.tag) {
    case Tag::ALWAYS: return "always";
    case Tag::NEVER:  return "never";
    case Tag::FLAG:   return pretty(cond.flag);
  	default: assert(false); return "";
  }
}

}  // anon namespace


AssignCond always(AssignCond::Tag::ALWAYS);  // Is a global to reduce eyestrain in gdb
AssignCond never(AssignCond::Tag::NEVER);    // idem


AssignCond::AssignCond(CmpOp const &cmp_op) : tag(FLAG), flag(cmp_op.assign_flag()) {}


AssignCond AssignCond::negate() const {
  AssignCond ret = *this;

  switch (tag) {
    case NEVER:  ret.tag = ALWAYS; break;
    case ALWAYS: ret.tag = NEVER;  break;
    case FLAG:   ret.flag = negFlag(flag); break;
  	default:
  		assert(false);
  		break;
  }

  return ret;
}


std::string AssignCond::to_string() const {
  auto ALWAYS = AssignCond::Tag::ALWAYS;

  if (tag == ALWAYS) {
  	return "";
  } else {
  	std::string ret;
  	ret << "where " << pretty(*this) << ": ";
  	return ret;
  }
}


/**
 * Translate an AssignCond instance to a BranchCond instance
 *
 * @param do_all  if true, set BranchCond tag to ALL, otherwise set to ANY
 */
BranchCond AssignCond::to_branch_cond(bool do_all) const {
  BranchCond bcond;
  if (is_always()) { bcond.tag = COND_ALWAYS; return bcond; }
  if (is_never())  { bcond.tag = COND_NEVER; return bcond; }

  assert(tag == AssignCond::Tag::FLAG);

  bcond.flag = flag;

  if (do_all) {
    bcond.tag = COND_ALL;
  } else {
    bcond.tag = COND_ANY;
  }

  return bcond;
}



}  // namespace V3DLib
