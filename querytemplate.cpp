#include "querytemplate.h"


namespace nkdhny {
namespace db {


static std::string guesNameForQuery(const std::string& query) {
    std::stringstream ss;
    ss<<rand();

    return ss.str();
}

struct ParameterCounter {
    int* result;
    void operator()(const char symbol) {
        if(symbol == '1') {
            ++(*result);
        }
    }
};

static int countParameters(const std::string& query) {

    int result;
    ParameterCounter counter;

    counter.result = &result;

    std::for_each(query.begin(), query.end(), counter);

    return result;
}


QueryTemplate::QueryTemplate(PGconn *_connection, const std::string& query, const std::string _name):
    parameters(),
    connection(_connection),
    name(_name)
{
    if(name.empty()){
        name = guesNameForQuery(query);
    }

    if(!checkIfAlreadyExists()) {

        PGresult* prepared_result = PQprepare(connection, name.c_str(), query.c_str(), countParameters(query), NULL);
        assert(PQresultStatus(prepared_result) == PGRES_COMMAND_OK);
        PQclear(prepared_result);
    }

}

Result QueryTemplate::operator ()()
{

    assert(checkParametersAreConsistentToQuery());
    PGresult* result = PQexecPrepared(connection, name.c_str(), parameters.count(), parameters.values().data(), parameters.sizes().data(), parameters.formats().data(), /*binary*/ 1);
    parameters.clear();
    Result r = Result(result);

    return r;
}

bool QueryTemplate::checkParametersAreConsistentToQuery()
{
    PGresult* result = PQdescribePrepared(connection, name.c_str());
    assert(PQresultStatus(result) == PGRES_COMMAND_OK);
    bool r = (parameters.count() == PQnparams(result));
    PQclear(result);
    return r;
}

bool QueryTemplate::checkIfAlreadyExists()
{

    Query check_statement_exists(connection,"select * from pg_prepared_statements where name like $1");
    check_statement_exists.pushParameter(name);
    Result result = check_statement_exists();
    int statementsCount = result.count();
    assert(statementsCount<2 && statementsCount>=0);
    bool r = statementsCount==1;    
    return r;
}

}
}
