#include "SourceTranslate.h"
#include <memory>
#include "Support/debug.h"
#include "Support/Platform.h"
#include "vc4/SourceTranslate.h"
#include "v3d/SourceTranslate.h"

namespace {
  std::unique_ptr<V3DLib::ISourceTranslate> _vc4_source_translate;
  std::unique_ptr<V3DLib::ISourceTranslate> _v3d_source_translate;

}  // anon namespace


namespace V3DLib {

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

}  // namespace V3DLib
