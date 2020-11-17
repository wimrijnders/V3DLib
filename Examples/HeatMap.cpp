#include <QPULib.h>
#include <stdio.h>
#include <stdlib.h>
#include "Support/Settings.h"
#include "Support/Timer.h"
#include "Support/pgm.h"
#include <CmdParameters.h>

using namespace QPULib;
using std::string;

const float K = 0.25;   // Heat dissipation constant

std::vector<const char *> const kernels = { "vector", "scalar" };  // Order important! First is default

CmdParameters params = {
  "Heatmap\n",
  {{
    "Kernel",
    "-k=",
		kernels,
    "Select the kernel to use"
	}, {
    "Number of steps",
    "-steps=",
		POSITIVE_INTEGER,
    "Set the number of steps to execute in the calculation",
		1500
  }}
};


struct HeatMapSettings : public Settings {
	// Parameters
  const int WIDTH  = 512;           // Should be a multiple of 16 for QPU
  const int HEIGHT = 506;           // Should be a multiple of num_qpus for QPU
  const int SIZE   = WIDTH*HEIGHT;  // Size of 2D heat map
  const int NSPOTS = 10;

	int    kernel;
	string kernel_name;
	int    num_steps;

	int init(int argc, const char *argv[]) {
		auto const SUCCESS = CmdParameters::ALL_IS_WELL;
		auto const FAIL    = CmdParameters::EXIT_ERROR;

		set_name(argv[0]);
		params.add(base_params(true));

		auto ret = params.handle_commandline(argc, argv, false);
		if (ret != CmdParameters::ALL_IS_WELL) return ret;

		// Init the parameters in the parent
		if (!process(&params, true)) {
			ret = FAIL;
		}

		kernel      = params.parameters()["Kernel"]->get_int_value();
		kernel_name = params.parameters()["Kernel"]->get_string_value();
		num_steps   = params.parameters()["Number of steps"]->get_int_value();

		return ret;
	}
} settings;


// ============================================================================
// Local Helper functions
// ============================================================================

template<typename Arr>
void inject_hotspots(Arr &arr) {
  srand(0);

  for (int i = 0; i < settings.NSPOTS; i++) {
    int t = rand() % 256;
    int x = 1 + rand() % (settings.WIDTH  - 2);
    int y = 1 + rand() % (settings.HEIGHT - 2);
    arr[y*settings.WIDTH + x] = (float) (1000*t);
  }
}


// ============================================================================
// Scalar version
// ============================================================================

// One time step
void scalar_step(float** map, float** mapOut, int width, int height)
{
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
  // Allocate
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

  // Inject hot spots
	inject_hotspots(map);

	output_pgm_file(map, settings.WIDTH, settings.HEIGHT, 255, "heatmap_pre.pgm");

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
    gather(p);
    current = 0;
    addr = p + 16;
  }

  void prime() {
    receive(next);
    gather(addr);
  }

  void advance() {
    addr = addr + 16;
    prev = current;
    gather(addr);
    current = next;
    receive(next);
  }

  void finish() {
    receive(next);
  }

  void shiftLeft(Float& result) {
    result = rotate(current, 15);
    Float nextRot = rotate(next, 15);
    Where (index() == 15)
      result = nextRot;
    End
  }

  void shiftRight(Float& result) {
    result = rotate(current, 1);
    Float prevRot = rotate(prev, 1);
    Where (index() == 0)
      result = prevRot;
    End
  }
};


void step(Ptr<Float> map, Ptr<Float> mapOut, Int height, Int width) {
	Int pitch = width;
  Cursor row[3];
  //map = map + pitch*me(); //+ index(); // WRI DEBUG

//  // Skip first row of output map
//  mapOut = mapOut + pitch;

  For (Int y = 1 + me(), y < height - 1, y = y + numQPUs())
    Ptr<Float> p = mapOut + y*pitch; // Point p to the output row

    // Initialize three cursors for the three input rows
    for (int i = 0; i < 3; i++) row[i].init(map + (y + i - 1)*pitch);
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

      store(row[1].current - K * (row[1].current - sum * 0.125), p);
      p = p + 16;
    End

    // Cursors are finished for this row
    for (int i = 0; i < 3; i++) row[i].finish();

    // Move to the next input rows
//    map = map + pitch*numQPUs();
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

  for (int i = 0; i < settings.SIZE; i++) {
		 mapA[i] = mapB[i] = 0.0;
	}

  // Inject hot spots
	inject_hotspots(mapA);

  // Compile kernel
  auto k = compile(step);
  k.setNumQPUs(settings.num_qpus);  // default is 1

	output_pgm_file(mapA, settings.WIDTH, settings.HEIGHT, 255, "heatmap_pre.pgm");

	// WRI Debug
	auto dump = [&mapB] (int count) {
  	for (int i = 0; i < count*16; i++) {
			if (i % 16 == 0) {
				printf("\n");
			}
			printf("%8.1f, ", mapB[i]);
		}
		printf("\n");
	};

	mapA[0] = 666;

  for (int i = 0; i < settings.num_steps; i++) {
    if (i & 1)
      k.load(&mapB, &mapA, settings.HEIGHT, settings.WIDTH);  // Load the uniforms
    else
      k.load(&mapA, &mapB, settings.HEIGHT, settings.WIDTH);  // Load the uniforms

		settings.process(k);  // Invoke the kernel
  }

	dump(10);

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

	auto name = kernels[settings.kernel];

	if (!settings.silent) {
		printf("Ran kernel '%s' with %d QPU's\n", name, settings.num_qpus);
	}
	timer.end(!settings.silent);

  return 0;
}
