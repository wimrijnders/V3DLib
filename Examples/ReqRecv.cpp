#include "V3DLib.h"
#include "Support/Settings.h"

using namespace V3DLib;

V3DLib::Settings settings;

void kernel(Int::Ptr p) {
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

  // Construct kernel
  auto k = compile(kernel);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(2*16);
  for (int i = 0; i < (int) array.size(); i++)
    array[i] = i;

  // Invoke the kernel
  k.load(&array);
  settings.process(k);

  // Display the result
  for (int i = 0; i < (int) array.size()/2; i++)
    printf("%i: %i\n", i, array[i]);
  
  return 0;
}
