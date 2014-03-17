#ifndef POOL_H
#define POOL_H

#include "connection.h"
#include <queue>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include "lock.h"
#include "time.h"

namespace nkdhny {

namespace db {

struct PoolParams {
  /** time to wait for borrowed connection to return to the pool */
  long wait;
  /** total capacity of the pool */
  int capacity;
  /** minimum count of pre initialized connections in pool if has enough capacity */
  int min_idle;
  /** maximum count of pre initialized connections in pool if has enough capacity */
  int max_idle;
  /** count to try recreate connection if freshly created one is NULL or not valid*/
  int retry;

  explicit PoolParams(int _capacity);
  PoolParams(int _capacity, int _min_idle, int _max_idle, int _retry, long _wait);

};

/** Exception to be thrown when all pooled objects are borrowed to clients
  * and one fails to borrow one more
*/
struct PoolIsEmpty{};
/** Exception to be thrown when pool was not able to create fresh
  * not NULL and valid connection after `retry` repeats
  */
struct PoolCouldNotCreateValidConnection{};

/** Action to be used for `nkdhny::db::Connection` to return its
  * underlying `PGcon*` to pool instead of closing it
  * @see nkdhny::db::Connection
  */
template <typename C, typename V, typename A, typename P, typename D> struct ReturnToPoolAction;

/**
  * Pool class is parametrized via following type parameters:
  * - Create: std::unary_function<void, PGcon*> how to create action
  * - Validate: std::unary_function<PGcon*, bool> check if connection is fine and could be returned to client
  * - Activate: std::unary_function<PGcon*, void> what to do before returning connection to client
  * - Passivate: std::unary_function<PGcon*, void> what to do before return connection to pool
  * - Destroy: std::unary_function<PGcon*, void> how to close the connection
  *
  * Main loop is as follows:
  * - cliaent asks pool for a object
  * - pool checks if it has idle one if no wait for a `wait`ms timeout periodicaly checkking if idle connection appeared, if no exception `PoolIsEmpty` is throwed
  * - pool validates object with `Validate` action if validation faild object is closed with `Destroy` action and recreated with `Create` action and validated again
  * - pool activates object with `Activate` action
  * - client obtains `PooledConnection` object
  * - when destroyed `PooledConnection` will returned back to its pool by calling `Pool::push()` function
  * - pool wil passivate object with `Passivate` action
  *
  * New object is created as follows:
  * - pool creates a fresh object with `Create` action
  * - check that object is not `NULL` and valid, if valid - done
  * - if not - close object and try once more
  * - do it no more than `retry` times
  * - if was not able to create good connection - throw an exception `PoolCouldNotCreateValidConnection`
  *
  * Consistency checking and thread safety
  * If it is possible pool tryes to maintain at least `min_idle` but no more than `max_idle` connections
  * this is done by calling `Pool::heat` and 'Pool::freeze' methods
  * Main loop is syncronized, it means:
  * - at a given time pool will give connection to only one client (instead of checking if connection is available, this action is concurrent)
  * - at a given time only one cllient could return connection to a pool
  * Constructor is not syncronized
  * it means that one could obtain inconsistent-partially constructed object when tryies to borrow object in concurrent thread just after creation of the pool
*/
template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
class Pool
{
protected:
  long wait;
  int capacity;
  int min_idle;
  int max_idle;
  int retry;

  Create createAction;
  Validate validateAction;
  Activate activateAction;
  Passivate passivateAction;
  Destroy destroyAction;

  int in_use_count;
  std::queue<PGconn*> idle_connections;

  /**
    * no default constructor no copy
    */
  Pool();
  Pool(const Pool&);
  const Pool& operator=(const Pool&);  

  int count();
  int idle();

  /** object cration loop */
  PGconn* create() throw (PoolCouldNotCreateValidConnection);
  void destroy(PGconn *c);

  /** consistenty maintenance functions*/
  void heat() throw (PoolCouldNotCreateValidConnection);
  void freeze();

  bool consistent();

  Mutex lock;

protected:
  /** connactions ae going to be pusshed back not by client directly,
    * but by destructor of the `PooledConnection` class thus it is protected
    */
  void push(PGconn* c);

public:

  /** `PooledConnection` is a specification of `nkdhny::db::Connection` class that will be returned to the owner pool
    * after destruction */
  typedef Connection<ReturnToPoolAction<Create, Validate, Activate, Passivate, Destroy > > PooledConnection;

  Pool(const PoolParams& params, Create _create, Validate _validate, Activate _activate, Passivate _passivate, Destroy _destroy) throw (PoolCouldNotCreateValidConnection);
  ~Pool();

  /** Main loop */
  PooledConnection borrow() throw (PoolIsEmpty,  PoolCouldNotCreateValidConnection);
  /** same as borrow */
  PooledConnection operator ()() throw (PoolIsEmpty, PoolCouldNotCreateValidConnection);

