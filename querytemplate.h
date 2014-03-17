#ifndef QUERYTEMPLATE_H
#define QUERYTEMPLATE_H

#include <string>
#include <vector>
#include <postgresql/libpq-fe.h>
#include "assert.h"
#include "parambuilder.h"
#include "result.h"
#include "query.h"
#include <stdlib.h>
#include <time.h>
#include <sstream>

namespace nkdhny {
namespace db {

/** libpq wrapper class inspired by Spring JdbcTemplate class
  * it compiles query (see constructor)
  * than one could bind parameters to a compiled query (see `pushParameter` method)
  * after parameters are bound one could execute query (see `operator ()`)
  */
class QueryTemplate
{
private:
    std::string name;
    PGconn* connection;
    ParamBuilder parameters;

    QueryTemplate(const QueryTemplate&);
    const QueryTemplate& operator =(const QueryTemplate&);

    bool checkParametersAreConsistentToQuery();
    bool checkIfAlreadyExists();

public:
    /** @brief compiles given `query` in the context of
      * `_connection` and stores it in DB with name `_name`
      * if no name is given explicitly it will be (randomly)
      * guessed*/
    QueryTemplate(PGconn* _connection, const std::string& query, const std::string _name="");

    /** @brief bind parameter of type `T` with value `value`
      * to a subsequent query parameter. See `ParamBuilder` on
      * how to extend this to User Defined Datatypes */
    template <typename T>
    QueryTemplate& pushParameter(T value);

    /** executes query and clears its parameter list
      * One could bund new parameters to this query and execute it
      * onece more
      */
    Result operator()();


};


template <typename T>
QueryTemplate& QueryTemplate::pushParameter(T value) {
    parameters.push<T>(value);
    return *this;
}

}
}

#endif // QUERYTEMPLATE_H
