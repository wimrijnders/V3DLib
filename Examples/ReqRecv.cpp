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
  settings.init(argc, argv);

  auto k = compile(kernel);                       // Construct kernel

  Int::Array array(2*16);                         // Allocate and initialise array shared between ARM and GPU
  for (int i = 0; i < (int) array.size(); i++)
    array[i] = i;

  k.load(&array);                                 // Invoke the kernel
  settings.process(k);

  for (int i = 0; i < (int) array.size()/2; i++)  // Display the result
    printf("%i: %i\n", i, array[i]);
  
  return 0;
}
