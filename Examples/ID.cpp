#include <V3DLib.h>
#include "Support/Settings.h"

using namespace V3DLib;

V3DLib::Settings settings;

// Define function that runs on the GPU.

void id_kernel(Ptr<Int> p, Ptr<Int> q) {
  *p = me();
  *q = index();
}


int main(int argc, const char *argv[]) {
	int numQPUs = 8;

	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Construct kernel
  auto k = compile(id_kernel);
  k.setNumQPUs(numQPUs);  // Value is max for v3d

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> id_array(16*numQPUs);
	id_array.fill(0);

  SharedArray<int> index_array(16*numQPUs);
	index_array.fill(0);

	k.load(&id_array, &index_array);  // Load the uniforms
	settings.process(k);              // Invoke the kernel

	// Display the result
  for (int i = 0; i < (int) id_array.size(); i++) {
    printf("%3i: %2i, %2i\n", i, id_array[i], index_array[i]);
  }
  
  return 0;
}
