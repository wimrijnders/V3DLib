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


///////////////////////////////////////////////////////////////////////////////
// Class PointerExpr and derivatives
///////////////////////////////////////////////////////////////////////////////

class PointerExpr : public BaseExpr {
public:
  PointerExpr(Expr::Ptr e);
  PointerExpr(BaseExpr const &e);

protected:
  PointerExpr add(IntExpr b);

private:
  PointerExpr &me();
};


/**
 * This exist to impose some form of type safety in the kernel code.
 *
 * A 'PtrExpr<T>' defines a pointer expression which can only be used on the
 * RHS of assignment statements.
 */
template <typename T>
struct PtrExpr : public PointerExpr {
  PtrExpr(Expr::Ptr e) : PointerExpr(e) {}

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
    breakpoint  // TODO When is this ever called??
    auto e = deref_with_index(expr(), index.expr());
    return Deref<T>(e);
  }


  PtrExpr<T> operator+(IntExpr b) {
    Expr::Ptr e = add(b).expr();
    return PtrExpr<T>(e);
  }
};


///////////////////////////////////////////////////////////////////////////////
// Class Deref
///////////////////////////////////////////////////////////////////////////////

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


///////////////////////////////////////////////////////////////////////////////
// Class Pointer and derivatives
///////////////////////////////////////////////////////////////////////////////

class Pointer : public BaseExpr {
public:
  Pointer();
  Pointer(PointerExpr rhs);

  void inc();
  PointerExpr operator=(PointerExpr rhs);
  PointerExpr operator+(int b);
  PointerExpr operator+=(int b);
  PointerExpr operator+(IntExpr b);
  PointerExpr operator-(IntExpr b);

  static void reset_increment();

protected:
  PointerExpr addself(int b);
  PointerExpr bare_addself(Int &b);
  PointerExpr subself(IntExpr b);
  PointerExpr add(int b);
  PointerExpr add(IntExpr b);
  PointerExpr sub(IntExpr b);

private:
  Pointer &me();
};


/**
 * This exist to impose some form of type safety in the kernel code.
 *
 * A 'Ptr<T>' defines a pointer variable which can be used in both the lhs and
 * rhs of an assignment.
 */
template <typename T>
class Ptr : public Pointer {
  using Parent = Pointer;

public:
  Ptr() = default;

/*
  // TODO get rid of ctor acting as reference
  Ptr<T>(Ptr<T> const &rhs) : Ptr<T>() {
    assign(expr(), rhs.expr());
  }

  Ptr<T>(PtrExpr<T> rhs) : Ptr<T>() {
    assign(expr(), rhs.expr());
  }
*/

  Ptr<T>(PtrExpr<T> rhs) : Pointer(rhs) {}

  // Assignment
  Ptr<T>& operator=(Ptr<T> &rhs) {
    assign(expr(), rhs.expr());
    return *this; //rhs;
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

  PtrExpr<T> operator+(int b) {
    Expr::Ptr e = add(b).expr();
    return PtrExpr<T>(e);
  }

  PtrExpr<T> operator+(IntExpr b) {
    Expr::Ptr e = add(b).expr();
    return PtrExpr<T>(e);
  }

  PtrExpr<T> operator-(IntExpr b) {
    Expr::Ptr e = sub(b).expr();
    return PtrExpr<T>(e);
  }

  PtrExpr<T> operator+=(int b) {
    Expr::Ptr e = addself(b).expr();
    return PtrExpr<T>(e);
  }

  PtrExpr<T> operator-=(IntExpr b) {
    Expr::Ptr e = subself(b).expr();
    return PtrExpr<T>(e);
  }
};


///////////////////////////////////////////////////////////////////////////////
// Specific operations (which I want to get rid of)
///////////////////////////////////////////////////////////////////////////////

template <typename T>
inline PtrExpr<T> getUniformPtr() {
  Var v = Var(UNIFORM);
  v.setUniformPtr();
  Expr::Ptr e = std::make_shared<Expr>(v);
  return PtrExpr<T>(e);
}

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_PTR_H_
