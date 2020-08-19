#include "SourceTranslate.h"
#include <memory>
#include "Support/debug.h"
#include "Support/Platform.h"
#include "vc4/SourceTranslate.h"
#include "v3d/SourceTranslate.h"

namespace {

	std::unique_ptr<QPULib::ISourceTranslate> _source_translate;

}  // anon namespace


namespace QPULib {

ISourceTranslate &getSourceTranslate() {
	if (_source_translate.get() == nullptr) {
		if (Platform::instance().has_vc4) {
	breakpoint
			_source_translate.reset(new vc4::SourceTranslate());
		} else {
			_source_translate.reset(new v3d::SourceTranslate());
		}
	}

	return *_source_translate.get();
}

}  // namespace QPULib