  /** action to be used for closing `PooledConnection` see bellow
    */
  friend struct ReturnToPoolAction<Create, Validate, Activate, Passivate, Destroy>;
};

/** action to be used for closing `PooledConnection` connections
  * `ReturnToPoolAction` tracks pointer to a pool that borrowed a connection and
  * calls `Pool::push()` in destructor of the connection
  */
template <typename C, typename V, typename A, typename P, typename D>
struct ReturnToPoolAction: std::unary_function<PGconn*, void> {
  Pool<C, V, A, P, D>* pool;
  explicit ReturnToPoolAction(Pool<C, V, A, P, D>* _pool = NULL);
  void operator ()(PGconn* c);
};

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
int Pool<Create, Validate, Activate, Passivate, Destroy>::count()
{
  assert(in_use_count <= capacity);

  return capacity - in_use_count;
}

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
int Pool<Create, Validate, Activate, Passivate, Destroy>::idle()
{
  assert(idle_connections.size() <= capacity);

  return idle_connections.size();
}

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
PGconn* Pool<Create, Validate, Activate, Passivate, Destroy>::create()  throw (PoolCouldNotCreateValidConnection)
{
  PGconn* c = NULL;

  for(int i = 0; i <= retry; i++) {
    c = createAction();
    if(c == NULL){
      continue;
    } else {
      if(validateAction(c)) {
        return c;
      } else {
        destroyAction(c);
      }
    }
  }

  throw PoolCouldNotCreateValidConnection();
}

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
void Pool<Create, Validate, Activate, Passivate, Destroy>::destroy(PGconn* c)
{
  destroyAction(c);
}

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
void Pool<Create, Validate, Activate, Passivate, Destroy>::heat() throw (PoolCouldNotCreateValidConnection)
{
  int lack = std::min(min_idle - idle(), count() - idle());

  if(lack<=0) {
    return;
  }

  for(int i =0; i< lack; i++) {
    idle_connections.push(create());
  }

  assert(idle() <= capacity);
  assert(idle() >= 0);
}

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
void Pool<Create, Validate, Activate, Passivate, Destroy>::freeze()
{
  int excess = idle() - max_idle;
  if(excess<=0){
    return;
  }

  for(int i = excess; i>0; i--) {
    PGconn* c = idle_connections.front();
    idle_connections.pop();
    destroy(c);
  }
}

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
bool Pool<Create, Validate, Activate, Passivate, Destroy>::consistent()
{
  bool idle_ok = idle()>=min_idle && idle()<=max_idle || count() < min_idle;
  bool size_ok = (idle()+in_use_count) <= capacity;
  bool used_ok = in_use_count>=0 && in_use_count <= capacity;

  return size_ok && idle_ok && used_ok;
}
template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
void Pool<Create, Validate, Activate, Passivate, Destroy>::push(PGconn *c)
{

  passivateAction(c);
  {
    volatile Lock _lock(lock);

    assert(consistent());

    --in_use_count;
    idle_connections.push(c);
    freeze();

    assert(consistent());
  }
}
template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
Pool<Create, Validate, Activate, Passivate, Destroy>::Pool(const PoolParams &params, Create _create, Validate _validate, Activate _activate, Passivate _passivate, Destroy _destroy)  throw (PoolCouldNotCreateValidConnection):
  wait(params.wait),
  capacity(params.capacity),
  min_idle(params.min_idle),
  max_idle(params.max_idle),
  retry(params.retry),
  in_use_count(0),
  idle_connections(),
  createAction(_create),
  validateAction(_validate),
  activateAction(_activate),
  passivateAction(_passivate),
  destroyAction(_destroy)
{
  heat();
}

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
Pool<Create, Validate, Activate, Passivate, Destroy>::~Pool()
{
  assert(in_use_count == 0);

  while(!idle_connections.empty()){
    destroyAction(idle_connections.front());
    idle_connections.pop();
  }
}

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
typename Pool<Create, Validate, Activate, Passivate, Destroy>::PooledConnection Pool<Create, Validate, Activate, Passivate, Destroy>::borrow()  throw (PoolIsEmpty,  PoolCouldNotCreateValidConnection)
{
  long started = gettime_ms();
  long will_end = started+wait;
  long sleep_for = wait/10;

  do {
    if(idle()>0) {

      volatile Lock _lock(lock);

      assert(consistent());

      PGconn* c = idle_connections.front();
      idle_connections.pop();

      if(!validateAction(c)) {
        destroyAction(c);
        c = create();
      }

      activateAction(c);
      ++in_use_count;

      heat();

      assert(consistent());

      return PooledConnection(c, ReturnToPoolAction<Create, Validate, Activate, Passivate, Destroy>(this));
    }

    usleep(sleep_for*1000); //sleep_for is in milliseconds

  } while(gettime_ms() < will_end);

  throw PoolIsEmpty();
}

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
typename Pool<Create, Validate, Activate, Passivate, Destroy>::PooledConnection Pool<Create, Validate, Activate, Passivate, Destroy>::operator ()()  throw (PoolIsEmpty,  PoolCouldNotCreateValidConnection)
{
  return borrow();
}

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
ReturnToPoolAction<Create, Validate, Activate, Passivate, Destroy>::ReturnToPoolAction(Pool<Create, Validate, Activate, Passivate, Destroy>* _pool):
  pool(_pool)
{}

template <typename Create, typename Validate, typename Activate, typename Passivate, typename Destroy>
void ReturnToPoolAction<Create, Validate, Activate, Passivate, Destroy>::operator ()(PGconn *c)
{
  if(pool !=NULL){
    pool->push(c);
  }
}

}
}


#endif // POOL_H
