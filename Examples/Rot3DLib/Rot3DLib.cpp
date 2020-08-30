#include <sys/time.h>
#include <math.h>
#include "../Support/Settings.h"
#include "Rot3DKernels.h"

using namespace Rot3DLib;
using Settings = QPULib::Settings;
using Generator = decltype(rot3D_1);  // All kernel functions except scalar have same prototype


// ============================================================================
// Command line handling
// ============================================================================

std::vector<const char *> const kernels = { "3", "2", "1", "cpu" };  // First is default

CmdParameters params = {
  "Rot3DLib\n",
  {{
    "Kernel",
    "-k=",
		kernels,
    "Select the kernel to use"
  }},
	&Settings::params()
};


struct Rot3DLibSettings : public Settings {
	int    kernel;

	int init(int argc, const char *argv[]) {
		set_name(argv[0]);

		CmdParameters &params = ::params;

		auto ret = params.handle_commandline(argc, argv, false);
		if (ret != CmdParameters::ALL_IS_WELL) return ret;

		// Init the parameters in the parent
		Settings::process(&params);

		kernel              = params.parameters()["Kernel"]->get_int_value();

		return ret;
	}
} settings;


// ============================================================================
// Kernels
// ============================================================================

// Number of vertices and angle of rotation
const int N = 19200; // 192000
const float THETA = (float) 3.14159;


timeval runScalar() {
	printf("Running scalar\n");

  // Allocate and initialise
  float* x = new float [N];
  float* y = new float [N];
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }

  timeval tvStart, tvEnd, tvDiff;
  gettimeofday(&tvStart, NULL);

  rot3D(N, cosf(THETA), sinf(THETA), x, y);

  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  // Display results
  //for (int i = 0; i < N; i++)
  //  printf("%f %f\n", x[i], y[i]);

	delete x;
	delete y;

	return tvDiff;
}


timeval run_qpu_kernel(int index) {
	Generator *kGenerator = nullptr;

	switch (index) {
	  case 0: kGenerator = rot3D_3; break;
	  case 1: kGenerator = rot3D_2; break;
	  case 2: kGenerator = rot3D_1; break;
		default: {
			char buf[64];
			sprintf(buf, "ERROR: No kernel with index %d", index);
			fatal(buf);
		}
  };

  auto k = compile(kGenerator);  // Construct kernel
  k.setNumQPUs(k.maxQPUs());     // Use all available QPUs

  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<float> x(N), y(N);
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }

  timeval tvStart, tvEnd, tvDiff;
  gettimeofday(&tvStart, NULL);

	settings.process(k, N, cosf(THETA), sinf(THETA), &x, &y);

  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  // Display results
  //for (int i = 0; i < N; i++)
  //  printf("%f %f\n", x[i], y[i]);

	return tvDiff;
}


/**
 * Run a kernel as specified by the passed kernel index
 */
void run_kernel(int kernel_index) {
  timeval tvDiff;

	if (kernel_index == 3) {
		tvDiff = runScalar();
  } else {
		tvDiff = run_qpu_kernel(kernel_index);
  }

	auto name = kernels[kernel_index];

	printf("Ran kernel '%s'.\n", name);
  printf("%ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);
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
