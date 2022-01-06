
#include <iomanip>
#include <sstream>

#include "log.h"

Timer::Timer() {
  start();
}

void Timer::start() {
  struct timeval tv;
  gettimeofday (&tv, NULL);
  start_   = tv.tv_sec;
  startus_ = tv.tv_usec;
}

double Timer::elapsed() const {
  struct timeval tv;
  gettimeofday (&tv, NULL);
  long long t = (tv.tv_sec  - start_) * 1000000 + \
                (tv.tv_usec - startus_);
  return (t / 1000) / 1000.0;
}

Timer tstamp;

std::ostream& operator<<(std::ostream& os, const Timer& t) {
  std::ostringstream oss; // seperate the impact of format
  oss << "[" << std::setprecision(3) << std::setw(8) << std::fixed
      << t.elapsed() << "] ";
  os << oss.str();
  return os;
}

std::ostream& log(std::ostream& os) {
  os << tstamp;
  return os;
}
