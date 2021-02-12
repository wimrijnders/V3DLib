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
  "Matrix Multiplication\n\n"
  "Calculates the multiplication of two square matrices\n",
  {{
    "Kernel",
    "-k=",
    kernel_id,
    "Select the kernel to use\n"
  },{
    "Read method",
    "-read=",
    { "default", "prefetch", "none"}, 
    "The way to retrieve data from memory. "
    "Option 'none' skips all reads and writes, the rest have only effect on reads.\n"
  },{
    "Matrix dimension",
    { "-d=","-dimension="},
    ParamType::POSITIVE_INTEGER,
    "The number of matrix elements in a row/column. "
    "Must be a multiple of 16",
    48
  },{
    "Number of repeats",
    { "-p=","-repeat="},
    ParamType::POSITIVE_INTEGER,
    "The number times to execute the matrix multiplication",
    1
  }}
};


struct MatrixSettings : public Settings {
  int kernel;
  int dimension;
  int repeats;
  MatrixReadMethod read_method;

  int size() const { return dimension*dimension; }

  MatrixSettings() : Settings(&params, true) {}

  bool init_params() override {
    kernel      = params.parameters()["Kernel"           ]->get_int_value();
    dimension   = params.parameters()["Matrix dimension" ]->get_int_value();
    repeats     = params.parameters()["Number of repeats"]->get_int_value();

    int in_read_method = params.parameters()["Read method"]->get_int_value();

    switch(in_read_method) {
      case 0: read_method = DEFAULT;      break;
      case 1: read_method = DO_PREFETCH;  break;
      case 2: read_method = NO_READWRITE; break;

      default: assertq(false, "Unknown read method"); return false;
    }

    return true;
  }
} settings;



// ============================================================================
// Local functions
// ============================================================================

void run_scalar_kernel() {
  if (settings.compile_only) return;
 
  // Allocate and initialise
  float *a      = new float [settings.size()];
  float *b      = new float [settings.size()];
  float *result = new float [settings.size()];

  for (int i = 0; i < settings.size(); i++) {
    a[i] = random_float();
    b[i] = random_float();
  }

  Timer timer;
  for (int i = 0; i < settings.repeats; ++i) {
    kernels::matrix_mult_scalar(settings.dimension, result, a, b);
  }
  timer.end(!settings.silent);

  delete [] a;
  delete [] b;
  delete [] result;
}


void run_qpu_kernel() {
  auto k = compile(kernels::matrix_mult_decorator(settings.dimension, settings.read_method));  // Construct kernel
  k.setNumQPUs(settings.num_qpus);


  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<float> a(settings.size());
  SharedArray<float> b(settings.size());
  SharedArray<float> result(settings.size());

  for (int i = 0; i < settings.size(); i++) {
    a[i] = random_float();
    b[i] = random_float();
  }

  Timer timer;
  k.load(&result, &a, &b);
  for (int i = 0; i < settings.repeats; ++i) {
    settings.process(k);
  }
  timer.end(!settings.silent);
}


// ============================================================================
// Main
// ============================================================================

int main(int argc, const char *argv[]) {
  auto ret = settings.init(argc, argv);
  if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Run a kernel as specified by the passed kernel index
  switch (settings.kernel) {
    case 0: run_qpu_kernel();     break;  
    case 1: run_scalar_kernel();  break;
    default: assert(false);       break;
  }

  if (!settings.silent) {
    auto name = kernel_id[settings.kernel];
    printf("Ran kernel '%s' %d time(s) with matrix size %d and %d QPU's.\n",
           name, settings.repeats, settings.dimension, settings.num_qpus);
  }

  return 0;
}
