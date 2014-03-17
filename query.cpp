#include "query.h"

namespace nkdhny{
namespace db{

Query::Query(PGconn *_connection, const std::string &_query):
    connection(_connection),
    query(_query)
{}

Result Query::operator ()()
{

    PGresult* result = PQexecParams(connection, query.c_str(), parameters.count(), NULL, parameters.values().data(), parameters.sizes().data(), parameters.formats().data(), /*binary*/ 1);
    parameters.clear();
#ifdef DEBUG
    int query_status = PQresultStatus(result);
    assert(query_status == PGRES_TUPLES_OK || query_status ==PGRES_COMMAND_OK);
#endif
    Result r = Result(result);

    return r;
}



}
}
