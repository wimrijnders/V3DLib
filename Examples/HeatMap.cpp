#include <V3DLib.h>
#include <stdio.h>
#include <stdlib.h>
#include "Support/Settings.h"
#include "Support/Timer.h"
#include "Support/pgm.h"
#include <CmdParameters.h>

using namespace V3DLib;
using std::string;

const float K = 0.25;   // Heat dissipation constant


CmdParameters params = {
  "HeatMap Example\n"
  "\n"
  "This example models the heat flow across a 2D surface.\n"
  "The output is a pgm-bitmap with the final state of the surface.\n"
  "\n"
  "The edges are set at zero temperature, and a number of hot points are placed randomly over the surface.\n"
  "The lower border can be broader than 1 pixel, depending on the number of QPU's running. This is due to\n"
  "preventing image overrun."
  "\n",
  {{
    "Kernel",
    "-k=",
    {"vector", "scalar"},  // First is default
    "Select the kernel to use"
  }, {
    "Number of steps",
    "-steps=",
    POSITIVE_INTEGER,
    "Set the number of steps to execute in the calculation",
    1500
  }, {
    "Number of points",
    "-points=",
    POSITIVE_INTEGER,
    "Set the number of randomly distributed hot points to start with",
    10
  }}
};


struct HeatMapSettings : public Settings {
  // Parameters
  const int WIDTH  = 512;           // Should be a multiple of 16 for QPU
  const int HEIGHT = 506;           // Should be a multiple of num_qpus for QPU
  const int SIZE   = WIDTH*HEIGHT;  // Size of 2D heat map

  int    kernel;
  string kernel_name;
  int    num_steps;
  int    num_points;

  HeatMapSettings() : Settings(&params, true) {}

  bool init_params() override {
    kernel      = params.parameters()["Kernel"]->get_int_value();
    kernel_name = params.parameters()["Kernel"]->get_string_value();
    num_steps   = params.parameters()["Number of steps"]->get_int_value();
    num_points  = params.parameters()["Number of points"]->get_int_value();
    return true;
  }
} settings;


// ============================================================================
// Local Helper functions
// ============================================================================

template<typename Arr>
void inject_hotspots(Arr &arr) {
  srand(0);

  for (int i = 0; i < settings.num_points; i++) {
    int t = rand() % 256;
    int x = 1 + rand() % (settings.WIDTH  - 2);
    int y = 1 + rand() % (settings.HEIGHT - 2);
    arr[y*settings.WIDTH + x] = (float) (1000*t);
  }
}


// ============================================================================
// Scalar version
// ============================================================================

/**
 * One time step for the scalar kernel
 */
void scalar_step(float** map, float** mapOut, int width, int height) {
  for (int y = 1; y < height-1; y++) {
    for (int x = 1; x < width-1; x++) {
      float surroundings =
        map[y-1][x-1] + map[y-1][x]   + map[y-1][x+1] +
        map[y][x-1]   +                 map[y][x+1]   +
        map[y+1][x-1] + map[y+1][x]   + map[y+1][x+1];
      surroundings *= 0.125f;
      mapOut[y][x] = (float) (map[y][x] - (K * (map[y][x] - surroundings)));
    }
  }
}


void run_scalar() {
  float* map       = new float [settings.SIZE];
  float* mapOut    = new float [settings.SIZE];
  float** map2D    = new float* [settings.HEIGHT];
  float** mapOut2D = new float* [settings.HEIGHT];

  // Initialise
  for (int i = 0; i < settings.SIZE; i++) {
    map[i] = mapOut[i] = 0.0;
  }

  for (int i = 0; i < settings.HEIGHT; i++) {
    map2D[i]    = &map[i*settings.WIDTH];
    mapOut2D[i] = &mapOut[i*settings.WIDTH];
  }

  inject_hotspots(map);

  // Simulate
  for (int i = 0; i < settings.num_steps; i++) {
    scalar_step(map2D, mapOut2D, settings.WIDTH, settings.HEIGHT);
    float** tmp = map2D; map2D = mapOut2D; mapOut2D = tmp;
  }

  // Display results
  output_pgm_file(mapOut, settings.WIDTH, settings.HEIGHT, 255, "heatmap.pgm");
}


