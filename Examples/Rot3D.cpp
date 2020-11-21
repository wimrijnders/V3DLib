#include <unistd.h>  // sleep()
#include <math.h>
#include <V3DLib.h>
#include "Support/Settings.h"
#include "Support/Timer.h"
#include "Support/debug.h"
#include "Rot3DLib/Rot3DKernels.h"

using namespace V3DLib;
using namespace Rot3DLib;
using KernelType = decltype(rot3D_1);  // All kernel functions except scalar have same prototype

// Number of vertices and angle of rotation
const int SIZE = 192000;
const float THETA = (float) 3.14159;


// ============================================================================
// Command line handling
// ============================================================================

std::vector<const char *> const kernels = { "2", "1", "cpu" };  // First is default


CmdParameters params = {
  "Rot3D\n",
  {{
    "Kernel",
    "-k=",
		kernels,
    "Select the kernel to use"
	}, {
    "Display Results",
    "-d",
		ParamType::NONE,
    "Show the results of the calculations"
  }}
};


struct Rot3DSettings : public Settings {
	int    kernel;
	bool   show_results;

	int init(int argc, const char *argv[]) {
		auto const SUCCESS = CmdParameters::ALL_IS_WELL;
		auto const FAIL    = CmdParameters::EXIT_ERROR;

		set_name(argv[0]);
		CmdParameters &params = ::params;
		params.add(base_params(true));

		auto ret = params.handle_commandline(argc, argv, false);
		if (ret != SUCCESS) return ret;

		// Init the parameters in the parent
		if (!process(&params, true)) {
			ret = FAIL;
		}

		kernel              = params.parameters()["Kernel"]->get_int_value();
		//kernel_name       = params.parameters()["Kernel"]->get_string_value();
		show_results        = params.parameters()["Display Results"]->get_bool_value();

		return ret;
	}
} settings;



// ============================================================================
// Local functions
// ============================================================================

template<typename Arr>
void init_arrays(Arr &x, Arr &y) {
  for (int i = 0; i < SIZE; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }
}


template<typename Arr>
void disp_arrays(Arr &x, Arr &y) {
	if (settings.show_results) {
  	for (int i = 0; i < SIZE; i++)
  		printf("%f %f\n", x[i], y[i]);
	}
}


void run_qpu_kernel(KernelType &kernel) {
	Timer timer;

  auto k = compile(kernel);  // Construct kernel
  k.setNumQPUs(settings.num_qpus);

  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<float> x(SIZE), y(SIZE);
	init_arrays(x, y);

  k.load(SIZE, cosf(THETA), sinf(THETA), &x, &y);
  settings.process(k);

	disp_arrays(x, y);
	timer.end(!settings.silent);
}


void run_scalar_kernel() {
	Timer timer;

  // Allocate and initialise
  float* x = new float[SIZE];
  float* y = new float[SIZE];
	init_arrays(x, y);

	if (!settings.compile_only) {
	  rot3D(SIZE, cosf(THETA), sinf(THETA), x, y);
	}

	disp_arrays(x, y);
	timer.end(!settings.silent);
	delete [] x;
	delete [] y;
}


/**
 * Run a kernel as specified by the passed kernel index
 */
void run_kernel(int kernel_index) {
	switch (kernel_index) {
		case 0: run_qpu_kernel(rot3D_2);  break;	
		case 1: run_qpu_kernel(rot3D_1);  break;	
		case 2: run_scalar_kernel(); break;
	}

	auto name = kernels[kernel_index];

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
