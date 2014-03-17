#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <assert.h>
#include "query.h"

namespace nkdhny{
namespace db{

/** Transaction is started in constructor
  * and rolled back if uncommited in destructor
  * Thus one must explicitly commit it by calling
  * `commit` method */
class Transaction
{
private:
    Transaction();
    Transaction(const Transaction&);
    Transaction& operator=(Transaction&);

    PGconn* connection;
    bool uncommited;
public:
    Transaction(PGconn* conn);
    ~Transaction();
    void commit();
};

}
}

#endif // TRANSACTION_H
