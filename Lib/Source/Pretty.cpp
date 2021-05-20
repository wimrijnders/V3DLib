#include "Pretty.h"
#include "Stmt.h"
#include "Support/basics.h"
#include "Support/Helpers.h"
#include "vc4/DMA/DMA.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

namespace {

/**
 * Split given string into first line and the rest.
 *
 * If there is no newline, put the entire string in `first`.
 */
void split_first_line(std::string const &in, std::string &first, std::string &rest) {

 auto pos = in.find("\n");

 if (pos == in.npos) {
   // No newline detected
   first = in;
   rest.clear();
 } else {
   first = in.substr(0, pos);  // This includes the newline
   rest  = in.substr(pos + 1);
 }
}


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
          << "If (" << s->if_cond()->dump() << ")\n"
          << pretty(indent+2, s->thenStmt());

      if (s->elseStmt().get() != nullptr) {
        ret << indentBy(indent) << "Else\n"
            << pretty(indent+2, s->elseStmt());
      }

      ret << indentBy(indent) << "End";
      break;

    case Stmt::WHILE:
      ret << indentBy(indent)
          << "While (" << s->loop_cond()->dump() << ")\n"
          << pretty(indent+2, s->body())
          << indentBy(indent) << "End";
      break;

    case Stmt::LOAD_RECEIVE:
      ret << indentBy(indent)
          << "receive(" << s->address()->pretty() << ")";
      break;

    case Stmt::GATHER_PREFETCH:
      ret << indentBy(indent) << "Prefetch Tag";
      break;

    default: {
        std::string tmp = s->dma.pretty(indent, s->tag);
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
  out << s->emit_header();

  if (s->emit_comment(0).empty()) {
    out << ret;
  } else {
    std::string first;
    std::string rest;
    split_first_line(ret, first, rest);

    out << first << s->emit_comment((int) first.length());

    if (!rest.empty()) {
      out << "\n" << rest;
    }
/*
    std::string msg;
    msg << "first: '" << first << "'\n"
        << "rest : '" << rest  << "'\n"
        << "out  : '" << out   << "'\n";
    debug(msg);
*/
  }

  if (do_eol) {
    out << "\n";
  }

  return out;
}

}  // anon namespace


/**
 * Pretty printer for the V3DLib source language
 */
std::string pretty(Stmts const &s) {
  assert(!s.empty());
  std::string ret;

  for (int i = 0; i < (int) s.size(); ++i) {
    ret << pretty(0, s[i]);
  }

  return ret;
}

}  // namespace V3DLib