// ============================================================================
// Vector version
// ============================================================================

struct Cursor {
  Ptr<Float> addr;
  Float prev, current, next;

  void init(Ptr<Float> p) {
    gather(p); comment("Cursor init");
    current = 0.0f;
    addr = p + 16;
  }

  void prime() {
    receive(next);
    gather(addr);
  }

  void advance() {
    addr.inc();     comment("Cursor advance");
    prev = current;
    gather(addr);
    current = next;
    receive(next);
  }

  void finish() {
    receive(next);
  }

  void shiftLeft(Float& result) {
    result = rotate(current, 15); comment("Cursor shiftLeft");
    Float nextRot = rotate(next, 15);
    Where (index() == 15)
      result = nextRot;
    End
  }

  void shiftRight(Float& result) {
    result = rotate(current, 1); comment("Cursor shiftRight");
    Float prevRot = rotate(prev, 1);
    Where (index() == 0)
      result = prevRot;
    End
  }
};


/**
 * Performs a single step for the heat transfer
 */
void heatmap_kernel(Ptr<Float> map, Ptr<Float> mapOut, Int height, Int width) {
  Cursor row[3];

  For (Int y = 1, y < height - 1 - numQPUs(), y = y + numQPUs())
    // Point p to the in- and output row
    Ptr<Float> p_in = map    + (y + me())*width;
    Ptr<Float> p    = mapOut + (y + me())*width;

    // Initialize three cursors for the three input rows
    for (int i = 0; i < 3; i++) row[i].init(p_in + (i - 1)*width);
    for (int i = 0; i < 3; i++) row[i].prime();

    // Compute one output row
    For (Int x = 0, x < width, x = x + 16)
      for (int i = 0; i < 3; i++) row[i].advance();

      Float left[3], right[3];
      for (int i = 0; i < 3; i++) {
        row[i].shiftLeft(right[i]);
        row[i].shiftRight(left[i]);
      }

      Float sum = left[0] + row[0].current + right[0] +
                  left[1] +                  right[1] +
                  left[2] + row[2].current + right[2];

       Float output = row[1].current - K * (row[1].current - sum * 0.125);

      // Ensure left and right borders are zero
      Int actual_x = x + index();
      Where (actual_x == 0)
        output = 0.0f;
      End
      Where (actual_x == width - 1)
        output = 0.0f;
      End

       *p = output;
      p.inc();
    End

    // Cursors are finished for this row
    for (int i = 0; i < 3; i++) row[i].finish();
  End
}


/**
 * The edges always have zero values.
 * i.e. there is constant cold at the edges.
 */
void run_kernel() {
  // Allocate and initialise input and output maps
  SharedArray<float> mapA(settings.SIZE);
  SharedArray<float> mapB(settings.SIZE);
  mapA.fill(0.0f);
  mapB.fill(0.0f);

  // Inject hot spots
  inject_hotspots(mapA);

  // Compile kernel
  auto k = compile(heatmap_kernel);
  k.setNumQPUs(settings.num_qpus);

  for (int i = 0; i < settings.num_steps; i++) {
    if (i & 1) {
      k.load(&mapB, &mapA, settings.HEIGHT, settings.WIDTH);  // Load the uniforms
    } else {
      k.load(&mapA, &mapB, settings.HEIGHT, settings.WIDTH);  // Load the uniforms
    }

    // Invoke the kernel
    settings.process(k);
  }

  // Output results
  output_pgm_file(mapB, settings.WIDTH, settings.HEIGHT, 255, "heatmap.pgm");
}


// ============================================================================
// Main
// ============================================================================

int main(int argc, const char *argv[]) {
  auto ret = settings.init(argc, argv);
  if (ret != CmdParameters::ALL_IS_WELL) return ret;

  Timer timer;

  switch (settings.kernel) {
    case 0: run_kernel();  break;  
    case 1: run_scalar(); break;
  }

  if (!settings.silent) {
    printf("Ran kernel '%s' with %d QPU's\n", settings.kernel_name.c_str(), settings.num_qpus);
  }
  timer.end(!settings.silent);

  return 0;
}
