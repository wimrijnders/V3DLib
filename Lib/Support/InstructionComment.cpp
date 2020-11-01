#include "InstructionComment.h"

namespace {

/**
 * Source: https://thispointer.com/find-and-replace-all-occurrences-of-a-sub-string-in-c/
 */
void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr) {
    // Get the first occurrence
    size_t pos = data.find(toSearch);
    // Repeat till end is reached
    while( pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos =data.find(toSearch, pos + replaceStr.size());
    }
}

}  // anon namespace

namespace QPULib {


/**
 * Assign header comment to current instance
 *
 * For display purposes only, when generating a dump of the opcodes.
 */
void InstructionComment::header(std::string const &msg) {
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


/**
 * Return comment as string with leading spaces
 *
 * @param instr_size  size of the associated instruction in bytes
 */
std::string InstructionComment::emit_comment(int instr_size) const {
	if (m_comment.empty()) return "";

	const int COMMENT_INDENT = 57;
	int spaces = COMMENT_INDENT - instr_size;
	if (spaces < 2) spaces = 2;

	// Output spaces till the position of the comment
	// NOTE: this does not take into account multi-line comments (don't occur at time of writing)
	auto emit_spaces = [] (int spaces) -> std::string {
		std::string ret;

		while (spaces > 0) {
			ret += " ";
			spaces--;
		}

		return ret;
	};

	std::string ret;
	ret  << emit_spaces(spaces) << "# " << m_comment;
	return ret;
}


}  // namespace QPULib
