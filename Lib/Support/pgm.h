#ifndef _EXAMPLE_SUPPORT_PGM_H
#define _EXAMPLE_SUPPORT_PGM_H
#include <cstdio>
#include <string>
#include "Support/basics.h"

class Color {
public:
  Color(int intensity) : red(intensity), green(intensity), blue(intensity) {}
  Color(int r, int g, int b) : red(r), green(g), blue(b) {}

  std::string disp() const {
    std::string ret;
    ret << red << " " << green << " " << blue;
    return ret;
  }

  Color scale(float factor) const {
    if (!(0 <= factor && factor <= 1)) {
      printf("factor: %f\n", factor);
    }
    assert(0 <= factor && factor <= 1);

    auto adjust = [factor] (int val) -> int {
     return (int) (((float) val)*factor);
    };

    return Color(adjust(red), adjust(green), adjust(blue));
  }

  Color invert() const {
    return Color(255 - red, 255 - green, 255 - blue);
  }

  Color operator+(Color const &rhs) const {
    return Color(rhs.red + red, rhs.green + green, rhs.blue + blue);
  }

private:
  int red;
  int green;
  int blue;
};


/**
 * Output a PGM file from the supplied array
 *
 * This will be a graymap (as the 'G' implies in PGM). 
 *
 * Defined as a template so that it can handle ShardeArray
 * as well as regular arrays.
 *
 * Also, set up with casts so that it can handle int as well as
 * float arrays as input.
 *
 *
 * Two format limits need to be taken into account:
 *
 * - Max line length of 70 characters, 'count' handles this
 * - Max gray value of 65536
 */
template<class Array>
void output_pgm_file(Array &arr, int width, int height, int maxGray, const char *filename) {
  const int GrayLimit = 65536;
  float factor = -1.0f;

  if (maxGray > GrayLimit) {
    // printf ("output_pgm adjust max gray\n");
    factor = ((float) GrayLimit)/((float) maxGray);
    maxGray = GrayLimit;
  }

  auto scale = [factor, maxGray] (float value) -> int {
    if (value < 0.0f) value = 0;
    if (factor == -1.0f) return (int) value;
    int ret = (int) (factor*((float) value));
    if (ret > maxGray) ret = maxGray;
    return ret;
  };


  FILE *fd = fopen(filename, "w") ;
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
      fprintf(fd, "%d ", scale((float) (arr[x + width*y])));
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


/**
 * Inspiration: http://warp.povusers.org/Mandelbrot/, "Coloring the image"
 */
class ColorMap {
public:
 ColorMap(int in_max) : max_intensity(in_max) { }

 Color calc(int value) {
   assert(0 <= value && value <= max_intensity);
   float fraction = ((float) value)/((float) max_intensity);

   if (value == max_intensity) return Color(0);

   if (fraction <= peak) {
     return main_color.scale(fraction/peak);
   } else {
     float fraction2 = (fraction - peak)/(1 - peak);
     return main_color + main_color.invert().scale(fraction2);
   }
 }

private:
 int max_intensity;
 float peak = 0.1f;

 Color main_color = Color(255, 255, 0);
};


template<class Array>
void output_ppm_file(Array &arr, int width, int height, int maxGray, const char *filename) {
  int const LINE_LIMIT = 70;
  const int GrayLimit = 255;
  float factor = -1.0f;
  ColorMap map(maxGray);

  if (maxGray > GrayLimit) {
    // printf ("output_pgm adjust max gray\n");
    factor = ((float) GrayLimit)/((float) maxGray);
    maxGray = GrayLimit;
  }

  auto scale = [factor, maxGray] (int value) -> int {
    if (value < 0) value = 0;
    if (factor == -1.0f) return (int) value;
    int ret = (int) (factor*((float) value));
    if (ret > maxGray) ret = maxGray;
    return ret;
  };


  FILE *fd = fopen(filename, "w") ;
  if (fd == nullptr) {
    printf("can't open file for pgm output\n");
    return;
  }

  // Write header
  fprintf(fd, "P3\n");
  fprintf(fd, "%d %d\n", width, height);
  fprintf(fd, "%d\n", 255);

  std::string curLine;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      //Color col = map.calc(scale(arr[x + width*y]));
      Color col = map.calc(arr[x + width*y]);
      std::string tmp = col.disp();

      std::string newLine = curLine;
      if (!newLine.empty()) {
        newLine << " ";
      }
      newLine << tmp;

      if (newLine.length() > LINE_LIMIT) {
        fprintf(fd, "%s\n", curLine.c_str());
        curLine = tmp;
      } else {
        curLine = newLine;
      }
    }
  }

  if (!curLine.empty()) {
    fprintf(fd, "%s\n", curLine.c_str());
  }

  fclose(fd);
}


#endif  // _EXAMPLE_SUPPORT_PGM_H
