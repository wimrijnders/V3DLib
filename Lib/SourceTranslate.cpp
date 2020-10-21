#include "SourceTranslate.h"
#include <memory>
#include "Support/debug.h"
#include "Support/Platform.h"
#include "vc4/SourceTranslate.h"
#include "v3d/SourceTranslate.h"

namespace {
	std::unique_ptr<QPULib::ISourceTranslate> _vc4_source_translate;
	std::unique_ptr<QPULib::ISourceTranslate> _v3d_source_translate;

}  // anon namespace


namespace QPULib {

int ISourceTranslate::get_init_begin_marker(Seq<Instr> &code) {
	// Find the init begin marker
	int index = 0;
	for (; index < code.size(); ++index) {
		if (code[index].tag == INIT_BEGIN) break; 
	}
	assertq(index < code.size(), "Expecting INIT_BEGIN marker.");
	assertq(index >= 2, "Expecting at least two uniform loads.");

	return index;
}


/**
 * Generate code to add an offset to the uniforms which are pointers.
 *
 * The calculated offset is assumed to be in ACC0
 */
Seq<Instr> ISourceTranslate::add_uniform_pointer_offset(Seq<Instr> &code) {
	using namespace QPULib::Target::instr;

	Seq<Instr> ret;

	// add the offset to all the uniform pointers
	for (int index = 0; index < code.size(); ++index) {
		auto &instr = code[index];

		if (!instr.isUniformLoad()) {  // Assumption: uniform loads always at top
			break;
		}

		if (instr.ALU.srcA.tag == REG && instr.ALU.srcA.reg.isUniformPtr) {
			ret << add(rf((uint8_t) index), rf((uint8_t) index), ACC0);
		}
	}

	return ret;
}


ISourceTranslate &getSourceTranslate() {
	if (Platform::instance().compiling_for_vc4()) {
		if (_vc4_source_translate.get() == nullptr) {
			_vc4_source_translate.reset(new vc4::SourceTranslate());
		}
		return *_vc4_source_translate.get();
	} else {
		if (_v3d_source_translate.get() == nullptr) {
			_v3d_source_translate.reset(new v3d::SourceTranslate());
		}
		return *_v3d_source_translate.get();
	}
}

}  // namespace QPULib
