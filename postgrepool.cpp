#include "postgrepool.h"

namespace nkdhny {
namespace db {

PostgrePool::PostgrePool(PostgreConnectionParams connectionParams, PoolParams params):
  Pool(params, poolactions::PostgreCreate(connectionParams.host, connectionParams.database, connectionParams.role, connectionParams.password, connectionParams.port), poolactions::QueryValidate("select 1"), poolactions::StubActivate(), poolactions::CheckTransactionStatusPassivate(), poolactions::FreeConnectionDestroy())
{}

bool PostgreConnectionParams::operator ==(const PostgreConnectionParams &other)
{
    return
        host == other.host &&
        database == other.database &&
        role == other.role &&
        password == other.password &&
        port == other.port;
}

PostgreConnectionParams::PostgreConnectionParams():
  host(""),
  database(""),
  role(""),
  password(""),
  port(0)
{}

}
}
