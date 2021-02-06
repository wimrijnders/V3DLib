#ifndef _EXAMPLE_SUPPORT_PGM_H
#define _EXAMPLE_SUPPORT_PGM_H
#include <cstdio>


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


#endif  // _EXAMPLE_SUPPORT_PGM_H
