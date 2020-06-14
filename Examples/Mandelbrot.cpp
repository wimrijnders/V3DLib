/**i***************************************************************************
 * TODO
 * ====
 *
 * - Following shouldn't be accepted:
 *
 *     sudo obj-qpu/bin/Mandelbrot -k=2 -pgmmpoep
 *
 ******************************************************************************/
#include <sys/time.h>
#include <string>
#include <QPULib.h>
#include <CmdParameters.h>

using namespace QPULib;
using std::string;

std::vector<const char *> const kernels = { "2", "1", "scalar", "all" };  // Order important! '2' at front by necessity, 'all' must be last


CmdParameters params = {
  "Mandelbrot Generator\n\n"
	"Calculates Mandelbrot for a given region and outputs the result as a PGM bitmap file.\n"
	"Because this calculation is purely hardware-bound, it is a good indication of overall speed.\n"
	"It will therefore be used for performance comparisons of platforms and configurations.\n",
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
    "Output PGM file",
		"-pgm",
		ParamType::NONE,   // Prefix needed to dsambiguate
    "Output a PGM bitmap of the calculation results.\n"
    "If enabled, a PGM bitmap named 'mandelbrot.pgm' will be created in the current working directory.\n"
    "Note that creating the PGM-file takes significant time, and will skew the performance results if enabled\n",
  }}
};


struct Settings {
	const int ALL = 3;

	int    kernel;
	int    num_qpus;
	string kernel_name;
	bool   output_pgm;

  // Initialize constants for the kernels
  const int   numStepsWidth   = 1024;
  const int   numStepsHeight  = 1024;
  const int   numIterations   = 1024;
  const float topLeftReal     = -2.5f;
  const float topLeftIm       = 2.0f;
  const float bottomRightReal = 1.5f;
  const float bottomRightIm   = -2.0f;


  int num_items() { return numStepsWidth*numStepsHeight; }


	int init(int argc, const char *argv[]) {
		auto ret = params.handle_commandline(argc, argv, false);
		if (ret != CmdParameters::ALL_IS_WELL) return ret;

		kernel      = params.parameters()[0]->get_int_value();
		kernel_name = params.parameters()[0]->get_string_value();
		num_qpus    = params.parameters()[1]->get_int_value();
		output_pgm  = params.parameters()[2]->get_bool_value();
		output();

		return ret;
	}


	void output() {
		printf("Settings:\n");
		printf("  kernel index: %d\n", kernel);
		printf("  kernel name : %s\n", kernel_name.c_str());
		printf("  Num QPU's   : %d\n", num_qpus);
		printf("  Num items   : %d\n", num_items());
		printf("  Output PGM  : %s\n", output_pgm?"true":"false");
		printf("\n");
	}
} settings;


// ============================================================================
// Kernels
// ============================================================================

/**
 * Scalar kernel
 *
 * This runs on the CPU
 */
void mandelbrot(int *result) {
  float offsetX = (settings.bottomRightReal - settings.topLeftReal)/((float) settings.numStepsWidth - 1);
  float offsetY = (settings.topLeftIm - settings.bottomRightIm)/((float) settings.numStepsHeight - 1);

  for (int xStep = 0; xStep < settings.numStepsWidth; xStep++) {
    for (int yStep = 0; yStep < settings.numStepsHeight; yStep++) {
      float realC = settings.topLeftReal   + ((float) xStep)*offsetX;
      float imC   = settings.bottomRightIm + ((float) yStep)*offsetY;

      int count = 0;
      float real = realC;
      float im   = imC;
      float radius = (real*real + im*im);

      while (radius < 4 && count < settings.numIterations) {
        float tmpReal = real*real - im*im;
        float tmpIm   = 2*real*im;
        real = tmpReal + realC;
        im   = tmpIm + imC;

        radius = (real*real + im*im);
        count++;
      }

      result[xStep + yStep*settings.numStepsWidth] = count;
    }
  }
}


/**
 * Common part of the QPU kernels
 */
void mandelbrotCore(
  Float reC, Float imC,
  Int &resultIndex,
  Int &numIterations,
  Ptr<Int> &result) {
  Float re = reC;
  Float im = imC;
  Int count = 0;

  Float reSquare = re*re;
  Float imSquare = im*im;

  // Following is a float version of boolean expression: ((reSquare + imSquare) < 4 && count < numIterations)
  // It works because `count` increments monotonically.
  FloatExpr condition = (4.0f - (reSquare + imSquare))*toFloat(numIterations - count);
  Float checkvar = condition;

  While (any(checkvar > 0))
    Where (checkvar > 0)
      Float imTmp = 2*re*im;
      re   = (reSquare - imSquare) + reC;
      im   = imTmp  + imC;

      reSquare = re*re;
      imSquare = im*im;
      count++;

      checkvar = condition; 
    End
  End

  store(count, result + resultIndex);
}


