///////////////////////////////////////////////////////////////////////////////
// Tests for Discrete Fourier Transform (DFT)
///////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "Support/Timer.h"
#include "Source/Complex.h"
#include "support/support.h"
#include "support/matrix_support.h"
#include "Kernel.h"
#include "Kernels/Matrix.h"
#include "LibSettings.h"
#include "support/ProfileOutput.h"

using namespace V3DLib;

namespace {

int const DimTest = 128;

void create_test_wavelet(Complex::Array2D &input, int const Dim) {
  REQUIRE(input.columns() == Dim);

  float offset = 0.1f;

  float freq_filter =  0.5f / ((float) Dim);
  float freq1       =  1.0f / ((float) Dim);
  float freq2       = 45.0f / ((float) Dim);

  for (int c = 0; c < Dim; ++c) {
    float filter = functions::sin(freq_filter*((float) c), true);
    float noise = 0.3f *random_float();
    float val1  = 1.0f *functions::sin(freq1*((float) c), true);
    float val2  = 0.75f*functions::sin(freq2*((float) c), true);

    input[0][c] = complex(offset + noise + (filter*filter)*(val1 + val2), 0.0f);
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
 * @return  true if all kernels compiled something,
 *          false if any kernel failed compilation for vc4 as well as v3d
 */ 
bool compare_dfts(int Dim, bool do_profiling) {
  REQUIRE(Dim > 0);
  REQUIRE(Dim % 16 == 0);
  REQUIRE(ProfileOutput::num_iterations > 0);

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

  //
  // Run the kernels
  //
  int compiled = 0;

  {
    std::string label = "matrix mult";

    // Do regular complex matrix multiplication
    // In this call, the matrix and input are switched.
    // This is slightly more efficient and should not affect the result
    Timer timer1;
    auto k = compile(kernels::complex_matrix_mult_decorator(input, dft_matrix, result_mult), for_platform);
    profile_output.add_compile(label, timer1, Dim);

    if (!k.has_errors()) {
      compiled += 1;
      k.load(&result_mult, &input, &dft_matrix);
      profile_output.run(k, Dim, label);
    }
  }

  {
    std::string label = "inline complex";

    Timer timer1;
    auto k = compile(kernels::dft_inline_decorator(input, result_complex), for_platform);
    profile_output.add_compile(label, timer1, Dim);

    if (!k.has_errors()) {
      compiled += 2;
      k.load(&result_complex, &input);
      profile_output.run(k, Dim, label);
      output_dft(input, result_complex, "dft_inline_complex");
    }
  }

  {
    std::string label = "inline float";

    Timer timer1;
    auto k = compile(kernels::dft_inline_decorator(input_float, result_float), for_platform);
    //k.pretty(false, "obj/test/dft_inline_float_v3d_hardware_sin.txt");
    //k.dump_compile_data(false, "obj/test/dft_inline_float_v3d_hardware_sin_data.txt");
    profile_output.add_compile(label, timer1, Dim);

    if (!k.has_errors()) {
      compiled += 4;
      k.load(&result_float, &input_float);
      profile_output.run(k, Dim, label);
      output_dft(input, result_float, "dft_inline_float");
    }
  }

  //std::cout << result_mult.dump() << std::endl;
  //std::cout << result_complex.dump() << std::endl;

  if (do_profiling) {
      std::cout << profile_output.dump();
  } else {
    REQUIRE(compiled == 7);  // All bits for all kernels should be set

//    std::cout << result_mult.dump();
//    std::cout << result_float.dump();
//    std::cout << result_complex.dump();

    INFO("Dimension: " << Dim);
    INFO("Comparing inline complex with inline float");
    compare_arrays(result_complex, result_float, 0.0f);  // Match should be exact

    INFO("Comparing inline complex with mult");
    // No need to compare inline float with mult, identical to inline complex (see previous)");
    compare_arrays(result_mult, result_complex);
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

    auto k = compile(kernels::complex_matrix_mult_decorator(dft_conjugate, dft_matrix, result));
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
      auto k = compile(kernels::complex_matrix_mult_decorator(dft_matrix, input, result_tmp));

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
      auto k = compile(kernels::complex_matrix_mult_decorator(input, dft_matrix, result_switched));

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
//  Platform::use_main_memory(true);

  SUBCASE("Check DFT with inline sin/cos") {
    int const Dim = 16*2;  // max vc4: 16*4. Max v3d is higher, at least 64*8

    Complex::Array2D input(1, Dim);  // Create input; remember, transposed!
    create_test_wavelet(input, Dim);

    Complex::Array2D result;

    Timer timer1("DFT compile time");
    auto k = compile(kernels::dft_inline_decorator(input, result));
    timer1.end();
/*
    k.pretty(true,  "obj/test/dft_inline_vc4.txt", false);
    k.dump_compile_data(true, "obj/test/dft_compile_data_vc4.txt");
    k.pretty(false, "obj/test/dft_inline_v3d.txt", false);
*/
    std::cout << k.compile_info() << std::endl;

    k.load(&result, &input);
    k.call();

    output_dft(input, result, "dft_inline");
  }

  SUBCASE("All DFT calculations should return the same") {
    bool do_profiling = false;

    if (!do_profiling) {
      // Following is enough for the unit test
      for (int N = 1; N < 4; ++N) {
        bool no_errors = compare_dfts(16*N, false);
        REQUIRE(no_errors);
      }
    } else {
      // Profiling: try all sizes until compilation fails
      std::cout << "DFT compare" << ProfileOutput::header();

      int Step = Platform::has_vc4()?2:4;
      int N = 1;
      bool can_continue = true;
      while (can_continue) {
        can_continue = compare_dfts(16*N, true);
        N += Step;
        if (N > 3) break;
      }
    }
  }

//  Platform::use_main_memory(false);
}
