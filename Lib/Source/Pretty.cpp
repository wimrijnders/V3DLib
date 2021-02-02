#include "Pretty.h"
#include "Stmt.h"
#include "Support/basics.h"
#include "Support/Helpers.h"
#include "vc4/DMA/DMA.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

namespace {

std::string pretty(int indent, Stmt::Ptr s) {
  std::string ret;
  bool do_eol = true;

  switch (s->tag) {
    case Stmt::SKIP: do_eol = false; break;

    case Stmt::ASSIGN:
      ret << indentBy(indent)
          << s->assign_lhs()->pretty() << " = " << s->assign_rhs()->pretty() << ";";
      break;

    case Stmt::SEQ:  // Sequential composition
      ret << pretty(indent, s->seq_s0())
          << pretty(indent, s->seq_s1());
      do_eol = false;
      break;

    case Stmt::WHERE:
      ret << indentBy(indent)
          << "Where (" << s->where_cond()->dump() << ")\n"
          << pretty(indent+2, s->thenStmt());

      if (s->elseStmt().get() != nullptr) {
        ret << indentBy(indent) << "Else\n"
            << pretty(indent+2, s->elseStmt());
      }

      ret << indentBy(indent) << "End";
      break;

    case Stmt::IF:
      ret << indentBy(indent)
          << "If  (" << s->if_cond()->dump() << ")\n"
          << pretty(indent+2, s->thenStmt());

      if (s->elseStmt().get() != nullptr) {
        ret << indentBy(indent) << "Else\n"
            << pretty(indent+2, s->elseStmt());
      }

      ret << indentBy(indent) << "End";
      break;

    case Stmt::WHILE:
      ret << indentBy(indent)
          << "While  (" << s->loop_cond()->dump() << ")\n"
          << pretty(indent+2, s->body())
          << indentBy(indent) << "End";
      break;

    case Stmt::PRINT:
      ret << indentBy(indent)
          << "Print (";

      if (s->print.tag() == PRINT_STR) {
        ret << s->print.str();
      } else {
        ret << s->print_expr()->pretty();
      }

      ret << ")\n";
      break;

    case Stmt::LOAD_RECEIVE:
      ret << indentBy(indent)
          << "receive(" << s->address()->pretty() << ")";
      break;

    case Stmt::GATHER_PREFETCH:
      ret << indentBy(indent) << "Prefetch Tag";
      break;

    default: {
        std::string tmp = DMA::pretty(indent, s);
        if (tmp.empty()) {
          assertq(false, "Unknown statement tag in Source::pretty()");
        }
        ret << tmp;
      }
      break;
  }


  if (ret.empty()) {  // Can only be empty for SKIP
    return ret;
  }

  std::string out;
  out << s->emit_header()
      << ret

      // NOTE: For multiline output, the comment gets added to the end!
      //       We might want to fix this.
      << s->emit_comment(27);  // param is temp measure till we figure out size of current instruction
                               // TODO fix this (too lazy now)


  if (do_eol) {
    out << "\n";
  }

  return out;
}

}  // anon namespace


/**
 * Pretty printer for the V3DLib source language
 */
std::string pretty(Stmt::Ptr s) {
  assert(s.get() != nullptr);
  return pretty(0, s);
}

}  // namespace V3DLib
