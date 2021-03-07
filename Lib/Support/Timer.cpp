#include "Timer.h"
#include <cstddef>  // NULL
#include <cstdio>   // printf
#include "Support/debug.h"


namespace V3DLib {

Timer::Timer(std::string const &label) : m_label(label) {
  gettimeofday(&tvStart, NULL);
}


void Timer::end(bool show_output) {
  assert(!m_label.empty());

  timeval tvEnd, tvDiff;
  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  if (show_output) {
    printf("%s: %ld.%06lds\n", m_label.c_str(), tvDiff.tv_sec, tvDiff.tv_usec);
  }
}

}  // namespace
