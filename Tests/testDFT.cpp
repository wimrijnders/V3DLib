///////////////////////////////////////////////////////////////////////////////
// Tests for Discrete Fourier Transform (DFT)
///////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "Support/Timer.h"
#include "Source/Complex.h"
#include "support/support.h"
#include "support/matrix_support.h"
#include "support/dft_support.h"
#include "Kernel.h"
#include "Kernels/Matrix.h"
#include "LibSettings.h"
#include "support/ProfileOutput.h"


using namespace V3DLib;

namespace {

int const DimTest = 128;

void create_test_wavelet(Complex::Array2D &input, int const Dim) {
  REQUIRE(input.columns() == Dim);

  for (int c = 0; c < Dim; ++c) {
    input[0][c] = complex(wavelet_function(c, Dim), 0.0f);
  }
}


/**
 * @param high_precision if true, use sin/cos with extra precision.
 *                       Generally, this is a good idea.
 */
void create_dft_matrix(Complex::Array2D &arr, bool high_precision = true) {
  REQUIRE(arr.rows() > 0);
  REQUIRE(arr.rows() % 16 == 0);
  int const Dim = arr.rows();
  REQUIRE(Dim == arr.columns());

  float const precision = high_precision?1.5e-3f:6.0e-2f;

  // Check if precision of value is within an acceptable range
  // V3D lib function is compared against the CPU lib value
  auto check_acceptable_precision = [precision, Dim] (int r, int c, complex const &tmp) {
    double angle_lib = (2*M_PI*((float) (-r*c)) / Dim);
    complex tmp_lib((float) std::cos(angle_lib), (float) std::sin(angle_lib)); 

    INFO("r: " << r << ", c: " << c);
    INFO("tmp_lib: " << tmp_lib.dump() << ", tmp: " << tmp.dump());
    REQUIRE(abs(tmp_lib.re() - tmp.re()) < precision);
    REQUIRE(abs(tmp_lib.im() - tmp.im()) < precision);
  };


  for (int r = 0; r < Dim; ++r) {
    for (int c = 0; c < Dim; ++c) {
      float angle = ((float) (-r*c))/((float) Dim); 
      complex tmp(functions::cos(angle, high_precision), functions::sin(angle, high_precision));
      //std::cout << tmp.dump();

      check_acceptable_precision(r, c, tmp);

      // Use the V3DLib scalar value, this compares well with the GPU sin/cos
      arr[r][c] = tmp;
    }
  }
}


/**
 * Create some visual output for DFT matrices
 */
void output_dft(Complex::Array2D &input, Complex::Array2D &result, char const *file_prefix) {
  assert(file_prefix != nullptr);
  int const Dim = input.columns();

  {
    float real_input[Dim];
    for (int c = 0; c < Dim; ++c) {
      real_input[c] = input[0][c].re();
    }

    std::string filename = "obj/test/";
    filename << file_prefix << "_input.pgm";

    PGM pgm(Dim, 100);
    pgm.plot(real_input, Dim)
       .save(filename.c_str());
  }

  {
    float real_result[Dim];
    for (int c = 0; c < Dim; ++c) {
      real_result[c] = result[0][c].magnitude();
    }

    std::string filename = "obj/test/";
    filename << file_prefix << "_result.pgm";

    PGM pgm(Dim, 100);
    pgm.plot(real_result, Dim)
       .save(filename.c_str());
  }
}


/**
 * Helper function for compare_dfts()
 */
void compare_arrays_dft(
  V3DLib::Complex::Array2D &result,
  V3DLib::Complex::Array2D &expected,
  std::string const &result_label,
  std::string const &expected_label,
  float precision = -1.0f
) {
  INFO("Comparing " << result_label << " with " << expected_label);
  INFO("Result  : " << result.dump());
  INFO("Expected: " << expected.dump());

  compare_arrays(result, expected, precision);
}


/**
 * @return  true if all kernels compiled something,
 *          false if any kernel failed compilation for vc4 as well as v3d
 */ 
bool compare_dfts(int Dim, bool do_profiling) {
  REQUIRE(Dim > 0);
  REQUIRE(Dim % 16 == 0);
  REQUIRE(ProfileOutput::num_iterations > 0);

  INFO("Dimension: " << Dim);

  //
  // Support Stuff
  //
  CompileFor for_platform = (Platform::has_vc4())?CompileFor::VC4:CompileFor::V3D;

  ProfileOutput profile_output;
  profile_output.show_compile(true);


  //
  // Initialize shared arrays
  //
  Complex::Array2D input(1, Dim);  // Create input; remember, transposed!
  create_test_wavelet(input, Dim);

  Float::Array input_float(Dim);
  for (int i = 0; i < input.re().columns(); ++i) {
    input_float[i] = input.re()[0][i];
  }

  // Prepare DFT matrix
  Complex::Array2D dft_matrix(Dim);
  create_dft_matrix(dft_matrix);

  Complex::Array2D result_mult;  // Will be Dimx1
  Complex::Array2D result_complex;
  Complex::Array2D result_float;

  // Selector for kernel tests to run
  // The goal is to make it easy to enable/disable isub-tests; this encompassing test is real heavy
  enum {
    FLOAT_MULT      = 1,         // Tests using DFT decorators
    COMPLEX_MULT    = 2,
    FLOAT_DFT       = 4,
    FLOAT_DFT_CLASS = 8,         // Tests using DFT class
    FLOAT_DFT_CLASS_BLOCKS = 16,
    FLOAT_DFT_CLASS_BLOCKS_MULTI = 32
  };
    

  //
  // Run the kernels
  //
  //int const run_kernels = 31;  // bit field specifying which kernels to run 
  int const run_kernels =     // bit field specifying which kernels to run 
      FLOAT_MULT              // Decorator calls
    + COMPLEX_MULT
    + FLOAT_DFT               // DFT class calls
    + FLOAT_DFT_CLASS
    + FLOAT_DFT_CLASS_BLOCKS
    + FLOAT_DFT_CLASS_BLOCKS_MULTI
  ;
  int compiled = 0;

  auto run = [&profile_output, &Dim] (BaseKernel &k, std::string const &label) {
    profile_output.run(Dim, label, [&k] (int numQPUs) {
      k.setNumQPUs(numQPUs);
      k.call();
    });
  };

  if (run_kernels & FLOAT_MULT) {
    std::string label = "float mult";
    Timer timer1;

    // Do regular complex matrix multiplication
    // In this call, the matrix and input are switched.
    // This is slightly more efficient and should not affect the result
    auto k = compile(kernels::matrix_mult_decorator(input, dft_matrix, result_mult), for_platform);
    profile_output.add_compile(label, timer1, Dim);

    if (!k.has_errors()) {
      compiled += FLOAT_MULT;
      k.load(&result_mult, &input, &dft_matrix);
      run(k, label);
    }
  }

  if (run_kernels & COMPLEX_MULT) {
    std::string label = "complex mult";
    Timer timer1;
    auto k = compile(kernels::dft_decorator(input, result_complex), for_platform);
    profile_output.add_compile(label, timer1, Dim);

    if (!k.has_errors()) {
      compiled += COMPLEX_MULT;
      k.load(&result_complex, &input);
      run(k, label);

      if (!do_profiling) {
        INFO("Comparing " << label << " with mult");
        compare_arrays(result_complex, result_mult, 0.02f);
      }
    }
  }

  if (run_kernels & FLOAT_DFT) {
    std::string label = "dft float";
    Timer timer1;

    auto k = compile(kernels::dft_decorator(input_float, result_float), for_platform);
    profile_output.add_compile(label, timer1, Dim);

    if (!k.has_errors()) {
      compiled += FLOAT_DFT;
      k.load(&result_float, &input_float);
      run(k, label);
      //debug(result_float.dump());

      if (!do_profiling) {
        INFO("Comparing " << label << " with complex");
        // No need to compare inline float with mult, identical to inline complex (see previous)");
        compare_arrays(result_float, result_complex);
      }
    }
  }

  if (run_kernels & FLOAT_DFT_CLASS) {
    std::string label = "DFT class float";
    Timer timer1;

    DFT k(input_float);
    k.compile();
    profile_output.add_compile(label, timer1, Dim);

    if (!k.has_errors()) {
      compiled += FLOAT_DFT_CLASS;
      //run(k, label);
      profile_output.run(Dim, label, [&k] (int numQPUs) {
        k.setNumQPUs(numQPUs);
        k.call();
      });

      if (!do_profiling) {
        INFO("Comparing " << label << " with float");
        INFO(k.result().dump());
        compare_arrays(k.result(), result_float, 0.0f);  // Match should be exact
      }
    }
  }

  if (run_kernels & FLOAT_DFT_CLASS_BLOCKS) {
    std::string label = "DFT float 2 blocks";
    Timer timer1;

    DFT k(input_float);
    k.num_blocks(2);
    k.compile();
    profile_output.add_compile(label, timer1, Dim);

    if (!k.has_errors()) {
      compiled += FLOAT_DFT_CLASS_BLOCKS;
      //run(k, label);
      profile_output.use_single_qpu(true);
      profile_output.run(Dim, label, [&k] (int numQPUs) {
        k.setNumQPUs(numQPUs);
        k.call();
      });
      profile_output.use_single_qpu(false);

      if (!do_profiling) {
        INFO("num QPUs: " << k.numQPUs());
        compare_arrays_dft(k.result(), result_float, label, "float", 2.0e-6f);  // Match should be exact, tiny diff
      }
    }
  }

  if (run_kernels & FLOAT_DFT_CLASS_BLOCKS_MULTI) {
    std::string label = "DFT float 2 blocks multi call";
    Timer timer1;

    DFT k(input_float);
    k.num_blocks(2);
    k.multi_block(true);
    k.compile();
    profile_output.add_compile(label, timer1, Dim);

    if (!k.has_errors()) {
      compiled += FLOAT_DFT_CLASS_BLOCKS_MULTI;
      //run(k, label);
      profile_output.use_single_qpu(true);
      profile_output.run(Dim, label, [&k] (int numQPUs) {
        k.setNumQPUs(numQPUs);
        k.call();
      });
      profile_output.use_single_qpu(false);

      if (!do_profiling) {
        INFO("num QPUs: " << k.numQPUs());
        compare_arrays_dft(k.result(), result_float, label, "float", 2.0e-6f);  // Match should be exact, tiny diff
      }
    }
  }

  if (do_profiling) {
      std::cout << profile_output.dump();
  } else {
    REQUIRE(compiled == run_kernels);  // All bits for all kernels should be set
  }

  //std::cout << "compiled: " << compiled;
  return (compiled != 0);
}

}  // anon namespace


