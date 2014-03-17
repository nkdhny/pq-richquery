#include "transaction.h"

namespace nkdhny{
namespace db{

Transaction::Transaction(PGconn *conn):
    connection(conn),
    uncommited(true)
{
    Query begin(connection, "BEGIN TRANSACTION;");
    begin();
}

Transaction::~Transaction()
{
    if(uncommited){
        Query rollback(connection, "ROLLBACK TRANSACTION;");
        rollback();
    }
}

void Transaction::commit()
{
    assert(uncommited);
    Query commit(connection, "COMMIT TRANSACTION;");
    commit();
    uncommited = false;
}


}
}
