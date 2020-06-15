#include "QPULib.h"
#include "Support/Settings.h"

using namespace QPULib;

Settings settings;


// Define function that runs on the GPU.

void hello(Ptr<Int> p)
{
  *p = 1;
}


int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Construct kernel
  auto k = compile(hello);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(16);
  for (int i = 0; i < 16; i++)
    array[i] = 100;

  // Invoke the kernel and display the result
  k(&array);
  for (int i = 0; i < 16; i++) {
    printf("%i: %i\n", i, array[i]);
  }

	settings.process(k);  
  return 0;
}
