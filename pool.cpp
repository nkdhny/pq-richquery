#include "pool.h"

namespace nkdhny {
namespace db {

PoolParams::PoolParams(int _capacity):
  capacity(_capacity)
{
  wait = 1000;
  min_idle = std::max(1, _capacity/10);
  max_idle = _capacity;
  retry = 0;

  assert(capacity>0);
}

PoolParams::PoolParams(int _capacity, int _min_idle, int _max_idle, int _retry, long _wait):
  capacity(_capacity),
  min_idle(_min_idle),
  max_idle(_max_idle),
  retry(_retry),
  wait(_wait)
{
  assert(capacity>0);
}

}
}
