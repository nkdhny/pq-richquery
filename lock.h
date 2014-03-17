#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>

namespace nkdhny{

class Mutex
{
public:
  Mutex();
  ~Mutex();

  void Lock();
  void Unlock();

private:
  pthread_mutex_t m_mutex;
};

class Lock
{
public:
  Lock(Mutex& mutex);
  ~Lock();

private:
  Lock(const Lock&);
  Mutex * _ref;
};

}

#endif //LOCK_H