void mandelbrot_1(
  Float topLeftReal, Float topLeftIm,
  Float offsetX, Float offsetY,
  Int numStepsWidth, Int numStepsHeight,
  Int numIterations,
  Ptr<Int> result) {
  For (Int yStep = 0, yStep < numStepsHeight, yStep++)
    For (Int xStep = 0, xStep < numStepsWidth, xStep = xStep + 16)
      Int xIndex = index() + xStep;
      Int resultIndex = xIndex + yStep*numStepsWidth;

      mandelbrotCore(
        (topLeftReal + offsetX*toFloat(xIndex)),
        (topLeftIm   - offsetY*toFloat(yStep)),
				resultIndex,
        numIterations,
        result);
    End
  End
}


/**
 * @brief Multi-QPU version
 */
void mandelbrot_2(
  Float topLeftReal, Float topLeftIm,
  Float offsetX, Float offsetY,
  Int numStepsWidth, Int numStepsHeight,
  Int numIterations,
  Ptr<Int> result) {
  Int inc = numQPUs();

  For (Int yStep = 0, yStep < numStepsHeight, yStep = yStep + inc)
    Int yIndex = me() + yStep;

    For (Int xStep = 0, xStep < numStepsWidth && yIndex < numStepsHeight, xStep = xStep + 16)
      Int xIndex = index() + xStep;
      Int resultIndex = xIndex + yIndex*numStepsWidth;

      mandelbrotCore(
        (topLeftReal + offsetX*toFloat(xIndex)),
        (topLeftIm   - offsetY*toFloat(yIndex)),
        resultIndex,
        numIterations,
        result);
    End
  End
}


// ============================================================================
// Local functions
// ============================================================================


/**
 * Two format limits need to be taken into account:
 *
 * - Max line length of 70 characters, 'count' handles this
 * - Max gray value of 65536
 */
template<class Array>
void output_pgm(Array &result) {
	if (!settings.output_pgm) return;


  int width  = settings.numStepsWidth;
	int height = settings.numStepsHeight;
	int numIterations = settings.numIterations;

	const int GrayLimit = 65536;
	float factor = -1.0f;
	int maxGray = numIterations;

	if (maxGray > GrayLimit) {
		// printf ("output_pgm adjust max gray\n");
		factor = ((float) GrayLimit)/((float) maxGray);
		maxGray = GrayLimit;
	}

	auto scale = [factor] (int value) -> int {
		if (factor == -1.0f) return value;
		return (int) (factor*((float) value));
	};


  FILE *fd = fopen("mandelbrot.pgm", "w") ;
  if (fd == nullptr) {
    printf("can't open file for pgm output\n");
    return;
  }

  // Write header
  fprintf(fd, "P2\n");
  fprintf(fd, "%d %d\n", width, height);
  fprintf(fd, "%d\n", maxGray);

  int count = 0; // Limit output to 10 elements per line
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      fprintf(fd, "%d ", scale(result[x + width*y]));
      count++;
      if (count >= 10) {
        fprintf(fd, "\n");
        count = 0;
      }
    }
    fprintf(fd, "\n");
  }

  fclose(fd);
}


void end_timer(timeval tvStart) {
  timeval tvEnd, tvDiff;
  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  printf("%ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);
}


void run_qpu_kernel( decltype(mandelbrot_1) &kernel) {
  auto k = compile(kernel);

	SharedArray<int> result(settings.num_items());  // Allocate and initialise

  assert(0 == settings.numStepsWidth % 16);    // width needs to be a multiple of 16
	float offsetX = (settings.bottomRightReal - settings.topLeftReal)/((float) settings.numStepsWidth - 1);
	float offsetY = (settings.topLeftIm - settings.bottomRightIm)/((float) settings.numStepsHeight - 1);

  k.setNumQPUs(settings.num_qpus);

	k(
		settings.topLeftReal, settings.topLeftIm,
		offsetX, offsetY,
		settings.numStepsWidth, settings.numStepsHeight,
		settings.numIterations,
		&result);

	output_pgm(result);
}


/**
 * Run a kernel as specified by the passed kernel index
 */
void run_kernel(int kernel_index) {
  timeval tvStart;
  gettimeofday(&tvStart, NULL);

	switch (kernel_index) {
		case 0: 
			run_qpu_kernel(mandelbrot_2);
			break;	
		case 1: 
			run_qpu_kernel(mandelbrot_1);
			break;
		case 2: {
  			int *result = new int [settings.num_items()];  // Allocate and initialise

  			mandelbrot(result);
				output_pgm(result);

				delete result;
			}
			break;
	}

	//auto name = settings.kernel_name.c_str();
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

	if (settings.kernel == settings.ALL) {
		for (int i = 0; i < settings.ALL; ++i ) {
			run_kernel(i);
		}
	} else {
		run_kernel(settings.kernel);
	}

	printf("\n");
  return 0;
}
