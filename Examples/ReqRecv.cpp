#include "QPULib.h"
#include "Support/Settings.h"

using namespace QPULib;

QPULib::Settings settings;

// Define function that runs on the GPU.

void test(Ptr<Int> p)
{
  Int x, y;
  gather(p+index());
  gather(p+16+index());
  receive(x);
  receive(y);
  *p = x+y;
}

int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Construct kernel
  auto k = compile(test);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(32);
  for (int i = 0; i < 32; i++)
    array[i] = i;

  // Invoke the kernel
  k.load(&array);
  settings.process(k);

	// Display the result
  for (int i = 0; i < 16; i++)
    printf("%i: %i\n", i, array[i]);
  
  return 0;
}
