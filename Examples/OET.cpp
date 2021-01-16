#include "V3DLib.h"
#include "Support/Settings.h"

using namespace V3DLib;

V3DLib::Settings settings;

/**
 * Odd/even transposition sorter for a 32-element array
 */
void oet(Ptr<Int> p) {
  Int evens = *p;
  Int odds  = *(p+16);

  For (Int count = 0, count < 16, count++)
    Int evens2 = min(evens, odds);
    Int odds2  = max(evens, odds);

    Int evens3 = rotate(evens2, 15);
    Int odds3  = odds2;
    Where (index() != 15)
      odds2 = min(evens3, odds3);
    End

    Where (index() != 0)
      evens2 = rotate(max(evens3, odds3), 1);
    End

    evens = evens2;
    odds  = odds2;
  End

  *p      = evens;
  *(p+16) = odds;
}


int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Construct kernel
  auto k = compile(oet);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> a(32);
  for (int i = 0; i < (int) a.size(); i++)
    a[i] = 100 - i;

	k.load(&a);           // Load the uniforms
	settings.process(k);  // Invoke the kernel

	// Display the result
  for (int i = 0; i < (int) a.size(); i++)
    printf("%i: %i\n", i, (i & 1) ? a[16+(i>>1)] : a[i>>1]);
  
  return 0;
}
