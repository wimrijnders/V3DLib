// This module defines type 'Ptr<T>' type denoting a pointer to a
// value of type 'T'.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _V3DLIB_SOURCE_PTR_H_
#define _V3DLIB_SOURCE_PTR_H_
#include "Int.h"
#include "CExpr.h"
#include "Support/debug.h"

namespace V3DLib {
  //
  // Extra declaration to prevent error:
  //
  //   error: there are no arguments to ‘assign’ that depend on a template parameter,
  //          so a declaration of ‘assign’ must be available [-fpermissive]
  //          
  void assign(Expr::Ptr lhs, Expr::Ptr rhs);


// ============================================================================
// Types                   
// ============================================================================

// A 'PtrExpr<T>' defines a pointer expression which can only be used on the
// RHS of assignment statements.
template <typename T>
struct PtrExpr : public BaseExpr {
  PtrExpr(Expr::Ptr e) : BaseExpr(e, "PtrExpr") {}

  /**
   * Dereference
   */
  Deref<T> operator*() {
    auto e = mkDeref(expr());
    return Deref<T>(e);
  }


  /**
   * Array index
   */
  Deref<T> operator[](IntExpr index) {
    //breakpoint  // TODO When is this ever called??
    auto e = deref_with_index(expr(), index.expr());
    return Deref<T>(e);
  }
};


template <typename T>
struct Deref : public BaseExpr {
  explicit Deref(Expr::Ptr e) : BaseExpr(e) {}

  T &operator=(T &rhs) {
    assign(m_expr, rhs.expr());
    return rhs;
  }

  T const &operator=(T const &rhs) {
    assign(m_expr, rhs.expr());
    return rhs;
  }
};


// A 'Ptr<T>' defines a pointer variable which can be used in both the LHS and
// RHS of an assignment.

template <typename T>
struct Ptr : public BaseExpr {

  Ptr<T>() : BaseExpr(mkVar(freshVar()), "Ptr") {}

/*
	// TODO get rid of ctor acting as reference
  Ptr<T>(Ptr<T> const &rhs) : Ptr<T>() {
    assign(expr(), rhs.expr());
  }
*/

  Ptr<T>(PtrExpr<T> rhs) : Ptr<T>() {
    assign(expr(), rhs.expr());
  }

  // Assignment
  Ptr<T>& operator=(Ptr<T>& rhs) {
    assign(expr, rhs.expr);
    return rhs;
  }

  PtrExpr<T> operator=(PtrExpr<T> rhs) {
    assign(expr(), rhs.expr());
    return rhs;
  }


  /**
   * Dereference
   */
  Deref<T> operator*() {
    auto e = mkDeref(expr());
    return Deref<T>(e);
  }


  /**
   * Array index
   */
  Deref<T> operator[](IntExpr index) {
    //breakpoint  // TODO When is this ever called??
    auto e = deref_with_index(expr(), index.expr());
    return Deref<T>(e);
  }
};


// ============================================================================
// Specific operations
// ============================================================================

template <typename T>
inline PtrExpr<T> getUniformPtr() {
  Var v = Var(UNIFORM);
  v.setUniformPtr();
  Expr::Ptr e = std::make_shared<Expr>(v);
  return PtrExpr<T>(e);
}


template <typename T>
inline PtrExpr<T> operator+(PtrExpr<T> a, int b) {
  Expr::Ptr e = mkApply(a.expr(), Op(ADD, INT32), mkIntLit(4*b));
  return PtrExpr<T>(e);
}


template <typename T>
inline PtrExpr<T> operator+(Ptr<T> &a, int b) {
  Expr::Ptr e = mkApply(a.expr(), Op(ADD, INT32), mkIntLit(4*b));
  return PtrExpr<T>(e);
}


template <typename T> inline PtrExpr<T> operator+=(Ptr<T> &a, int b) {
  return a = a + b;
}

template <typename T> inline PtrExpr<T> operator+(PtrExpr<T> a, IntExpr b) {
  Expr::Ptr e = mkApply(a.expr(), Op(ADD, INT32), (b << 2).expr());
  return PtrExpr<T>(e);
}

template <typename T> inline PtrExpr<T> operator+(Ptr<T> &a, IntExpr b) {
  Expr::Ptr e = mkApply(a.expr(), Op(ADD, INT32), (b << 2).expr());
  return PtrExpr<T>(e);
}

template <typename T> inline PtrExpr<T> operator-(Ptr<T> &a, IntExpr b) {
  Expr::Ptr e = mkApply(a.expr(), Op(SUB, INT32), (b << 2).expr());
  return PtrExpr<T>(e);
}

template <typename T> inline PtrExpr<T> operator-=(Ptr<T> &a, IntExpr b) {
  return a = a - b;
}

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_PTR_H_
