#include <sys/time.h>
#include <math.h>
#include <QPULib.h>
#include <CmdParameters.h>

using namespace QPULib;

// #define USE_SCALAR_VERSION


// ============================================================================
// Command line handling
// ============================================================================

std::vector<const char *> const kernels = { "3", "2", "1", "cpu" };  // Order important! First is default


CmdParameters params = {
  "Rot3D\n",
  {{
    "Kernel",
    "-k=",
		kernels,
    "Select the kernel to use"
	}, {
    "Num QPU's",
    "-n=",
		INTEGER,
    "Number of QPU's to use. Must be a value between 1 an 12 inclusive (TODO: not enforced yet)",
		12
	}, {
    "Display Results",
    "-d",
		ParamType::NONE,   // Prefix needed to disambiguate
    "Show the results of the calculations",
		12
  }}
};


struct Settings {
	int    kernel;
	int    num_qpus;
	bool   show_results;

	int init(int argc, const char *argv[]) {
		auto ret = params.handle_commandline(argc, argv, false);
		if (ret != CmdParameters::ALL_IS_WELL) return ret;

		kernel        = params.parameters()[0]->get_int_value();
		//kernel_name = params.parameters()[0]->get_string_value();
		num_qpus      = params.parameters()[1]->get_int_value();
		show_results  = params.parameters()[2]->get_bool_value();

		return ret;
	}
} settings;


// ============================================================================
// Kernels
// ============================================================================

/**
 * Scalar versionS
 */
void rot3D(int n, float cosTheta, float sinTheta, float* x, float* y)
{
  for (int i = 0; i < n; i++) {
    float xOld = x[i];
    float yOld = y[i];
    x[i] = xOld * cosTheta - yOld * sinTheta;
    y[i] = yOld * cosTheta + xOld * sinTheta;
  }
}


/**
 * Vector version 1
 */
void rot3D_1(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y)
{
  For (Int i = 0, i < n, i = i+16)
    Float xOld = x[i];
    Float yOld = y[i];
    x[i] = xOld * cosTheta - yOld * sinTheta;
    y[i] = yOld * cosTheta + xOld * sinTheta;
  End
}


/**
 * Vector version 2
 */
void rot3D_2(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y)
{
  Int inc = 16;
  Ptr<Float> p = x + index();
  Ptr<Float> q = y + index();
  gather(p); gather(q);
 
  Float xOld, yOld;
  For (Int i = 0, i < n, i = i+inc)
    gather(p+inc); gather(q+inc); 
    receive(xOld); receive(yOld);
    store(xOld * cosTheta - yOld * sinTheta, p);
     store(yOld * cosTheta + xOld * sinTheta, q);
    p = p+inc; q = q+inc;
  End

  receive(xOld); receive(yOld);
}


/**
 * Vector version 3
 */
void rot3D_3(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y)
{
  Int inc = numQPUs() << 4;
  Ptr<Float> p = x + index() + (me() << 4);
  Ptr<Float> q = y + index() + (me() << 4);
  gather(p); gather(q);
 
  Float xOld, yOld;
  For (Int i = 0, i < n, i = i+inc)
    gather(p+inc); gather(q+inc); 
    receive(xOld); receive(yOld);
    store(xOld * cosTheta - yOld * sinTheta, p);
    store(yOld * cosTheta + xOld * sinTheta, q);
    p = p+inc; q = q+inc;
  End

  receive(xOld); receive(yOld);
}

using KernelType = decltype(rot3D_3);


// ============================================================================
// Local functions
// ============================================================================

// Number of vertices and angle of rotation
const int N = 192000; // 192000
const float THETA = (float) 3.14159;


/**
 * TODO: Consolidate with Mandelbrot
 */
void end_timer(timeval tvStart) {
  timeval tvEnd, tvDiff;
  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  printf("%ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);
}


void run_qpu_kernel(KernelType &kernel) {
  auto k = compile(rot3D_3);  // Construct kernel

  k.setNumQPUs(settings.num_qpus);

  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<float> x(N), y(N);
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }

  k(N, cosf(THETA), sinf(THETA), &x, &y);

	if (settings.show_results) {
  	for (int i = 0; i < N; i++)
  		printf("%f %f\n", x[i], y[i]);
	}
}


void run_scalar_kernel() {
  // Allocate and initialise
  float* x = new float [N];
  float* y = new float [N];
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }

  rot3D(N, cosf(THETA), sinf(THETA), x, y);

	if (settings.show_results) {
  	for (int i = 0; i < N; i++)
  		printf("%f %f\n", x[i], y[i]);
	}
}


/**
 * Run a kernel as specified by the passed kernel index
 */
void run_kernel(int kernel_index) {
  timeval tvStart;
  gettimeofday(&tvStart, NULL);

	switch (kernel_index) {
		case 0: run_qpu_kernel(rot3D_3);  break;	
		case 1: run_qpu_kernel(rot3D_2);  break;	
		case 2: run_qpu_kernel(rot3D_1);  break;	
		case 3: run_scalar_kernel(); break;
	}

	auto name = kernels[kernel_index];

	printf("Ran kernel '%s' with %d QPU's in ", name, settings.num_qpus);
	end_timer(tvStart);
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
