#include <QPULib.h>
#include "Support/Settings.h"

using namespace QPULib;

QPULib::Settings settings;

// Define function that runs on the GPU.

void id_kernel(Ptr<Int> p)
{
  *p = me();
}


int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Construct kernel
  auto k = compile(id_kernel);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(16*k.maxQPUs());
  for (int i = 0; i < array.size(); i++)
    array[i] = 0;

  k.setNumQPUs(k.maxQPUs());  // Invoke the kernel
	k.load(&array);             // Load the uniforms
	settings.process(k);

	// Display the result
  for (int i = 0; i < array.size(); i++) {
    printf("%i: %i\n", i, array[i]);
  }
  
  return 0;
}
