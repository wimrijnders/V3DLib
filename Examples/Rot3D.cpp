#include <unistd.h>  // sleep()
#include <math.h>
#include <V3DLib.h>
#include "Support/Settings.h"
#include "Support/Timer.h"
#include "Support/debug.h"
#include "Kernels/Rot3D.h"

using namespace V3DLib;
using namespace kernels;
using KernelType = decltype(rot3D_1);  // All kernel functions except scalar have same prototype


// ============================================================================
// Command line handling
// ============================================================================

std::vector<const char *> const kernel_id = { "2", "3", "1", "1a", "cpu" };  // First is default

CmdParameters params = {
  "Rot3D\n"
  "\n"
  "Rotates a number of vectors by a given angle.\n"
  "The kernel is IO-bound, the data transfer time dominates over the calculation during execution.\n"
  "It is therefore an indicator of data transfer speed, "
  "and is used for performance comparisons of platforms and configurations.\n",
  {{
    "Kernel",
    "-k=",
    kernel_id,
    "Select the kernel to use"
  }, {
    "Display Results",
    "-d",
    ParamType::NONE,
    "Show the results of the calculations"
  }, {
    "Number of vertices",
    {"-v=", "-vertices="},
    ParamType::POSITIVE_INTEGER,
    "Number of vertices to rotate",
    1920  // was 192000
  }}
};


struct Rot3DSettings : public Settings {
  const float THETA = (float) 3.14159;  // Rotation angle in radians

  int  kernel;
  bool show_results;
  int  num_vertices;

  Rot3DSettings() : Settings(&params, true) {}

  bool init_params() override {
    auto const &p = parameters();

    kernel       = p["Kernel"]->get_int_value();
    show_results = p["Display Results"]->get_bool_value();
    num_vertices = p["Number of vertices"]->get_int_value();

    if (num_vertices % 16 != 0) {
      printf("ERROR: Number of vertices must be a multiple of 16.\n");
      return false;
    }

    return true;
  }
} settings;



// ============================================================================
// Local functions
// ============================================================================

template<typename Arr>
void init_arrays(Arr &x, Arr &y) {
  for (int i = 0; i < settings.num_vertices; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }
}


template<typename Arr>
void disp_arrays(Arr &x, Arr &y) {
  if (settings.show_results) {
    for (int i = 0; i < settings.num_vertices; i++)
      printf("%f %f\n", x[i], y[i]);
  }
}


void run_scalar_kernel() {
  // Allocate and initialise
  float* x = new float[settings.num_vertices];
  float* y = new float[settings.num_vertices];
  init_arrays(x, y);

  if (!settings.compile_only) {
    Timer timer;  // Time the run only
    rot3D(settings.num_vertices, cosf(settings.THETA), sinf(settings.THETA), x, y);
    timer.end(!settings.silent);
  }

  disp_arrays(x, y);
  delete [] x;
  delete [] y;
}


void run_qpu_kernel(KernelType &kernel) {
  auto k = compile(kernel);  // Construct kernel
  k.setNumQPUs(settings.num_qpus);

  // Allocate and initialise arrays shared between ARM and GPU
  Float::Array x(settings.num_vertices), y(settings.num_vertices);
  init_arrays(x, y);

  k.load(settings.num_vertices, cosf(settings.THETA), sinf(settings.THETA), &x, &y);

  Timer timer;  // Time the run only
  settings.process(k);
  timer.end(!settings.silent);

  disp_arrays(x, y);
}


void run_qpu_kernel_3() {
  auto k = compile(rot3D_3_decorator(settings.num_vertices, settings.num_qpus));
  k.setNumQPUs(settings.num_qpus);

  // Allocate and initialise arrays shared between ARM and GPU
  Float::Array x(settings.num_vertices), y(settings.num_vertices);
  init_arrays(x, y);

  k.load(cosf(settings.THETA), sinf(settings.THETA), &x, &y);

  Timer timer;  // Time the run only
  settings.process(k);
  timer.end(!settings.silent);

  disp_arrays(x, y);
}


/**
 * Run a kernel as specified by the passed kernel index
 */
void run_kernel(int kernel_index) {
  switch (kernel_index) {
    case 0: run_qpu_kernel(rot3D_2);  break;  
    case 1: run_qpu_kernel_3();       break;  
    case 2: run_qpu_kernel(rot3D_1);  break;  
    case 3: run_qpu_kernel(rot3D_1a); break;  
    case 4: run_scalar_kernel();      break;
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
  settings.init(argc, argv);

  run_kernel(settings.kernel);
  return 0;
}
