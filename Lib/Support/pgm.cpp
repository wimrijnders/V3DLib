#include "pgm.h"
#include <cstdio>

///////////////////////////////////////////////////////////////////////////////
// Class Color
///////////////////////////////////////////////////////////////////////////////

std::string Color::disp() const {
  std::string ret;
  ret << red << " " << green << " " << blue;
  return ret;
}


Color Color::scale(float factor) const {
  if (!(0 <= factor && factor <= 1)) {
    printf("factor: %f\n", factor);
  }
  assert(0 <= factor && factor <= 1);

  auto adjust = [factor] (int val) -> int {
   return (int) (((float) val)*factor);
  };

  return Color(adjust(red), adjust(green), adjust(blue));
}


Color Color::invert() const {
  return Color(255 - red, 255 - green, 255 - blue);
}


Color Color::operator+(Color const &rhs) const {
  return Color(rhs.red + red, rhs.green + green, rhs.blue + blue);
}


///////////////////////////////////////////////////////////////////////////////
// Class ColorMap
///////////////////////////////////////////////////////////////////////////////

Color ColorMap::calc(int value) {
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


///////////////////////////////////////////////////////////////////////////////
// File outpur function(s) 
///////////////////////////////////////////////////////////////////////////////

/**
 * These format limits need to be taken into account:
 *
 * - Max line length of 70 characters.
 * - Max gray value is 65536
 * - Max color value is 255 (standard in file header)
 */
void output_ppm_file(
  std::string const &header,
  int width,
  int height,
  const char *filename,
  std::function<std::string (int)> f) {
  assert(!header.empty());
  int const LINE_LIMIT = 70;

  FILE *fd = fopen(filename, "w") ;
  if (fd == nullptr) {
    printf("can't open file for graphics output\n");
    return;
  }

  fprintf(fd, "%s\n", header.c_str());

  std::string curLine;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      std::string tmp = f(x + width*y);

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
