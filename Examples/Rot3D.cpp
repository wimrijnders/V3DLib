#include <sys/time.h>
#include <unistd.h>  // sleep()
#include <math.h>
#include <QPULib.h>
#include "Support/Settings.h"
#include "Support/debug.h"
#include "Rot3DLib/Rot3DKernels.h"

using namespace QPULib;
using namespace Rot3DLib;
using KernelType = decltype(rot3D_1);  // All kernel functions except scalar have same prototype

// Number of vertices and angle of rotation
const int N = 192000; // 192000
const float THETA = (float) 3.14159;


// ============================================================================
// Command line handling
// ============================================================================

std::vector<const char *> const kernels = { "3", "2", "1", "cpu" };  // First is default


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

		//printf("Num QPU's in settings: %d\n", num_qpus);
		return ret;
	}
} settings;



// ============================================================================
// Local functions
// ============================================================================



/**
 * TODO: Consolidate with Mandelbrot
 */
void end_timer(timeval tvStart) {
  timeval tvEnd, tvDiff;
  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  printf("Run time: %ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);
}


void run_qpu_kernel(KernelType &kernel) {
  timeval tvStart;
  gettimeofday(&tvStart, NULL);

  auto k = compile(kernel);  // Construct kernel
  k.setNumQPUs(settings.num_qpus);

  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<float> x(N), y(N);
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }

  k.load(N, cosf(THETA), sinf(THETA), &x, &y);
  settings.process(k);

	end_timer(tvStart);

	if (settings.show_results) {
  	for (int i = 0; i < N; i++)
  		printf("%f %f\n", x[i], y[i]);
	}
}


void run_scalar_kernel() {
  timeval tvStart;
  gettimeofday(&tvStart, NULL);

  // Allocate and initialise
  float* x = new float [N];
  float* y = new float [N];
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }

	if (settings.compile_only) {
	  rot3D(N, cosf(THETA), sinf(THETA), x, y);
	}

	end_timer(tvStart);

	if (settings.show_results) {
  	for (int i = 0; i < N; i++)
  		printf("%f %f\n", x[i], y[i]);
	}
}


/**
 * Run a kernel as specified by the passed kernel index
 */
void run_kernel(int kernel_index) {
	switch (kernel_index) {
		case 0: run_qpu_kernel(rot3D_3);  break;	
		case 1: run_qpu_kernel(rot3D_2);  break;	
		case 2: run_qpu_kernel(rot3D_1);  break;	
		case 3: run_scalar_kernel(); break;
	}

	auto name = kernels[kernel_index];

	printf("Ran kernel '%s' with %d QPU's.\n", name, settings.num_qpus);
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
