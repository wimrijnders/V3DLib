#ifndef _EXAMPLE_SUPPORT_TIMER_H
#define _EXAMPLE_SUPPORT_TIMER_H
#include <sys/time.h>
#include <string>

namespace V3DLib {

/**
 * Simple wrapper class for outputting run time for examples
 */
class Timer {
public:
  Timer(std::string const &label = "Run time", bool disp_in_dtor = false);
  ~Timer();
  std::string end(bool show_output = true);

private:
  bool m_disp_in_dtor = false;
  std::string m_label;
  timeval tvStart;
};

}  // namespace


#endif  // _EXAMPLE_SUPPORT_TIMER_H
