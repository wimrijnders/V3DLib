#include "Timer.h"
#include <cstddef>  // NULL
#include <cstdio>   // printf


namespace V3DLib {

Timer::Timer() {
  gettimeofday(&tvStart, NULL);
}


void Timer::end(bool show_output) {
  timeval tvEnd, tvDiff;
  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

	if (show_output) {
	  printf("Run time: %ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);
	}
}

}  // namespace
