#include "Stmt.h"
#include "Support/basics.h"
#include "vc4/DMA/DMA.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

// ============================================================================
// Class Stmt
// ============================================================================

/**
 * Replacement initializer for this class,
 * because a class with unions can not have a ctor.
 *
 * TODO union has been removed, dissolve this method
 */
void Stmt::init(Tag in_tag) {
  clear_comments();  // TODO prob not necessary, check

  assert(SKIP <= in_tag && in_tag <= DMA_START_WRITE);
  assertq(tag == SKIP, "Stmt::init(): can't reassign tag once assigned");
  tag = in_tag;
}


void Stmt::append(Ptr rhs) {
  Ptr s0;
  s0.reset(new Stmt(*this));
  auto tmp = create_sequence(s0, rhs);
  *this = *tmp;
}


void Stmt::cond(CExpr::Ptr cond) {
  m_cond = cond;
}

BExpr::Ptr Stmt::where_cond() const {
  assert(tag == WHERE);
  assert(m_where_cond.get() != nullptr);
  return m_where_cond;
}


void Stmt::where_cond(BExpr::Ptr cond) {
  assert(tag == WHERE);
  assert(m_where_cond.get() == nullptr);  // Don't reassign
  m_where_cond = cond;
}


Expr::Ptr Stmt::assign_lhs() const {
  assert(tag == ASSIGN);
  assert(m_exp_a.get() != nullptr);
  return m_exp_a;
}


Expr::Ptr Stmt::assign_rhs() const {
  assert(tag == ASSIGN);
  assert(m_exp_b.get() != nullptr);
  return m_exp_b;
}


Expr::Ptr Stmt::address() {
  assert(tag == LOAD_RECEIVE);
  assert(m_exp_a.get() != nullptr);
  assert(m_exp_b.get() == nullptr);
  return m_exp_a;
}


Stmt::Ptr Stmt::seq_s0() const {
  assert(tag == SEQ);
  assert(m_stmt_a.get() != nullptr);
  return m_stmt_a;
}


Stmt::Ptr Stmt::seq_s1() const {
  assertq(tag == SEQ, "Expecting SEQ", true);
  assert(m_stmt_b.get() != nullptr);
  return m_stmt_b;
}


Stmt::Ptr Stmt::thenStmt() const {
  assertq(tag == IF || tag == WHERE || tag == WHILE, "Then-statement only valid for IF, WHERE and WHILE", true);

  // while must have then and no else
  assert(tag != WHILE || (m_stmt_a.get() != nullptr || m_stmt_b.get() == nullptr));

  assertq(m_stmt_a.get() != nullptr || m_stmt_b.get() != nullptr,
         "thenStmt(): then and else blocks may not both be null",
         true);

  return m_stmt_a;  // May be null
}


Stmt::Ptr Stmt::body() const {
  assertq(tag == WHILE || tag == FOR, "Body-statement only valid for WHILE and FOR", true);
  assert(m_stmt_a.get() != nullptr);
  return m_stmt_a;
}


Stmt::Ptr Stmt::elseStmt() const {
  assertq(tag == IF || tag == WHERE, "Else-statement only valid for IF and WHERE", true);
  // where and else stmt may not both be null
  assert(m_stmt_a.get() != nullptr || m_stmt_b.get() != nullptr);
  return m_stmt_b; // May be null
}


void Stmt::thenStmt(Ptr then_ptr) {
  assertq(tag == IF || tag == WHERE, "Then-statement only valid for IF and WHERE", true);
  assert(m_stmt_a.get() == nullptr);  // Only assign once
  m_stmt_a = then_ptr;
}


/**
 * @return true if block successfully added, false otherwise
 */
bool Stmt::thenStmt(Array const &in_block) {
  bool ok = false;
  auto block = in_block.to_stmt();

  bool then_is_null = (m_stmt_a.get() == nullptr);

  if ((tag == Stmt::IF || tag == Stmt::WHERE ) && then_is_null) {
    thenStmt(block);
    ok = true;
  }

  return ok;
}


/**
 * @return true if block successfully added, false otherwise
 */
