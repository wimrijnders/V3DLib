/**i***************************************************************************
 * TODO
 * ====
 *
 * - Following shouldn't be accepted:
 *
 *     sudo obj-qpu/bin/Mandelbrot -k=2 -pgmmpoep
 *
 ******************************************************************************/
#include <string>
#include <V3DLib.h>
#include <CmdParameters.h>
#include "Support/Timer.h"
#include "Support/Settings.h"
#include "Support/pgm.h"
#include "vc4/RegisterMap.h"


using namespace V3DLib;
using std::string;

std::vector<const char *> const kernels = { "multi", "single", "cpu", "all" };  // Order important! First is default, 'all' must be last


CmdParameters params = {
  "Mandelbrot Generator\n"
	"\n"
	"Calculates Mandelbrot for a given region and outputs the result as a PGM bitmap file.\n"
	"Because this calculation is purely hardware-bound, it is a good indication of overall speed.\n"
	"It will therefore be used for performance comparisons of platforms and configurations.\n",
  {{
    "Kernel",
    "-k=",
		kernels,
    "Select the kernel to use"
	}, {
    "Output PGM file",
		"-pgm",
		ParamType::NONE,   // Prefix needed to disambiguate
    "Output a PGM bitmap of the calculation results.\n"
    "If enabled, a PGM bitmap named 'mandelbrot.pgm' will be created in the current working directory.\n"
    "Note that creating the PGM-file takes significant time, and will skew the performance results if enabled\n",
	}, {
		"Number of steps",
		"-steps=",
		ParamType::POSITIVE_INTEGER,
		"Maximum number of iterations to perform per point",
		1024
  }}
};


struct MandSettings : public Settings {
	const int ALL = 3;

	int    kernel;
	string kernel_name;
	bool   output_pgm;
	int    num_iterations;

  // Initialize constants for the kernels
  const int   numStepsWidth   = 1024;
  const int   numStepsHeight  = 1024;
  const float topLeftReal     = -2.5f;
  const float topLeftIm       = 2.0f;
  const float bottomRightReal = 1.5f;
  const float bottomRightIm   = -2.0f;

  int num_items() const { return numStepsWidth*numStepsHeight; }
  float offsetX() const { return (bottomRightReal - topLeftReal  )/((float) numStepsWidth  - 1); }
  float offsetY() const { return (topLeftIm       - bottomRightIm)/((float) numStepsHeight - 1); }

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

		kernel         = params.parameters()["Kernel"]->get_int_value();
		kernel_name    = params.parameters()["Kernel"]->get_string_value();
		output_pgm     = params.parameters()["Output PGM file"]->get_bool_value();
		num_iterations = params.parameters()["Number of steps"]->get_int_value();

