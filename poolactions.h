#ifndef POOLACTIONS_H
#define POOLACTIONS_H

#include "query.h"

namespace nkdhny {
namespace db {
namespace poolactions {

/** libpq connection factory, creates a new connection*/
struct PostgreCreate: std::unary_function<void, PGconn*> {
  std::string host;
  std::string database;
  std::string role;
  std::string password;
  int port;

  PostgreCreate(std::string _host, std::string _database, std::string _role, std::string _password, int _port);

  PGconn* operator()();
};

/** validation functor. it checks that connection status is fine
  * and it is possible to make a simple query with this connection
  */
struct QueryValidate: std::unary_function<PGconn*, bool> {
  std::string validator;

  QueryValidate(std::string _validator);

  bool operator ()(PGconn* c);
};

/** nothing to do with connection befor borrowing it to a client */
struct StubActivate: std::unary_function<PGconn*, void> {
  void operator ()(PGconn*) const {}
};

/** Action to passivate a connection based on transaction status associated with the connection
  * if transaction is finished successfully - nothing to do
  * if transaction was started and not finished or transaction is in error
  * or has unknown status transaction will rolled back */
struct CheckTransactionStatusPassivate:std::unary_function<PGconn*, void> {
  void operator()(PGconn* c);
};

/** libpq connection free*/
struct FreeConnectionDestroy: std::unary_function<PGconn*, void> {
  void operator()(PGconn* c);
};

}
}
}


#endif // POOLACTIONS_H
