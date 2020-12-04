#include "V3DLib.h"
#include "Support/Settings.h"

using namespace V3DLib;

V3DLib::Settings settings;

void loop(Int n) {
  For (Int i = 0, i < n, i++)
    Print(i);
    Print("\n");
  End
}

int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Construct kernel
  auto k = compile(loop);

  // Invoke kernel with argument 20
	k.load(20);
	settings.process(k);
  
  return 0;
}