		return ret;
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
void mandelbrot_cpu(int *result) {
  for (int xStep = 0; xStep < settings.numStepsWidth; xStep++) {
    for (int yStep = 0; yStep < settings.numStepsHeight; yStep++) {
      float realC = settings.topLeftReal   + ((float) xStep)*settings.offsetX();
      float imC   = settings.bottomRightIm + ((float) yStep)*settings.offsetY();

      int count = 0;
      float real = realC;
      float im   = imC;
      float radius = (real*real + im*im);

      while (radius < 4 && count < settings.num_iterations) {
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
  Int &numIterations,
  Ptr<Int> &dst
) {
  Float re = reC;
  Float im = imC;
  Int count = 0;

  Float reSquare = re*re;
  Float imSquare = im*im;

  // Following is a float version of boolean expression: ((reSquare + imSquare) < 4 && count < numIterations)
  // It works because `count` increments monotonically.
  FloatExpr condition = (4.0f - (reSquare + imSquare))*toFloat(numIterations - count);
  Float checkvar = condition;

  While (any(checkvar > 0.0f))
    Where (checkvar > 0.0f)
      Float imTmp = 2*re*im;
      re = (reSquare - imSquare) + reC;
      im = imTmp  + imC;

      reSquare = re*re;
      imSquare = im*im;
      count++;

      checkvar = condition; 
    End
  End

  store(count, dst);
}


void mandelbrot_single(
  Float topLeftReal, Float topLeftIm,
  Float offsetX, Float offsetY,
  Int numStepsWidth, Int numStepsHeight,
  Int numIterations,
  Ptr<Int> result
) {
  For (Int yStep = 0, yStep < numStepsHeight, yStep++)
    For (Int xStep = 0, xStep < numStepsWidth - 16, xStep = xStep + 16)
      Int xIndex = xStep + index();
			Ptr<Int> dst = result + xStep + yStep*numStepsWidth;

      mandelbrotCore(
        (topLeftReal + offsetX*toFloat(xIndex)),
        (topLeftIm   - offsetY*toFloat(yStep)),
        numIterations,
        dst);
    End
  End
}


/**
 * @brief Multi-QPU version
 */
void mandelbrot_multi(
  Float topLeftReal, Float topLeftIm,
  Float offsetX, Float offsetY,
  Int numStepsWidth, Int numStepsHeight,
  Int numIterations,
  Ptr<Int> result
) {
	result -= me() << 4;  // Correct for per-QPU offset

  For (Int yStep = 0, yStep < numStepsHeight - numQPUs(), yStep = yStep + numQPUs())
    Int yIndex = yStep + me();

    For (Int xStep = 0, xStep < numStepsWidth - 16, xStep = xStep + 16)
      Int xIndex = xStep + index();
			Ptr<Int> dst = result + xStep + yIndex*numStepsWidth;

      mandelbrotCore(
        (topLeftReal + offsetX*toFloat(xIndex)),
        (topLeftIm   - offsetY*toFloat(yIndex)),
        numIterations,
        dst);
    End
  End
}


// ============================================================================
// Local functions
// ============================================================================

using KernelType = decltype(mandelbrot_single);

template<class Array>
void output_pgm(Array &result) {
	if (!settings.output_pgm) return;

  int width         = settings.numStepsWidth;
	int height        = settings.numStepsHeight;
	int numIterations = settings.num_iterations;

	output_pgm_file(result, width, height, numIterations, "mandelbrot.pgm");
}


void run_qpu_kernel(KernelType &kernel) {
  assert(0 == settings.numStepsWidth % 16);       // width needs to be a multiple of 16

  auto k = compile(kernel);
  k.setNumQPUs(settings.num_qpus);

	SharedArray<int> result(settings.num_items());  // Allocate and initialise

  k.load(
		settings.topLeftReal, settings.topLeftIm,
		settings.offsetX(), settings.offsetY(),
		settings.numStepsWidth, settings.numStepsHeight,
		settings.num_iterations,
		&result);

  settings.process(k);
	output_pgm(result);
}


/**
 * Run a kernel as specified by the passed kernel index
 */
void run_kernel(int kernel_index) {
	Timer timer;

	switch (kernel_index) {
		case 0: run_qpu_kernel(mandelbrot_multi);  break;	
		case 1: run_qpu_kernel(mandelbrot_single); break;
		case 2: {
  			int *result = new int [settings.num_items()];  // Allocate and initialise

  			mandelbrot_cpu(result);
				output_pgm(result);

				delete result;
			}
			break;
	}

	auto name = kernels[kernel_index];

	timer.end(!settings.silent);

	if (!settings.silent) {
		printf("Ran kernel '%s' with %d QPU's\n", name, settings.num_qpus);
	}
}


// ============================================================================
// Main
// ============================================================================

int main(int argc, const char *argv[]) {
	//printf("Check pre\n");
	//RegisterMap::checkThreadErrors();   // TODO: See if it's useful to check this every time after a kernel has run

	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

	if (settings.kernel == settings.ALL) {
		for (int i = 0; i < settings.ALL; ++i ) {
			run_kernel(i);
		}
	} else {
		run_kernel(settings.kernel);
	}

	//printf("Check post\n");
	//RegisterMap::checkThreadErrors();

	printf("\n");
  return 0;
}
