#include "connection.h"

namespace nkdhny {
namespace db {

void CloseConnection::operator ()(PGconn *connection)
{
    PQfinish(connection);
}

}
}
