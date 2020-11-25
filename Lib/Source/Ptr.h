// This module defines type 'Ptr<T>' type denoting a pointer to a
// value of type 'T'.

#ifndef _V3DLIB_SOURCE_PTR_H_
#define _V3DLIB_SOURCE_PTR_H_

//#include <assert.h>
#include "Source/Syntax.h"

namespace V3DLib {
  //
  // Extra declaration to prevent error:
  //
  //   error: there are no arguments to ‘assign’ that depend on a template parameter,
  //          so a declaration of ‘assign’ must be available [-fpermissive]
  //          
  void assign(Expr* lhs, Expr* rhs);


// ============================================================================
// Types                   
// ============================================================================

// A 'PtrExpr<T>' defines a pointer expression which can only be used on the
// RHS of assignment statements.
template <typename T> struct PtrExpr {
  // Abstract syntax tree
  Expr* expr;

  PtrExpr<T>() { this->expr = NULL; }

  // Dereference
  T& operator*() {
    // This operation must return a reference to T, so we allocate the
    // AST node on the heap an return a reference to it.
    //T* p = astHeap.alloc<T>(1);
    T *p = new T;
		
    p->expr = mkDeref(expr);
    return *p;
  }

  // Array index
  T& operator[](IntExpr index) {
    //T* p = astHeap.alloc<T>(1);
		T *p = new T;
    p->expr = mkDeref(mkApply(expr, Op(ADD, INT32),
                mkApply(index.expr, Op(SHL, INT32), mkIntLit(2))));
    return *p;
  }
};

// A 'Ptr<T>' defines a pointer variable which can be used in both the LHS and
// RHS of an assignment.

template <typename T> struct Ptr {
  // Abstract syntax tree
  Expr* expr;

  // Constructors
  Ptr<T>() {
    Var v    = freshVar();
    this->expr = mkVar(v);
  }

  Ptr<T>(PtrExpr<T> rhs) {
    Var v    = freshVar();
    this->expr = mkVar(v);
    assign(this->expr, rhs.expr);
  }

/*
  // Copy constructors
  Ptr<T>(Ptr<T>& x) {
printf("Ptr copy ctor called\n");
    Var v    = freshVar();
    this->expr = mkVar(v);
    assign(this->expr, x.expr);
  }
  Ptr<T>(const Ptr<T>& x) {
printf("Ptr const copy ctor called\n");
    Var v    = freshVar();
    this->expr = mkVar(v);
    assign(this->expr, x.expr);
  }
*/

  // Assignment
  Ptr<T>& operator=(Ptr<T>& rhs) {
    assign(this->expr, rhs.expr);
    return rhs;
  }

  PtrExpr<T> operator=(PtrExpr<T> rhs) {
    assign(this->expr, rhs.expr);
    return rhs;
  }

  // Dereference
  T& operator*() {
    // This operation must return a reference to T, so we allocate the
    // AST node on the heap an return a reference to it.
    //T* p = astHeap.alloc<T>(1);
    T *p = new T;
    p->expr = mkDeref(expr);
    return *p;
  }

  // Array index
  T& operator[](IntExpr index) {
    //T* p = astHeap.alloc<T>(1);
    T *p = new T;
    p->expr = mkDeref(mkApply(expr, Op(ADD, INT32),
                mkApply(index.expr, Op(SHL, INT32), mkIntLit(2))));
    return *p;
  }
};

// ============================================================================
// Specific operations
// ============================================================================

template <typename T> inline PtrExpr<T> getUniformPtr() {
  Expr* e    = mkExpr();
  e->tag     = VAR;
  e->var.tag = UNIFORM;
  e->var.isUniformPtr = true;
  PtrExpr<T> x; x.expr = e; return x;
}

template <typename T> inline PtrExpr<T> operator+(PtrExpr<T> a, int b) {
  Expr* e = mkApply(a.expr, Op(ADD, INT32), mkIntLit(4*b));
  PtrExpr<T> x; x.expr = e; return x;
}

template <typename T> inline PtrExpr<T> operator+(Ptr<T> &a, int b) {
  Expr* e = mkApply(a.expr, Op(ADD, INT32), mkIntLit(4*b));
  PtrExpr<T> x; x.expr = e; return x;
}

template <typename T> inline PtrExpr<T> operator+=(Ptr<T> &a, int b) {
  return a = a + b;
}

template <typename T> inline PtrExpr<T> operator+(PtrExpr<T> a, IntExpr b) {
  Expr* e = mkApply(a.expr, Op(ADD, INT32), (b<<2).expr);
  PtrExpr<T> x; x.expr = e; return x;
}

template <typename T> inline PtrExpr<T> operator+(Ptr<T> &a, IntExpr b) {
  Expr* e = mkApply(a.expr, Op(ADD, INT32), (b<<2).expr);
  PtrExpr<T> x; x.expr = e; return x;
}

template <typename T> inline PtrExpr<T> operator-(Ptr<T> &a, IntExpr b) {
  Expr* e = mkApply(a.expr, Op(SUB, INT32), (b<<2).expr);
  PtrExpr<T> x; x.expr = e; return x;
}

template <typename T> inline PtrExpr<T> operator-=(Ptr<T> &a, IntExpr b) {
  return a = a - b;
}

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_PTR_H_
