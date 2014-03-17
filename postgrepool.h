#ifndef POSTGREPOOL_H
#define POSTGREPOOL_H

#include "pool.h"
#include "poolactions.h"

namespace nkdhny {
namespace db {

struct PostgreConnectionParams {
  std::string host;
  std::string database;
  std::string role;
  std::string password;
  int port;

  PostgreConnectionParams();
  bool operator == (const PostgreConnectionParams& other);
};

/** specification of a generic pool with postgre factory validator and passivate action*/
class PostgrePool: public Pool<poolactions::PostgreCreate, poolactions::QueryValidate, poolactions::StubActivate, poolactions::CheckTransactionStatusPassivate, poolactions::FreeConnectionDestroy>
{
public:
  PostgrePool(PostgreConnectionParams connectionParams, PoolParams params);
};


}
}

#endif // POSTGREPOOL_H
