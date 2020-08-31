#include <stdlib.h>
#include "QPULib.h"
#include "Support/Settings.h"

using namespace QPULib;

QPULib::Settings settings;

void gcd(Ptr<Int> p, Ptr<Int> q, Ptr<Int> r) {
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

  // Construct kernel
  auto k = compile(gcd);

  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<int> a(16), b(16), r(16);
  srand(0);
  for (int i = 0; i < 16; i++) {
    a[i] = 100 + (rand() % 100);
    b[i] = 100 + (rand() % 100);
  }

  // Invoke the kernel
	k.load(&a, &b, &r);
	settings.process(k);

	// Display the result
  for (int i = 0; i < 16; i++)
    printf("gcd(%i, %i) = %i\n", a[i], b[i], r[i]);
  
  return 0;
}
