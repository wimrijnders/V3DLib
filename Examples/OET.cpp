#include "V3DLib.h"
#include "Support/Settings.h"

using namespace V3DLib;

V3DLib::Settings settings;

/**
 * Odd/even transposition sorter for a 32-element array
 */
void kernel(Int::Ptr p) {
  Int evens = *p;
  Int odds  = *(p+16);

  For (Int count = 0, count < 16, count++)
    Int evens2 = min(evens, odds);
    Int odds2  = max(evens, odds);

    Int evens3 = rotate(evens2, 15);
    Int odds3  = odds2;
    Where (index() != 15)
      odds2 = min(evens3, odds3);
    End

    Where (index() != 0)
      evens2 = rotate(max(evens3, odds3), 1);
    End

    evens = evens2;
    odds  = odds2;
  End

  *p      = evens;
  *(p+16) = odds;
}


int main(int argc, const char *argv[]) {
  settings.init(argc, argv);

  auto k = compile(kernel);                       // Construct kernel

  Int::Array a(32);                               // Allocate and initialise array shared between ARM and GPU
  for (int i = 0; i < (int) a.size(); i++)
    a[i] = 100 - i;

  k.load(&a);                                     // Load the uniforms
  settings.process(k);                            // Invoke the kernel

  for (int i = 0; i < (int) a.size(); i++)        // Display the result
    printf("%i: %i\n", i, (i & 1) ? a[16+(i>>1)] : a[i>>1]);
  
  return 0;
}
