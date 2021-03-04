#include "V3DLib.h"
#include "Support/Settings.h"

using namespace V3DLib;

V3DLib::Settings settings;


void hello(Int::Ptr p) {                          // The kernel definition
  *p = 1;
}


int main(int argc, const char *argv[]) {
  auto ret = settings.init(argc, argv);
  if (ret != CmdParameters::ALL_IS_WELL) return ret;

  auto k = compile(hello);                        // Construct the kernel

  Int::Array array(16);                           // Allocate and initialise the array shared between ARM and GPU
  array.fill(100);

  k.load(&array);                                 // Invoke the kernel
  settings.process(k);  

  for (int i = 0; i < (int) array.size(); i++) {  // Display the result
    printf("%i: %i\n", i, array[i]);
  }

  return 0;
}
