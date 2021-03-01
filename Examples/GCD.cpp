#include <stdlib.h>
#include "V3DLib.h"
#include "Support/Settings.h"

using namespace V3DLib;

V3DLib::Settings settings;

void gcd(Int::Ptr p, Int::Ptr q, Int::Ptr r) {
  Int a = *p;
  Int b = *q;

  While (any(a != b))
    Where (a > b)
      a = a-b;
    End
    Where (a < b)
      b = b-a;
    End
  End

  *r = a;
}


int main(int argc, const char *argv[]) {
  auto ret = settings.init(argc, argv);
  if (ret != CmdParameters::ALL_IS_WELL) return ret;

  auto k = compile(gcd);                 // Construct the kernel

  SharedArray<int> a(16), b(16), r(16);  // Allocate and initialise the arrays shared between ARM and GPU
  srand(0);
  for (int i = 0; i < 16; i++) {
    a[i] = 100 + (rand() % 100);
    b[i] = 100 + (rand() % 100);
  }

  k.load(&a, &b, &r);                    // Invoke the kernel
  settings.process(k);

  for (int i = 0; i < 16; i++)           // Display the result
    printf("gcd(%i, %i) = %i\n", a[i], b[i], r[i]);
  
  return 0;
}
