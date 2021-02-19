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

class BaseSharedArray;

//
// Extra declaration to prevent error:
//
//   error: there are no arguments to ‘assign’ that depend on a template parameter,
//          so a declaration of ‘assign’ must be available [-fpermissive]
//          
void assign(Expr::Ptr lhs, Expr::Ptr rhs);

///////////////////////////////////////////////////////////////////////////////
// Non-templated base class
///////////////////////////////////////////////////////////////////////////////

class PointerExpr : public BaseExpr {
public:
  PointerExpr(Expr::Ptr e);
  PointerExpr(BaseExpr const &e);

protected:
  PointerExpr add(IntExpr b);

private:
  PointerExpr &self();
};


class Pointer : public BaseExpr {
public:
  Pointer();
  Pointer(PointerExpr rhs);

  void inc();
  PointerExpr operator=(PointerExpr rhs);
  PointerExpr operator+(int b);
  PointerExpr operator+=(Int &b);
  PointerExpr operator+=(int b);
  PointerExpr operator+(IntExpr b);
  PointerExpr operator-(IntExpr b);

  static void reset_increment();
  static bool passParam(Seq<int32_t> *uniforms, BaseSharedArray const *p);
  static Expr::Ptr getUniformPtr();

protected:
  PointerExpr addself(int b);
  PointerExpr bare_addself(Int &b);
  PointerExpr addself(IntExpr b);
  PointerExpr subself(IntExpr b);
  PointerExpr add(int b);
  PointerExpr add(IntExpr b);
  PointerExpr sub(IntExpr b);

private:
  Pointer &self();
};


///////////////////////////////////////////////////////////////////////////////
// Templated classes
//
// These exist to impose some form of type safety in the kernel code.
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


/**
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


/**
 * A 'Ptr<T>' defines a pointer variable which can be used in both the lhs and
 * rhs of an assignment.
 */
template <typename T>
class Ptr : public Pointer {
  using Parent = Pointer;

public:
  Ptr() = default;

  /**
   * Gets rid of ctor acting as reference
   */
  Ptr<T>(Ptr<T> const &rhs) : Ptr<T>() {
    assign(expr(), rhs.expr());
  }

  Ptr<T>(PtrExpr<T> rhs) : Pointer(rhs) {}

  static Ptr<T> mkArg();

  // Assignments
  Ptr<T>&    operator=(Ptr<T> const &rhs) { assign(expr(), rhs.expr()); return *this; //rhs; }
  PtrExpr<T> operator=(PtrExpr<T> rhs)    { assign(expr(), rhs.expr()); return rhs; }


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

  PtrExpr<T> operator+(int b)      { Expr::Ptr e = add(b).expr();     return PtrExpr<T>(e); }
  PtrExpr<T> operator+(IntExpr b)  { Expr::Ptr e = add(b).expr();     return PtrExpr<T>(e); }
  PtrExpr<T> operator-(IntExpr b)  { Expr::Ptr e = sub(b).expr();     return PtrExpr<T>(e); }
  PtrExpr<T> operator+=(int b)     { Expr::Ptr e = addself(b).expr(); return PtrExpr<T>(e); }
  PtrExpr<T> operator+=(IntExpr b) { Expr::Ptr e = addself(b).expr(); return PtrExpr<T>(e); }
  PtrExpr<T> operator-=(IntExpr b) { Expr::Ptr e = subself(b).expr(); return PtrExpr<T>(e); }
};


template <typename T>
Ptr<T> Ptr<T>::mkArg() {
  Ptr<T> x;
  x = PtrExpr<T>(getUniformPtr());
  return x;
}

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_PTR_H_
