#include "V3DLib.h"
#include "Support/Settings.h"

using namespace V3DLib;

V3DLib::Settings settings;

// Define function that runs on the GPU.

void kernel(Ptr<Int> p) {
  Int x, y;

  gather(p);
  gather(p+16);
  receive(x);
  receive(y);

  *p = x + y;
}


int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

	int numQpus = 1;

  // Construct kernel
  auto k = compile(kernel);
  k.setNumQPUs(numQpus);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(numQpus*16 + 16);
  for (int i = 0; i < (int) array.size(); i++)
    array[i] = i;

  // Invoke the kernel
  k.load(&array);
  settings.process(k);

	// Display the result
  for (int i = 0; i < numQpus*16; i++)
    printf("%i: %i\n", i, array[i]);
  
  return 0;
}
