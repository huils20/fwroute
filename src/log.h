#include <iostream>
#include <string>
#include <sys/time.h>

class Timer {
 public:
  Timer();
  void start();
  double elapsed() const;

 private:
  time_t start_;
  time_t startus_;
};

std::ostream& operator<<(std::ostream& os, const Timer& t);
std::ostream& log(std::ostream& os = std::cout);

