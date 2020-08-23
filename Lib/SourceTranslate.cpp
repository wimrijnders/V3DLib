#include "SourceTranslate.h"
#include <memory>
#include "Support/debug.h"
#include "Support/Platform.h"
#include "vc4/SourceTranslate.h"
#include "v3d/SourceTranslate.h"

namespace {

	bool _compiling_for_vc4 = true;
	std::unique_ptr<QPULib::ISourceTranslate> _vc4_source_translate;
	std::unique_ptr<QPULib::ISourceTranslate> _v3d_source_translate;

}  // anon namespace


namespace QPULib {

void set_compiling_for_vc4(bool val) {
	_compiling_for_vc4 = val;
}


ISourceTranslate &getSourceTranslate() {
	if (_compiling_for_vc4) {
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
