#include "poolactions.h"

namespace nkdhny {
namespace db {
namespace poolactions {

PostgreCreate::PostgreCreate(std::string _host, std::string _database, std::string _role, std::string _password, int _port):
  host(_host),
  database(_database),
  role(_role),
  password(_password),
  port(_port)
{}

PGconn *PostgreCreate::operator ()()
{
  PGconn *conn = NULL;
  std::stringstream connectionString;

  connectionString << "user='"<<role<<"' password='"<<password<<"' dbname='"<<database<<"' hostaddr='"<<host<<"' port='"<<port<<"' connect_timeout=5";

  conn = PQconnectdb(connectionString.str().c_str());
  assert(conn != NULL);
  assert(PQstatus(conn) == CONNECTION_OK);
  return conn;
}

QueryValidate::QueryValidate(std::string _validator):
  validator(_validator)
{}

bool QueryValidate::operator ()(PGconn *c)
{
  if(PQstatus(c) != CONNECTION_OK) {
    return false;
  }

  Query q(c, validator);
  Result r = q();

  return r.begin() != r.end();
}

void CheckTransactionStatusPassivate::operator ()(PGconn *c)
{
  PGTransactionStatusType transaction_status = PQtransactionStatus(c);

  if(transaction_status == PQTRANS_INTRANS || transaction_status == PQTRANS_INERROR || transaction_status == PQTRANS_UNKNOWN) {
    Query rollback(c, "rollback transaction;");
    rollback();
  }
}

void FreeConnectionDestroy::operator ()(PGconn *c)
{
  PQfinish(c);
}

}
}
}
