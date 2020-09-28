#include "QPULib.h"
#include "Support/Settings.h"

using namespace QPULib;

QPULib::Settings settings;

// Define function that runs on the GPU.

void kernel(Ptr<Int> p) {
  Int x, y;

/*
	// For a block of 16 values, following is the same as:
	x = *p;
	y = *(p + 16);
*/
  gather(p+index());
  gather(p+16+index());
  receive(x);
  receive(y);

  *p = x + y;
}


int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Construct kernel
  auto k = compile(kernel);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(32);
  for (int i = 0; i < array.size(); i++)
    array[i] = i;

  // Invoke the kernel
  //k.setNumQPUs(8);  // TODO: Examine if this example works with > 1 QPUs
  k.load(&array);
  settings.process(k);

	// Display the result
  for (int i = 0; i < 16; i++)
    printf("%i: %i\n", i, array[i]);
  
  return 0;
}
