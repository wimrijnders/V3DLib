#include <unistd.h>  // sleep()
#include <math.h>
#include <V3DLib.h>
#include "Support/Settings.h"
#include "Support/Timer.h"
#include "Support/debug.h"
#include "Kernels/Matrix.h"

using namespace V3DLib;
using namespace kernels;


// ============================================================================
// Command line handling
// ============================================================================

std::vector<const char *> const kernel_id = { "cpu" };  // First is default


CmdParameters params = {
  "Matrix Multiplication\n",
  {{
    "Kernel",
    "-k=",
		kernel_id,
    "Select the kernel to use"
  }}
};


struct MatrixSettings : public Settings {
	int const N = 1;

	int    kernel;

	MatrixSettings() : Settings(&params, true) {}

	void init_params() override {
		kernel              = params.parameters()["Kernel"]->get_int_value();
	}
} settings;



// ============================================================================
// Local functions
// ============================================================================

/*
void run_qpu_kernel(KernelType &kernel) {
	Timer timer;

  auto k = compile(kernel);  // Construct kernel
  k.setNumQPUs(settings.num_qpus);

  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<float> x(SIZE), y(SIZE);
	init_arrays(x, y);

  k.load(SIZE, cosf(THETA), sinf(THETA), &x, &y);
  settings.process(k);

	timer.end(!settings.silent);
}
*/


void run_scalar_kernel() {
	int const DIM  = 16*settings.N;
	int const SIZE = DIM*DIM;

	Timer timer;

  // Allocate and initialise
  float a[SIZE];
  float b[SIZE];
  float result[SIZE];

  for (int i = 0; i < SIZE; i++) {
    a[i] = random_float();
    b[i] = random_float();
  }

	if (!settings.compile_only) {
		kernels::matrix_mult_scalar(settings.N, result, a, b);
	}

	timer.end(!settings.silent);
}


/**
 * Run a kernel as specified by the passed kernel index
 */
void run_kernel(int kernel_index) {
	switch (kernel_index) {
		case 0: run_scalar_kernel(); break;
		//case 1: run_qpu_kernel(rot3D_2);  break;	
		//case 2: run_qpu_kernel(rot3D_1);  break;	
	}

	auto name = kernel_id[kernel_index];

	if (!settings.silent) {
		printf("Ran kernel '%s' with %d QPU's.\n", name, settings.num_qpus);
	}
}


// ============================================================================
// Main
// ============================================================================

int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

	run_kernel(settings.kernel);

  return 0;
}
