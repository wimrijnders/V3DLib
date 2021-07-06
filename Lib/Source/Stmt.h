#ifndef _V3DLIB_SOURCE_STMT_H_
#define _V3DLIB_SOURCE_STMT_H_
#include "Support/InstructionComment.h"
#include "Int.h"
#include "Expr.h"
#include "CExpr.h"
#include "vc4/DMA/DMA.h"

namespace V3DLib {

// ============================================================================
// Class Stmt
// ============================================================================

/**
 * An instance is actually a tree of statements, due to having SEQ as 
 * a possible element.
 *
 * The original implementation put instances of `Stmt` on a custom heap.
 * This placed a strict limit on compilation size, which I continually ran
 * into, and complicated initialization of instances (notably, ctors were
 * not called).
 *
 * The custom heap has thus been removed and instances are allocated in
 * the regular C++ way.
 *
 * Pointers within this definition are in the process of being replaced
 * with smart pointers.
 * TODO check if done
 */
struct Stmt : public InstructionComment {
  using Ptr = std::shared_ptr<Stmt>;

  enum Tag {
    SKIP,
    ASSIGN,
    SEQ,
    WHERE,
    IF,
    WHILE,
    FOR,
    LOAD_RECEIVE,

    GATHER_PREFETCH,

    // DMA stuff
    SET_READ_STRIDE,
    SET_WRITE_STRIDE,
    SEND_IRQ_TO_HOST,
    SEMA_INC,
    SEMA_DEC,
    SETUP_VPM_READ,
    SETUP_VPM_WRITE,
    SETUP_DMA_READ,
    SETUP_DMA_WRITE,
    DMA_READ_WAIT,
    DMA_WRITE_WAIT,
    DMA_START_READ,
    DMA_START_WRITE,

    NUM_TAGS
  };

  class Array : public std::vector<Ptr> {
  public:
    std::string dump() const;
  };

  ~Stmt() {}

  Stmt &header(std::string const &msg) { InstructionComment::header(msg);  return *this; }
  Stmt &comment(std::string msg)       { InstructionComment::comment(msg); return *this; }

  std::string disp() const { return disp_intern(false, 0); }
  std::string dump() const { return disp_intern(true, 0); }
  void append(Array const &rhs);

  //
  // Accessors for pointer objects.
  // They basically all do the same thing, but in a controlled manner.
  // Done like this to ease the transition to smart pointers.
  // Eventually this can be cleaned up.
  //
  BExpr::Ptr where_cond() const;
  void where_cond(BExpr::Ptr cond);

  Expr::Ptr assign_lhs() const;
  Expr::Ptr assign_rhs() const;
  Expr::Ptr address();
  Stmt *first_in_seq() const;

  Array &stmts();  // TODO consider const

  // TODO rename to thenBlock and elseBlock

  Array const &thenStmts() const;
  bool thenStmt(Array const &in_block);

  Array const &elseStmts() const;

  bool add_block(Array const &in_block);
  Array const &body() const;

  void inc(Array const &arr);
  bool body_is_null() const;

  void cond(CExpr::Ptr cond);
  CExpr::Ptr if_cond() const;
  CExpr::Ptr loop_cond() const;

  //
  // Instantiation methods
  //
  static Ptr create(Tag in_tag);
  static Ptr create(Tag in_tag, Expr::Ptr e0, Expr::Ptr e1);
  static Ptr create_assign(Expr::Ptr lhs, Expr::Ptr rhs);

  Tag tag;
  DMA::Stmt dma;

  void break_point() { m_break_point = true; }
  bool do_break_point() const { return m_break_point; }

private:
  BExpr::Ptr m_where_cond;

  Expr::Ptr m_exp_a;
  Expr::Ptr m_exp_b;

  Array m_stmts_a;
  Array m_stmts_b;

  CExpr::Ptr m_cond;

  bool m_break_point = false;

  static Ptr create(Tag in_tag, Ptr s0, Ptr s1);
  void init(Tag in_tag);
  std::string disp_intern(bool with_linebreaks, int seq_depth) const;
};


using Stmts = Stmt::Array;

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_STMT_H_
