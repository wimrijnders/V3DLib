#include <QPULib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <CmdParameters.h>
#include "Support/Settings.h"

using namespace QPULib;
using std::string;

// Heat dissapation constant
#define K 0.25


std::vector<const char *> const kernels = { "vector", "scalar" };  // Order important! First is default

CmdParameters params = {
  "Heatmap\n",
  {{
    "Kernel",
    "-k=",
		kernels,
    "Select the kernel to use"
  }}
};


struct HeatMapSettings : public Settings {
	const int ALL = 3;

	int    kernel;
	string kernel_name;

	int init(int argc, const char *argv[]) {
		auto const SUCCESS = CmdParameters::ALL_IS_WELL;
		auto const FAIL    = CmdParameters::EXIT_ERROR;

		set_name(argv[0]);
		params.add(base_params(true));

		auto ret = params.handle_commandline(argc, argv, false);
		if (ret != CmdParameters::ALL_IS_WELL) return ret;

		// Init the parameters in the parent
		if (!process(&params)) {
			ret = FAIL;
		}

		kernel      = params.parameters()["Kernel"]->get_int_value();
		kernel_name = params.parameters()["Kernel"]->get_string_value();

		return ret;
	}
} settings;


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
  // Parameters
  const int WIDTH  = 512;
  const int HEIGHT = 506;
  const int NSPOTS = 10;
  const int NSTEPS = 1500;

  // Allocate
  float* map       = new float [WIDTH*HEIGHT];
  float* mapOut    = new float [WIDTH*HEIGHT];
  float** map2D    = new float* [HEIGHT];
  float** mapOut2D = new float* [HEIGHT];

  // Initialise
  for (int i = 0; i < WIDTH*HEIGHT; i++) map[i] = mapOut[i] = 0.0;
  for (int i = 0; i < HEIGHT; i++) {
    map2D[i]    = &map[i*WIDTH];
    mapOut2D[i] = &mapOut[i*WIDTH];
  }

  // Inject hot spots
  srand(0);
  for (int i = 0; i < NSPOTS; i++) {
    int t = rand() % 256;
    int x = 1 + rand() % (WIDTH-2);
    int y = 1 + rand() % (HEIGHT-2);
    map2D[y][x] = 1000.0f*((float) t);
  }

  // Simulate
  for (int i = 0; i < NSTEPS; i++) {
    scalar_step(map2D, mapOut2D, WIDTH, HEIGHT);
    float** tmp = map2D; map2D = mapOut2D; mapOut2D = tmp;
  }

  // Display results
  printf("P2\n%i %i\n255\n", WIDTH, HEIGHT);
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int t = (int) map2D[y][x];
      t = t < 0   ? 0 : t;
      t = t > 255 ? 255 : t;
      printf("%d ", t);
    }
    printf("\n");
	}
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
    addr = p+16;
  }

  void prime() {
    receive(next);
    gather(addr);
  }

  void advance() {
    addr = addr+16;
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

void step(Ptr<Float> map, Ptr<Float> mapOut, Int pitch, Int width, Int height)
{
  Cursor row[3];
  map = map + pitch*me() + index();

  // Skip first row of output map
  mapOut = mapOut + pitch;

  For (Int y = me(), y < height, y=y+numQPUs())

    // Point p to the output row
    Ptr<Float> p = mapOut + y*pitch;

    // Initilaise three cursors for the three input rows
    for (int i = 0; i < 3; i++) row[i].init(map + i*pitch);
    for (int i = 0; i < 3; i++) row[i].prime();

    // Compute one output row
    For (Int x = 0, x < width, x=x+16)

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
    map = map + pitch*numQPUs();

  End
}

void run_kernel() {
  // Size of 2D heat map is WIDTH*HEIGHT:
  //   * with zero padding, it is NROWS*NCOLS
  //   * i.e. there is constant cold at the edges
  //   * NCOLs should be a multiple of 16
  //   * HEIGHT should be a multiple of num_qpus
  const int WIDTH  = 512-16;
  const int NCOLS  = WIDTH+16;
  const int HEIGHT = 504;
  const int NROWS  = HEIGHT+2;
  const int NSPOTS = 10;
  const int NSTEPS = 1500;

  // Allocate and initialise input and output maps

  SharedArray<float> mapA(NROWS*NCOLS), mapB(NROWS*NCOLS);
  for (int y = 0; y < NROWS; y++)
    for (int x = 0; x < NCOLS; x++) {
      mapA[y*NCOLS+x] = 0;
      mapB[y*NCOLS+x] = 0;
    }

  // Inject hot spots
  srand(0);
  for (int i = 0; i < NSPOTS; i++) {
    int t = rand() % 256;
    int x = rand() % WIDTH;
    int y = 1 + rand() % HEIGHT;
    mapA[y*NCOLS+x] = (float) (1000*t);
  }

  // Compile kernel
  auto k = compile(step);
  //k.setNumQPUs(settings.num_qpus);  // default is 1

  for (int i = 0; i < NSTEPS; i++) {
    if (i & 1)
      k.load(&mapB, &mapA, NCOLS, WIDTH, HEIGHT);  // Load the uniforms
    else
      k.load(&mapA, &mapB, NCOLS, WIDTH, HEIGHT);  // Load the uniforms

		settings.process(k);  // Invoke the kernel
  }

  // Display results
  printf("P2\n%i %i\n255\n", WIDTH, HEIGHT);
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int t = (int) mapB[(y+1)*NCOLS+x];
      t = t < 0   ? 0 : t;
      t = t > 255 ? 255 : t;
      printf("%d ", t);
    }
    printf("\n");
	}
}


// ============================================================================
// Local functions
// ============================================================================

void end_timer(timeval tvStart) {
  timeval tvEnd, tvDiff;
  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  printf("%ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);
}


// ============================================================================
// Main
// ============================================================================

int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

  timeval tvStart;
  gettimeofday(&tvStart, NULL);

	switch (settings.kernel) {
		case 0: run_kernel();  break;	
		case 1: run_scalar(); break;
	}

	auto name = kernels[settings.kernel];
	printf("Ran kernel '%s' with %d QPU's in ", name, settings.num_qpus);
	end_timer(tvStart);

  return 0;
}
