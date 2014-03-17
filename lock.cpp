#include "lock.h"

namespace nkdhny{

Mutex::Mutex() 
{
  pthread_mutex_init(&this->m_mutex, NULL);
}

Mutex::~Mutex()
{
  pthread_mutex_destroy(&this->m_mutex);
}

void Mutex::Lock()
{
  pthread_mutex_lock(&this->m_mutex);
}

void Mutex::Unlock()
{
  pthread_mutex_unlock(&this->m_mutex);
}

Lock::Lock(Mutex& mutex)    
{
  _ref = const_cast<Mutex *>(&mutex);
  _ref->Lock();
}

Lock::~Lock()
{
  _ref->Unlock();
}


}
