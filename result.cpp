#include "result.h"
#include <utility>

namespace nkdhny {
namespace db {


Result::Result(const Result &other):
    owner(true),
    result(other.result)
{
    assert(other.owner);
    assert(other.result!=NULL);

    const_cast<Result&>(other).owner = false;
    const_cast<Result&>(other).result = NULL;
}

Result::Result(PGresult *_result):
    owner(true),
    result(_result)
{
    assert(_result!=NULL);
}

Result::~Result()
{
    if(isDefined()) {
        PQclear(result);
    }
    result = NULL;
}

const Result& Result::operator=(Result& other) {
    assert(other.isDefined() && !isDefined());

    std::swap(owner, other.owner);
    std::swap(result, other.result);

    return *this;
}

Row Result::begin()
{
    return Row(result, 0);
}

Row Result::end()
{
    return Row(result, PQntuples(result));
}

int Result::count()
{
    assert(isDefined());
    return PQntuples(result);
}

bool Result::isDefined()
{
    if(owner){
        assert(result != NULL);
        return owner;
    } else {
        assert(result == NULL);
        return owner;
    }
}

}
}
