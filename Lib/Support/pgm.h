#ifndef _EXAMPLE_SUPPORT_PGM_H
#define _EXAMPLE_SUPPORT_PGM_H
#include <cmath>
#include <string>
#include <functional>
#include "Support/basics.h"


class Color {
public:
  Color(int intensity) : red(intensity), green(intensity), blue(intensity) {}
  Color(int r, int g, int b) : red(r), green(g), blue(b) {}

  std::string disp() const;
  Color scale(float factor) const;
  Color invert() const;
  Color operator+(Color const &rhs) const;

private:
  int red;
  int green;
  int blue;
};


/**
 * Inspiration: http://warp.povusers.org/Mandelbrot/, "Coloring the image"
 */
class ColorMap {
public:
 ColorMap(int in_max) : max_intensity(in_max) { }

 Color calc(int value);

private:
 int max_intensity;
 float peak = 0.2f;

 Color main_color = Color(128, 128, 255);
};


void output_ppm_file(
  std::string const &header,
  int width,
  int height, 
  const char *filename,
  std::function<std::string (int)> f);


/**
 * Output a PGM (greyscale) bitmap from the supplied array
 *
 * Defined as a template so that it can handle SharedArray
 * as well as regular arrays.
 *
 * Also, set up with casts so that it can handle int as well as
 * float arrays as input.
 */
template<class Array>
void output_pgm_file(Array &arr, int width, int height, int in_max_value, const char *filename) {
  assert(in_max_value >= 1);

  int const GrayLimit    = 1024;       // largest allowed value in pgm file is 65536 (-1?)
  int const LINEAR_LIMIT = 128;        // Use log scale above this number of iterations
  float factor           = 1.0f;
  bool do_log            = (in_max_value > LINEAR_LIMIT);
  float max_value        = (float) in_max_value;

  if (do_log) {
    max_value = (float) log2((float) in_max_value);
  }
  assert(max_value >= 1.0f);

  factor = ((float) GrayLimit)/max_value;

  auto scale = [factor, do_log, in_max_value] (float value) -> int {
    if (value  <= 0) return 0;
    if (value  == (float) in_max_value) return 0;

    if (do_log) {
      value = (float) log2(value);
    }

    int ret = (int) (factor*value);
    if (ret <= 0) return 0;
    if (ret > GrayLimit) ret = GrayLimit;

    return ret;
  };

  std::string header;
  header << "P2\n"
         << width << " " << height << "\n"
         << GrayLimit << "\n";


  output_ppm_file(header, width, height, filename, [scale, &arr] (int index) -> std::string {
    std::string ret;
    ret << scale((float) arr[index]);
    return ret;
  });
}


/**
 * Output a PPM (color) bitmap from the supplied array
 */
template<class Array>
void output_ppm_file(Array &arr, int width, int height, int max_value, const char *filename) {
  ColorMap map(max_value);

  std::string header;
  header << "P3\n"
         << width << " " << height <<"\n"
         << 255 << "\n";

  output_ppm_file(header, width, height, filename, [&map, &arr] (int index) -> std::string {
    Color col = map.calc(arr[index]);
    return col.disp();
  });
}


class PGM {
public:
  PGM(int width, int height);
  ~PGM();

  PGM &plot(float const *arr, int size, int color = MAX_COLOR);
  void save(char const *filename);

private:
  static int const MAX_COLOR = 127;

  int m_width  = 0;
  int m_height = 0;
  int *m_arr   = nullptr;
};

#endif  // _EXAMPLE_SUPPORT_PGM_H
