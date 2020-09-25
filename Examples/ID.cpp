#include <QPULib.h>
#include "Support/Settings.h"

using namespace QPULib;

QPULib::Settings settings;

// Define function that runs on the GPU.

void id_kernel(Ptr<Int> p, Ptr<Int> q)
{
  *p = me();
  *q = index();
}


int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Construct kernel
  auto k = compile(id_kernel);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> id_array(16*k.maxQPUs());
  for (int i = 0; i < id_array.size(); i++)
    id_array[i] = 0;

  SharedArray<int> index_array(16*k.maxQPUs());
  for (int i = 0; i < index_array.size(); i++)
    index_array[i] = 0;

  k.setNumQPUs(k.maxQPUs());        // Invoke the kernel
	k.load(&id_array, &index_array);  // Load the uniforms
	settings.process(k);

	// Display the result
  for (int i = 0; i < id_array.size(); i++) {
    printf("%3i: %2i, %2i\n", i, id_array[i], index_array[i]);
  }
  
  return 0;
}
