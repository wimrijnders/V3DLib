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


}  // namespace QPULib
