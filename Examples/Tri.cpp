#include "QPULib.h"
#include <CmdParameters.h>
#include "Support/Settings.h"

using namespace QPULib;

std::vector<const char *> const kernels = { "single", "multi", "float" };  // First is default

CmdParameters params = {
	"Tri - Calculate triangular numbers\n",
  {{
    "Kernel",
    "-k=",
		kernels,
    "Select the kernel to use"
	}},
	&Settings::params()
};


struct TriSettings : public Settings {
	int    kernel;

	int init(int argc, const char *argv[]) {
		set_name(argv[0]);
		CmdParameters &params = ::params;

		auto ret = params.handle_commandline(argc, argv, false);
		if (ret != CmdParameters::ALL_IS_WELL) return ret;

		// Init the parameters in the parent
		Settings::process(&params);

		kernel      = params.parameters()[0]->get_int_value();
		return ret;
	}


	void output() {
		printf("Settings:\n");
		printf("  kernel index: %d\n", kernel);
		printf("  kernel name : %s\n", kernels[kernel]);
		printf("\n");
	}
} settings;


///////////////////////////////////////////
// Kernels
///////////////////////////////////////////

// Define function that runs on the GPU.

void tri_single(Ptr<Int> p)
{
  Int n = *p;
  Int sum = 0;
  While (any(n > 0))
    Where (n > 0)
      sum = sum+n;
      n = n-1;
    End
  End
  *p = sum;
}


void tri_multi(Ptr<Int> p)
{
  p = p + (me() << 4);
  Int n = *p;
  Int sum = 0;
  While (any(n > 0))
    Where (n > 0)
      sum = sum+n;
      n = n-1;
    End
  End
  *p = sum;
}


void tri_float(Ptr<Float> p)
{
  Int n = toInt(*p);
  Int sum = 0;
  While (any(n > 0))
    Where (n > 0)
      sum = sum+n;
      n = n-1;
    End
  End
  *p = toFloat(sum);
}


///////////////////////////////////////////
// Local functions
///////////////////////////////////////////

void run_single() {
	printf("Running single kernel.\n");

  // Construct kernel
  auto k = compile(tri_single);
  //k.pretty("Tri.log");  // Output source and target code to file

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(16);
  for (int i = 0; i < 16; i++)
    array[i] = i;

  // Invoke the kernel
  settings.process(k, &array);

  // Display the result
  for (int i = 0; i < 16; i++)
    printf("%i: %i\n", i, array[i]);
}


void run_multi() {
	printf("Running multi kernel.\n");

  // Construct kernel
  auto k = compile(tri_multi);

  // Use 4 QPUs
  k.setNumQPUs(4);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(64);
  for (int i = 0; i < 64; i++)
    array[i] = i;

  // Invoke the kernel
  settings.process(k, &array);

  // Display the result
  for (int i = 0; i < 64; i++)
    printf("%i: %i\n", i, array[i]);
}


void run_float() {
	printf("Running float kernel.\n");

  // Construct kernel
  auto k = compile(tri_float);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<float> array(16);
  for (int i = 0; i < 16; i++)
    array[i] = (float) i;

  // Invoke the kernel
  settings.process(k, &array);

  // Display the result
  for (int i = 0; i < 16; i++)
    printf("%i: %f\n", i, array[i]);
}


///////////////////////////////////////////
// Main
///////////////////////////////////////////

int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

	switch (settings.kernel) {
		case 0: run_single(); break;
		case 1: run_multi(); break;
		case 2: run_float(); break;
	}
  
  return 0;
}