TEST_CASE("Discrete Fourier Transform [dft]") {

  /**
   * Check out how DFT with a matrix looks like
   * Turns out that the precision is pretty lousy - still usable, though.
   */
  SUBCASE("Check DFT matrix") {
    int const DimShift = 4;
    int const Dim = 1 << DimShift;

    Complex::Array2D dft_matrix(Dim);
    create_dft_matrix(dft_matrix, true);  // Setting high_precision = false fails the test miserably
    //std::cout << dft_matrix.dump();

    // Tranpose should be equal to self
    for (int r = 0; r < Dim; ++r) {
      for (int c = 0; c < Dim; ++c) {
        REQUIRE(dft_matrix[r][c] == dft_matrix[c][r]); 
      }
    }


    //
    // Multiplying with (transposed) conjugate should give DimxI (I: identity matrix)
    //
    Complex::Array2D dft_conjugate(Dim);
    for (int r = 0; r < Dim; ++r) {
      for (int c = 0; c < Dim; ++c) {
        dft_conjugate[r][c] = dft_matrix[r][c].conjugate();  // Don't transpose!
      }
    }

    Complex::Array2D result;

    auto k = compile(kernels::matrix_mult_decorator(dft_conjugate, dft_matrix, result));
    result.fill({-1, -1});
    k.load(&result, &dft_conjugate, &dft_matrix).emu();

    float const precision2 = 2e-2f;

    //INFO(result.dump());

    for (int r = 0; r < Dim; ++r) {
      for (int c = 0; c < Dim; ++c) {

        float test = 0.0f;
        if (r == c) {
          test = (float) Dim;
        }

        INFO("r: " << r << ", c: " << c 
                   << ", expected: (" << test << ", 0.0)" 
                   << ", got: " << result[r][c].dump());

        REQUIRE(abs(result[r][c].re() - test) < precision2);
        REQUIRE(abs(result[r][c].im() - 0.0f) < precision2);
      }
    }
  }


  SUBCASE("Check DFT using standard matrix multiplication") {
    int const Dim = DimTest;

    Complex::Array2D input(1, Dim);  // Create input; remember, transposed!
    create_test_wavelet(input, Dim);

    // Prepare DFT matrix
    Complex::Array2D dft_matrix(Dim);
    create_dft_matrix(dft_matrix, false);

    Complex::Array2D result(1, Dim);

    {
      Complex::Array2D result_tmp;  // Will be Dimx16, columns padded to 16 and only 1st relevant
      auto k = compile(kernels::matrix_mult_decorator(dft_matrix, input, result_tmp));
      k.pretty(false,  "obj/test/dft_matrix_v3d.txt");

      //std::cout << "result dimensions: (" << result_tmp.rows() << ", " << result_tmp.columns() << ")" << std::endl;

      k.setNumQPUs(8);  // Running with multi-QPU gives very limited performance improvement
      result.fill({-1, -1});
      k.load(&result_tmp, &dft_matrix, &input);
      k.call();

      // Columns are padded to multiples of 16, only the first column is relevant
      // Translate to better form
      for (int c = 0; c < Dim; ++c) {
        result[0][c] = result_tmp[c][0];
      }
    }


    // Switching input and matrix around should have same result, but transposed compared to previous
    // No need to transpose dft_matrix due to symmetry
    Complex::Array2D result_switched;  // Will be Dimx1

    {
      auto k = compile(kernels::matrix_mult_decorator(input, dft_matrix, result_switched));

      k.setNumQPUs(8);  // Running with multi-QPU gives very limited performance improvement
      k.load(&result_switched, &input, &dft_matrix);

      k.interpret();
    }

    //std::cout << result.dump() << std::endl;
    //std::cout << result_switched.dump() << std::endl;

    REQUIRE(result_switched.columns() == Dim);
    compare_arrays(result, result_switched);

    output_dft(input, result_switched, "dft");
  }
}


