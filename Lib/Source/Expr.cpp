#include "Expr.h"
#include "Target/SmallLiteral.h"
#include "Support/basics.h"
#include "Source/Lang.h"  // assign()

namespace V3DLib {

using ::operator<<;  // C++ weirdness


Expr::Expr(Expr const &rhs) :
  m_tag(rhs.m_tag),
  m_exp_a(rhs.m_exp_a),
  m_exp_b(rhs.m_exp_b)
{
  // sort out union and other things dependent on tag
  switch(m_tag) {
    case INT_LIT  : intLit     = rhs.intLit; break;
    case FLOAT_LIT: floatLit   = rhs.floatLit; break;
    case VAR:       m_var      = rhs.m_var; break;
    case APPLY:     m_apply_op.reset(new Op(*rhs.m_apply_op)); break;  // I hate this

    case DEREF: break;
    default: assert(false); break;
  }
}


Expr::Expr(Var in_var) : m_var(in_var), m_tag(VAR) {}


Expr::Expr(int in_lit) {
  m_tag = INT_LIT; 
  intLit = in_lit;
}


Expr::Expr(float in_lit) {
  m_tag = FLOAT_LIT; 
  floatLit = in_lit;
}


Expr::Expr(Ptr in_lhs, Op const &op, Ptr in_rhs) {
  m_tag = APPLY;
  lhs(in_lhs);
  m_apply_op.reset(new Op(op));  // urgh
  rhs(in_rhs);
}


Var Expr::var() const {
  assertq(m_tag == VAR, "Expr is not a VAR, shouldn't access var member.", true);
  return m_var;
}


Expr::Expr(Ptr ptr) {
  m_tag = DEREF;
  deref_ptr(ptr);
}


Expr::Ptr Expr::lhs() const {
  assert(m_tag == APPLY && m_exp_a.get() != nullptr);
  return m_exp_a;
}


Expr::Ptr Expr::rhs() const {
  assert(m_tag == APPLY && m_exp_b.get() != nullptr);
  return m_exp_b;
}


Op const &Expr::apply_op() const {
  assert(m_tag == APPLY && m_apply_op != nullptr);
  return *m_apply_op;
}


Expr::Ptr Expr::deref_ptr() const {
  assert(m_tag == DEREF && m_exp_a.get() != nullptr);
  return m_exp_a;
}


void Expr::lhs(Ptr p) {
  assert(m_tag == APPLY);
  m_exp_a = p;
}


void Expr::rhs(Ptr p) {
  assert(m_tag == APPLY);
  m_exp_b = p;
}


void Expr::deref_ptr(Ptr p) {
  assert(m_tag == DEREF);
  m_exp_a = p;
}


std::string Expr::disp_apply() const {
  std::string ret;

  auto op = apply_op();

  if (op.noParams()) {
    ret << op.to_string() << "()";
  } else if (op.isFunction()) {
    ret << op.to_string() << "(" << lhs()->pretty() << ")";
  } else if (op.isUnary()) {
    ret << "(" << op.to_string() << lhs()->pretty() << ")";
  } else {
    ret << "(" << lhs()->pretty() << op.to_string() << rhs()->pretty() <<  ")";
  }

  return ret;
}


std::string Expr::pretty() const {
  std::string ret;

  switch (tag()) {
    case INT_LIT:   ret << intLit;                       break;
    case FLOAT_LIT: ret << floatLit;                     break;
    case VAR:       ret << m_var.dump();                 break;
    case APPLY:     ret << disp_apply();                 break;
    case DEREF:     ret << "*" << deref_ptr()->pretty(); break;
    default:
      assert(false);
    break;
  }

  return ret;
}


std::string Expr::dump() const {
  std::string ret;

  switch(m_tag) {
    case INT_LIT:   ret << "Int "    << intLit;              break;
    case FLOAT_LIT: ret << "Float "  << floatLit;            break;
    case VAR:       ret << "Var: "   << m_var.dump();        break;
    case APPLY:     ret << "Apply: " << disp_apply();        break;
    case DEREF:     ret << "Deref: " << deref_ptr()->dump(); break;
    default:
      assertq(false, "Invalid tag for Expr", true);
    break;
  }

  return ret;
}


/**
 * An expression is 'simple' if it is a small literal or a variable.
 */
bool Expr::isSimple() const {
  bool isSmallLit = encodeSmallLit(*this) >= 0;
  return (m_tag == VAR) || isSmallLit;
}


// ============================================================================
// Class BaseExpr
// ============================================================================

BaseExpr::BaseExpr() {}


BaseExpr::BaseExpr(char const *label) : m_label(label) {
  assert(label != nullptr);
  assign_intern();
}


BaseExpr::BaseExpr(Expr::Ptr e, char const *label) : m_label(label) {
  assert(e != nullptr);
  m_expr = e;
}


void BaseExpr::assign_intern() {
  Var v  = VarGen::fresh();
  m_expr = mkVar(v);
}


void BaseExpr::assign_intern(Expr::Ptr expr) {
  assign_intern();
  assign(m_expr, expr);
}


/**
 * Create deref with added index offset to base
 *
 * Meant for Ptr-types
 */
Expr::Ptr BaseExpr::deref_with_index(Expr::Ptr base, Expr::Ptr index_expr) {
  return mkDeref(mkApply(base, Op(ADD, INT32), mkApply(index_expr, Op(SHL, INT32), mkIntLit(2))));
}


std::string BaseExpr::dump() const {
  std::string ret;
  ret << m_label << " " << m_expr->dump(); 
  return ret;
}


// ============================================================================
// Functions on expressions
// ============================================================================

Expr::Ptr mkIntLit(int lit)      { return std::make_shared<Expr>(lit); }
Expr::Ptr mkVar(Var var)         { return std::make_shared<Expr>(var); }
Expr::Ptr mkDeref(Expr::Ptr ptr) { return std::make_shared<Expr>(ptr); }


/**
 * Binary op version
 *
 * Unary operation can be passed in, the second parameter 'rhs'
 * will be ignored in the assembly.
 */
Expr::Ptr mkApply(Expr::Ptr lhs, Op const &op, Expr::Ptr rhs) {
  return std::make_shared<Expr>(lhs, op, rhs);
}


/**
 * Unary op version
 */
Expr::Ptr mkApply(Expr::Ptr lhs, Op const &op) {
  if (!op.isUnary()) {
    std::string msg;
    msg << "mkApply(): " << op.dump() << " expected to be unary";
    assertq(false, msg);
  }
  return std::make_shared<Expr>(lhs, op, mkIntLit(0));
}


// ============================================================================
// Class IntExpr                   
// ============================================================================

IntExpr::IntExpr(int x) { m_expr = mkIntLit(x); }


}  // namespace V3DLib
