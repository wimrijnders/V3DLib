#include <sys/time.h>
#include <string>
#include <QPULib.h>
#include <CmdParameters.h>

using namespace QPULib;
using std::string;

std::vector<const char *> const kernels = { "2", "1", "scalar", "all" };


CmdParameters params = {
  "Mandelbrot generator\n\n"
	"Calculates mandelbrot for a given region and outputs the result as a pgm graphics file.\n"
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
  }}
};


struct Settings {
	const int ALL = 3;

	int kernel;
	int num_qpus;
	string kernel_name;

  // Initialize constants for the kernels
  const int numStepsWidth     = 1024;
  const int numStepsHeight    = 1024;
  const int numIterations     = 1024;
  const float topLeftReal     = -2.5f;
  const float topLeftIm       = 2.0f;
  const float bottomRightReal = 1.5f;
  const float bottomRightIm   = -2.0f;
  const int NUM_ITEMS = numStepsWidth*numStepsHeight;

	void output() {
		printf("Settings:\n");
		printf("  kernel index: %d\n", kernel);
		printf("  kernel name : %s\n", kernel_name.c_str());
		printf("  Num QPU's   : %d\n", num_qpus);
		printf("  Num items   : %d\n", NUM_ITEMS);
		printf("\n");
	}
} settings;


// ============================================================================
// Vector version
// ============================================================================

void mandelbrotCore(
  Float reC, Float imC,
  Int &resultIndex,
  Int &numIterations,
  Ptr<Int> &result)
{
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
  //result[resultIndex] = count;
}


void mandelbrot_1(
  Float topLeftReal, Float topLeftIm,
  Float offsetX, Float offsetY,
  Int numStepsWidth, Int numStepsHeight,
  Int numIterations,
  Ptr<Int> result)
{
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
  Ptr<Int> result)
{
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

int init_settings(int argc, const char *argv[]) {
	auto ret = params.handle_commandline(argc, argv, false);
	if (ret != CmdParameters::ALL_IS_WELL) {
		return ret;
	}

	settings.kernel      = params.parameters()[0]->get_int_value();
	settings.kernel_name = params.parameters()[0]->get_string_value();
	settings.num_qpus    = params.parameters()[1]->get_int_value();
	settings.output();

	return ret;
}



/**
 * Two format limits need to be taken into account:
 *
 * - Max line length of 70 characters, 'count' handles this
 * - Max gray value of 65536
 */
template<class Array>
void output_pgm(Array &result) {
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


// ============================================================================
// Scalar version
// ============================================================================

void mandelbrot(
  float topLeftReal, float topLeftIm,
  float bottomRightReal, float bottomRightIm,
  int numStepsWidth, int numStepsHeight,
  int numIterations,
  int *result) {
  float offsetX = (bottomRightReal - topLeftReal)/((float) numStepsWidth - 1);
  float offsetY = (topLeftIm - bottomRightIm)/((float) numStepsHeight - 1);

  for (int xStep = 0; xStep < numStepsWidth; xStep++) {
    for (int yStep = 0; yStep < numStepsHeight; yStep++) {
      float realC = topLeftReal   + ((float) xStep)*offsetX;
      float imC   = bottomRightIm + ((float) yStep)*offsetY;

      int count = 0;
      float real = realC;
      float im   = imC;
      float radius = (real*real + im*im);
      while (radius < 4 && count < numIterations) {
        float tmpReal = real*real - im*im;
        float tmpIm   = 2*real*im;
        real = tmpReal + realC;
        im   = tmpIm + imC;

        radius = (real*real + im*im);
        count++;
      }

      result[xStep + yStep*numStepsWidth] = count;
    }
  }
}


void run_qpu_kernel( decltype(mandelbrot_1) &kernel) {
  // Construct kernel
  auto k = compile(kernel);		// Using this as default

	// Allocate and initialise
	SharedArray<int> result(settings.NUM_ITEMS);

  assert(0 == settings.numStepsWidth % 16);  // width needs to be a multiple of 16
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
 * TODO: time includes the creation of the PGM. We need to remove this from the time.
 *       OR make the output conditional.
 */
void run_kernel(int kernel_index) {
  timeval tvStart;
  gettimeofday(&tvStart, NULL);

	switch (kernel_index) {
		case 1: 
						run_qpu_kernel(mandelbrot_1);
						break;
		case 0: 
						run_qpu_kernel(mandelbrot_2);
						break;	
		case 2: {
  						// Allocate and initialise
  						int *result = new int [settings.NUM_ITEMS];

  						mandelbrot(
								settings.topLeftReal,
								settings.topLeftIm,
								settings.bottomRightReal,
								settings.bottomRightIm,
								settings.numStepsWidth,
								settings.numStepsHeight,
								settings.numIterations,
								result);

						  output_pgm(result);
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
	auto ret = init_settings(argc, argv);
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
