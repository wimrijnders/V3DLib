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

std::vector<const char *> const kernel_id = { "qpu", "cpu" };  // First is default


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
	static int const N    = 5;
	static int const DIM  = 16*N;
	static int const SIZE = DIM*DIM;

	int    kernel;

	MatrixSettings() : Settings(&params, true) {}

	void init_params() override {
		kernel = params.parameters()["Kernel"]->get_int_value();
	}
} settings;



// ============================================================================
// Local functions
// ============================================================================

void run_qpu_kernel() {
  auto k = compile(kernels::matrix_mult<MatrixSettings::N>);  // Construct kernel
  k.setNumQPUs(settings.num_qpus);

  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<float> a(MatrixSettings::SIZE);
  SharedArray<float> b(MatrixSettings::SIZE);
  SharedArray<float> result(MatrixSettings::SIZE);

  for (int i = 0; i < MatrixSettings::SIZE; i++) {
    a[i] = random_float();
    b[i] = random_float();
  }

  k.load(&result, &a, &b);
  settings.process(k);
}


void run_scalar_kernel() {
  // Allocate and initialise
  float a[MatrixSettings::SIZE];
  float b[MatrixSettings::SIZE];
  float result[MatrixSettings::SIZE];

  for (int i = 0; i < MatrixSettings::SIZE; i++) {
    a[i] = random_float();
    b[i] = random_float();
  }

	if (!settings.compile_only) {
		kernels::matrix_mult_scalar(settings.N, result, a, b);
	}
}


/**
 * Run a kernel as specified by the passed kernel index
 */
void run_kernel() {
	Timer timer;

	switch (settings.kernel) {
		case 0: run_qpu_kernel();    break;	
		case 1: run_scalar_kernel(); break;
	}

	auto name = kernel_id[settings.kernel];

	timer.end(!settings.silent);

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

	run_kernel();

  return 0;
}
