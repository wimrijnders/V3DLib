#ifndef _V3DLIB_SOURCE_STMT_H_
#define _V3DLIB_SOURCE_STMT_H_
#include "Support/InstructionComment.h"
#include "Int.h"
#include "Expr.h"
#include "CExpr.h"

namespace V3DLib {

// ============================================================================
// Class PrintStmt
// ============================================================================

// For displaying values in emulation
enum PrintTag { PRINT_INT, PRINT_FLOAT, PRINT_STR };

struct PrintStmt {
  PrintTag tag() const { return m_tag;}
  void tag(PrintTag tag) { m_tag = tag;}

  char const *str() const;
  void str(char const *str);
  std::string disp() const;

private:
  PrintTag    m_tag;
  const char *m_str;
};


// ============================================================================
// Class Stmt
// ============================================================================



/**
 * The original implementation put instances of `Stmt` on a custom heap.
 * This placed a strict limit on compilation size, which I continually ran
 * into, and complicated initialization of instances (notably, ctors were
 * not called).
 *
 * The custom heap has thus been removed and instances are allocated in
 * the regular C++ way.
 * Pointers within this definition are in the process of being replaced
 * with smart pointers.
 */
struct Stmt : public InstructionComment {
  using Ptr = std::shared_ptr<Stmt>;

  // What kind of statement is it?
  enum Tag {
    SKIP,
    ASSIGN,
    SEQ,
    WHERE,
    IF,
    WHILE,
    PRINT,
    FOR,
    LOAD_RECEIVE,
    STORE_REQUEST,

    GATHER_PRELOAD,

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
    DMA_START_WRITE
  };

  ~Stmt() {}

  std::string disp() const { return disp_intern(false, 0); }
  std::string dump() const { return disp_intern(true, 0); }

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
  Expr::Ptr stride();
  Expr::Ptr storeReq_data();
  Expr::Ptr storeReq_addr();
  Expr::Ptr address();
  Expr::Ptr print_expr() const;

  Ptr seq_s0() const;
  Ptr seq_s1() const;
  Ptr thenStmt() const;
  Ptr elseStmt() const;
  Ptr body() const;
  void thenStmt(Ptr then_ptr);
  void elseStmt(Ptr else_ptr);
  void body(Ptr ptr);
  void inc(Ptr ptr);
  bool then_is_null() const;
  bool else_is_null() const;
  bool body_is_null() const;

  void for_to_while(Ptr in_body);

  CExpr::Ptr if_cond() const;
  CExpr::Ptr loop_cond() const;

  //
  // Instantiation methods
  //
  static Ptr create(Tag in_tag);
  static Ptr create(Tag in_tag, Expr::Ptr e0, Expr::Ptr e1);  // TODO make private
  static Ptr create(Tag in_tag, Ptr s0, Ptr s1);
  static Ptr create_assign(Expr::Ptr lhs, Expr::Ptr rhs);
  static Ptr create_sequence(Ptr s0, Ptr s1);

  static Ptr mkIf(CExpr::Ptr cond, Ptr thenStmt, Ptr elseStmt);
  static Ptr mkWhile(CExpr::Ptr cond, Ptr body);
  static Ptr mkFor(CExpr::Ptr cond, Ptr inc, Ptr body);

  Tag tag;  // What kind of statement is it?

  union {
    // Print
    PrintStmt print;

    // Semaphore id for increment / decrement
    int semaId;

    // VPM read setup
    struct { int numVecs; int hor; int stride; } setupVPMRead;

    // VPM write setup
    struct { int hor; int stride; } setupVPMWrite;

    // DMA read setup
    struct {
      int numRows;
      int rowLen;
      int hor;
      int vpitch;
    } setupDMARead;

    // DMA write setup
    struct {
      int numRows;
      int rowLen;
      int hor;
    } setupDMAWrite;
  };

private:
  BExpr::Ptr m_where_cond;

  Expr::Ptr m_exp_a;
  Expr::Ptr m_exp_b;

  Ptr m_stmt_a;
  Ptr m_stmt_b;

  CExpr::Ptr m_cond;

  void init(Tag in_tag);
  std::string disp_intern(bool with_linebreaks, int seq_depth) const;
};


// Functions to construct statements
Stmt::Ptr mkSkip();
Stmt::Ptr mkWhere(BExpr::Ptr cond, Stmt::Ptr thenStmt, Stmt::Ptr elseStmt);
Stmt::Ptr mkPrint(PrintTag t, Expr::Ptr e);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_STMT_H_