bool Stmt::add_block(Array const &in_block) {
  bool ok = false;
  auto block = in_block.to_stmt();

  bool then_is_null = (m_stmt_a.get() == nullptr);
  bool else_is_null = (m_stmt_b.get() == nullptr);

  if (tag == Stmt::IF || tag == Stmt::WHERE) {
    if (then_is_null) {
      thenStmt(block);
      ok = true;
    } else if (else_is_null) {
      m_stmt_b = block;
      ok = true;
    }
  }

  if (tag == Stmt::WHILE && body_is_null()) {
    m_stmt_a = block;
    ok = true;
  }

  if (tag == Stmt::FOR && body_is_null()) {
    // convert For to While

    //m_cond retained as is
    tag = WHILE;
    auto inc = m_stmt_b;
    Stmt::Ptr whileBody = Stmt::create_sequence(block, inc);
    m_stmt_a = whileBody;
    ok = true;
  }

  return ok;
}


void Stmt::inc(Ptr ptr) {
  assertq(tag == FOR, "Inc-statement only valid for FOR", true);
  assert(m_stmt_b.get() == nullptr);  // Only assign once
  m_stmt_b = ptr;
}


bool Stmt::body_is_null() const {
  assertq(tag == WHILE || tag == FOR, "Body-statement only valid for WHILE and FOR", true);
  return m_stmt_a.get() == nullptr;
}


/**
 * Debug routine for easier display of instance contents during debugging
 */
std::string Stmt::disp_intern(bool with_linebreaks, int seq_depth) const {
  std::string ret;

  switch (tag) {
    case SKIP:
      ret << "SKIP";
    break;
    case ASSIGN:
      ret << "ASSIGN " << assign_lhs()->dump() << " = " << assign_rhs()->dump();
    break;
    case SEQ:
      assert(seq_s0().get() != nullptr);
      assert(seq_s1().get() != nullptr);

      if (with_linebreaks) {
        std::string tmp;

        tmp << "  " << seq_s0()->disp_intern(with_linebreaks, seq_depth + 1) << "\n"
            << "  " << seq_s1()->disp_intern(with_linebreaks, seq_depth + 1) << "\n";

        // Remove all superfluous whitespace
        if (seq_depth == 0) {
          std::string tmp2;
          bool changed = true;

          while (changed)  {
            tmp2 = tmp;
            findAndReplaceAll(tmp2, "    ", "  ");
            findAndReplaceAll(tmp2, "\n\n", "\n");

            changed = (tmp2 != tmp);
            tmp = tmp2;
          }

          // TODO make indent based on sequence depth
          ret << "SEQ*: {\n" << tmp << "} END SEQ*\n";
        } else {
          ret << tmp;
        }
  
      } else {
        ret << "SEQ {" << seq_s0()->disp_intern(with_linebreaks, seq_depth + 1) << "; "
                       << seq_s1()->disp_intern(with_linebreaks, seq_depth + 1) << "}";
      }
    break;
    case WHERE:
      assert(m_where_cond.get() != nullptr);
      ret << "WHERE (" << m_where_cond->dump() << ") THEN " << thenStmt()->dump();
      if (elseStmt().get() != nullptr) {
        ret << " ELSE " << elseStmt()->dump();
      }
    break;
    case IF:
      assert(m_cond.get() != nullptr);
      ret << "IF (" << m_cond->dump() << ") THEN " << thenStmt()->dump();
      if (elseStmt().get() != nullptr) {
        ret << " ELSE " << elseStmt()->dump();
      }
    break;
    case WHILE:
      assert(m_cond.get() != nullptr);
      ret << "WHILE (" << m_cond->dump() << ") " << thenStmt()->dump();
      // There is no ELSE for while
    break;

    case GATHER_PREFETCH:  ret << "GATHER_PREFETCH";  break;
    case FOR:              ret << "FOR";              break;
    case LOAD_RECEIVE:     ret << "LOAD_RECEIVE";     break;

    default: {
        std::string tmp = DMA::disp(tag);
        if (tmp.empty()) {
          std::string msg;
          msg << "Unknown tag '" << tag << "' in Stmt::disp_intern()";

          if (tag < 0 || tag > DMA_START_WRITE) {
            msg << "; tag out of range";
          }

          assertq(false, msg, true);
        }
        ret << tmp;
      }
      break;
  }

  return ret;
}


