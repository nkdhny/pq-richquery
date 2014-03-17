#include "time.h"

namespace nkdhny{

long gettime_ms(){
  struct timeval  tv;
  long stamp_ms;

  gettimeofday(&tv, 0);

  stamp_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  return stamp_ms;
}

}