TEST_CASE("Discrete Fourier Transform tmp [dft][dft2]") {
  SUBCASE("Check DFT with inline sin/cos") {
    int const Dim = 16*2;  // max vc4: 16*4. Max v3d is higher, at least 64*8

    Complex::Array2D input(1, Dim);  // Create input; remember, transposed!
    create_test_wavelet(input, Dim);

    Complex::Array2D result;

    Timer timer1("DFT compile time");
    auto k = compile(kernels::dft_decorator(input, result));
    timer1.end();

    k.load(&result, &input);
    k.call();
    output_dft(input, result, "dft");
  }


  SUBCASE("All DFT calculations should return the same") {
    bool do_profiling = true;

    if (!do_profiling) {
      // Following is enough for the unit test
      for (int N = 1; N < 4; ++N) {
        bool no_errors = compare_dfts(2*16*N, false);  // 2* for block kernels, to avoid width restrictions
        REQUIRE(no_errors);
      }
    } else {
      // Profiling: try all sizes until compilation fails
      std::cout << "DFT compare" << ProfileOutput::header();

      int Step = Platform::has_vc4()?2:4;
      int N = 1;
      bool can_continue = true;
      while (can_continue) {
        can_continue = compare_dfts(2*16*N, true);  // 2* for block kernels, to avoid width restrictions
        N += Step;
        if (N > 6*Step) break;  // Comment this out for full profiling
      }
    }
  }
}
