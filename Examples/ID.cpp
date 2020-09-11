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

	int array_size = 16*k.maxQPUs();

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(array_size);
  for (int i = 0; i < array_size; i++)
    array[i] = 0;

  k.setNumQPUs(k.maxQPUs());  // Invoke the kernel
	k.load(&array);             // Load the uniforms
	settings.process(k);

	// Display the result
  for (int i = 0; i < array_size; i++) {
    printf("%i: %i\n", i, array[i]);
  }
  
  return 0;
}
