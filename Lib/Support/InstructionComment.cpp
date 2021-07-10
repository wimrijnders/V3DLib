#include "InstructionComment.h"
#include "Support/basics.h"

namespace V3DLib {

void InstructionComment::transfer_comments(InstructionComment const &rhs) {
  if (!rhs.header().empty()) {
    header(rhs.header());
  }

  if (!rhs.comment().empty()) {
    comment(rhs.comment());
  }
}


void InstructionComment::clear_comments() {
  m_header.clear();
  m_comment.clear();
}


/**
 * Assign header comment to current instance
 *
 * For display purposes only, when generating a dump of the opcodes.
 */
void InstructionComment::header(std::string const &msg) {
  if (msg.empty()) return;
  assertq(m_header.empty(), "Header comment already has a value when setting it", true);

  m_header = msg;
  findAndReplaceAll(m_header, "\n", "\n# ");
}


/**
 * Assign comment to current instance
 *
 * If a comment is already present, the new comment will be appended.
 *
 * For display purposes only, when generating a dump of the opcodes.
 */
void InstructionComment::comment(std::string msg) {
  if (msg.empty()) return;

  findAndReplaceAll(msg, "\n", "\n# ");

  if (!m_comment.empty()) {
    m_comment += "; ";
  }

  m_comment += msg;
}


std::string InstructionComment::emit_header() const {
  if (m_header.empty()) return "";

  std::string ret;
  ret << "\n# " << header() << "\n";
  return ret;
}


/**
 * Return comment as string with leading spaces
 *
 * NOTE: this does not take into account multi-line comments (don't occur at time of writing)
 *
 * @param instr_size  size of the associated instruction in bytes
 */
std::string InstructionComment::emit_comment(int instr_size) const {
  if (m_comment.empty()) return "";

  const int COMMENT_INDENT = 60;
  int spaces = COMMENT_INDENT - instr_size;
  if (spaces < 2) spaces = 2;

  std::string ret;
  ret << tabs(spaces) << "# " << m_comment;
  return ret;
}

}  // namespace V3DLib
