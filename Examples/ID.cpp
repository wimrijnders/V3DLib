#include <QPULib.h>
#include "Support/Settings.h"

using namespace QPULib;

QPULib::Settings settings;

// Define function that runs on the GPU.

void hello(Ptr<Int> p)
{
  p = p + (me() << 4);
  *p = me();
}


int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Construct kernel
  auto k = compile(hello);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(192);
  for (int i = 0; i < 192; i++)
    array[i] = 0;

  // Invoke the kernel
  k.setNumQPUs(12);
	settings.process(k, &array);

	// Display the result
  for (int i = 0; i < 192; i++) {
    printf("%i: %i\n", i, array[i]);
  }
  
  return 0;
}