Stmt::Ptr Stmt::create(Tag in_tag) {
  Ptr ret(new Stmt());
  ret->init(in_tag);
  return ret;
}


Stmt::Ptr Stmt::create(Tag in_tag, Expr::Ptr e0, Expr::Ptr e1) {
  // Intention: assert(!DMA::Stmt::is_dma_tag(in_tag);  - and change default below
  Ptr ret(new Stmt());
  ret->init(in_tag);

  switch (in_tag) {
    case ASSIGN:
      if (e0 == nullptr) {
        error("Stmt::create(): e0 is null, variable might not be initialized", true);
      }
      if (e1 == nullptr) {
        error("Stmt::create(): e1 is null, variable might not be initialized", true);
      }
      //assertq(e0 != nullptr && e1 != nullptr, "create 1");
      ret->m_exp_a = e0;
      ret->m_exp_b = e1;
    break;

    case LOAD_RECEIVE:
      assertq(e0 != nullptr && e1 == nullptr, "create 2");
      ret->m_exp_a = e0;
    break;

    case GATHER_PREFETCH:
      // Nothing to do
    break;

    default:
      if (!ret->dma.address(in_tag, e0)) {
        fatal("create(Expr,Expr): Tag not handled");
      }
    break;
  }

  return ret;
}


Stmt::Ptr Stmt::create(Tag in_tag, Ptr s0, Ptr s1) {
  Ptr ret(new Stmt());
  ret->init(in_tag);

  switch (in_tag) {
    case SEQ:
      assertq(s0.get() != nullptr && s1.get() != nullptr, "create 3");
      assertq(ret->m_stmt_a.get() == nullptr, "create() SEQ: don't reassign stmt a ptr");
      assertq(ret->m_stmt_b.get() == nullptr, "create() SEQ: don't reassign stmt b ptr");

      ret->m_stmt_a = s0;
      ret->m_stmt_b = s1;
    break;
    default:
      fatal("create(Stmt,Stmt): Tag not handled");
    break;
  }

  return ret;
}


Stmt::Ptr Stmt::create_assign(Expr::Ptr lhs, Expr::Ptr rhs) {
  return create(ASSIGN, lhs, rhs);
}


Stmt::Ptr Stmt::create_sequence(Ptr s0, Ptr s1) {
  return Stmt::create(SEQ, s0, s1);
}


CExpr::Ptr Stmt::if_cond() const {
  assert(tag == IF);
  assert(m_cond.get() != nullptr);
  return m_cond;
}


CExpr::Ptr Stmt::loop_cond() const {
  assert(tag == WHILE);
  assert(m_cond.get() != nullptr);
  return m_cond;
}


/**
 * Do a leftmost search for non-SEQ item
 */
Stmt *Stmt::first_in_seq() const {
  if (tag != SEQ) {
    if (tag == SKIP) {
      return nullptr;
    } else {
      assert(tag != Stmt::GATHER_PREFETCH);  // paranoia
      return const_cast<Stmt *>(this);
    }
  }

  Stmt *ret = seq_s0()->first_in_seq();
  if (ret != nullptr) {
    return ret;
  }

  return seq_s1()->first_in_seq();
}


Stmt *Stmt::last_in_seq() const {
  if (tag == SEQ) {
    return seq_s1()->last_in_seq();
  }

  assert(tag != SKIP);                   // paranoia
  assert(tag != Stmt::GATHER_PREFETCH);  // paranoia

  return const_cast<Stmt *>(this);
}




// ============================================================================
// Functions on statements
// ============================================================================
namespace {

Stmt::Ptr mkSkip() { return Stmt::create(Stmt::SKIP); }

}  // anon namespace


std::string Stmt::Array::dump() const {
  if (empty()) return "<Empty>";
  std::string ret;

  for (int i = 0; i < (int) size(); i++) {
    ret << (*this)[i]->dump() << "\n";
  }

  return ret;
}


Stmt::Ptr Stmt::Array::to_stmt() const {
  assert(!empty());
  Ptr ptr = mkSkip();

  for (int i = 0; i < (int) size(); i++) {
    ptr->append((*this)[i]);
  }

  return ptr;
}

}  // namespace V3DLib
