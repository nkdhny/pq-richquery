#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <vector>
#include <postgresql/libpq-fe.h>
#include "assert.h"
#include "parambuilder.h"
#include "result.h"
#include <stdlib.h>
#include <time.h>
#include <sstream>


namespace nkdhny{
namespace db{

class Query
{
private:
    std::string query;
    PGconn* connection;
    ParamBuilder parameters;

    Query(const Query&);
    const Query& operator =(const Query&);

public:
    /** @brief stores a `_query` in object later on one could
      * provide parameters to this query and execute it. No comunication
      * to DB is done prior to query execution.
      * Query will be compiled each time it executed, thus for repeated
      * queries consider `nkdhny::QueryTemplate`*/
    Query(PGconn* _connection, const std::string& _query);

    /** @brief bind parameter of type `T` with value `value`
      * to a subsequent query parameter. See `ParamBuilder` on
      * how to extend this to User Defined Datatypes */
    template <typename T>
    Query& pushParameter(T value);

    /** executes query and clears its parameter list
      * One could bund new parameters to this query and execute it
      * onece more. On the db side it will be treated as absolutly new
      * query, i.e. it will be compiled once more. For repeated queries consider
      * `nkdhny::QueryTemplate`
      */
    Result operator()();

};

template <typename T>
Query& Query::pushParameter(T value) {
    parameters.push<T>(value);
    return *this;
}


}
}


#endif // QUERY_H
