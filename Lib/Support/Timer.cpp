///////////////////////////////////////////////////////////////////////////////
//
// NOTE: use following to add two timerval structs:
//
//     void timeradd(struct timeval *a, struct timeval *b, struct timeval *res);
//
// From man timercmp:
//
//    struct timeval {
//        time_t      tv_sec;     /* seconds */
//        suseconds_t tv_usec;    /* microseconds */
//    };
//
// - tv_usec has a value in the range 0 to 999,999.
//
///////////////////////////////////////////////////////////////////////////////
#include "Timer.h"
#include <cstddef>  // NULL
#include <cstdio>   // printf
#include "Support/debug.h"


namespace V3DLib {

Timer::Timer(std::string const &label, bool disp_in_dtor) : m_disp_in_dtor(disp_in_dtor), m_label(label) {
  gettimeofday(&tvStart, NULL);
}


Timer::~Timer() {
  if (m_disp_in_dtor) {  // Allows RAII usage
    end();
  }
}


std::string Timer::end(bool show_output) {
  assert(!m_label.empty());

  timeval tvEnd, tvDiff;
  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  if (show_output) {
    printf("%s: %ld.%06lds\n", m_label.c_str(), tvDiff.tv_sec, tvDiff.tv_usec);
  }

  char buf[128]; 
  sprintf(buf, "%ld.%06ld", tvDiff.tv_sec, tvDiff.tv_usec);
  return std::string(buf); 
}

}  // namespace
