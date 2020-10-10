#ifndef _EXAMPLE_SUPPORT_TIMER_H
#define _EXAMPLE_SUPPORT_TIMER_H
#include <sys/time.h>

namespace QPULib {

/**
 * Simple wrapper class for outputting run time for examples
 *
 */
class Timer {
public:
	Timer();
	void end(bool show_output = true);

private:
  timeval tvStart;
};

}  // namespace


#endif  // _EXAMPLE_SUPPORT_TIMER_H
