#ifndef PARAMBUILDER_H
#define PARAMBUILDER_H

#include <vector>
#include <memory>
#include <algorithm>

#include "parameter.h"

namespace nkdhny{
namespace db{

/** The ParamBuilder class
  * is a builder of query parameter list
  * Inside it uses bunary representation of each concrete type
  * Thus one should explicitly specify template method
  * `push` (see bellow) with ones own type.
  * Builder is non copyable
  * Builder could be used separate from template class, like this
  * @verbatim
  *     int foo_id = 1;
  *     ParamBuilder b;
  *     b.push(f);
  *     PGresult* result = PQexecParams(connection, "select * from foo where id = $1", b.count(), NULL, b.values().get(), b.sizes().get(), b.formats().get(), Parameter::BINARY_FORMAT);
  */
class ParamBuilder
{
private:

    ParamBuilder(const ParamBuilder&);
    ParamBuilder& operator=(const ParamBuilder&);

    std::vector<Parameter*> params;


public:
    /**
      * type shortcuts to be used for list of values, parameter sizes and formats
      * see libpq `ExecParams` method documentation for details
      */
    typedef char *  Values;
    typedef int     Sizes;
    typedef int     Formats;

    /** @brief Constructs an empty builder */
    ParamBuilder();
    ~ParamBuilder();

    /** Add copy of `_value` to my parameter list. Parameters are represented as a
      * pointers to a derivatives of a `Parameter` class
      * The template method is stub in sence that it has only specializations for common and domain types.
      * No generic method is defined.
      * When one wants to push some type `Foo` to a query one has several options:
      * (i) define `push` method through existing parameter representation (see `Parameter` class). For example
      * one whants to push `Foo` class as a parameter to a query. Suppose `Foo` has an identity field `id` of
      * type `int` and this object will be representetd as this `id` in the DB. Thus one have to define `push<Foo>`
      * specialization as follows:
      * @verbatim
      * namespace nkdhny{
      * namespace db{
      * template <>
      * ParamBuilder& ParamBuilder::push<Foo>(Foo _value) {
      *     IntegerParameter *parameter = new IntegerParameter(_value.id);
      *     params.push_back(parameter);
      *     return *this;
      * }
      *
      * (ii) define ones own `Parameter` subclass representing `Foo` in the DB
      * see e.g. `nkdhny::db::Position3DParameter` and specialization
      * `push<Position3D>`
      * @endverbatim
      *
      */
    template <typename _T>
    ParamBuilder& push(_T _value);

    /** @brief prepare `const char * const *paramValues`
      * for `PQexecParams` based on my params */
    std::vector<Values>   values();

    /** @brief prepare `const int * paramLengths`
      * for `PQexecParams` based on my params */
    std::vector<Sizes>    sizes();

    /** @brief prepare `const int * paramFormats`
      * for `PQexecParams` based on my params */
    std::vector<Formats>  formats();

    /** @brief clears the parameter list by calling destructors
      * of all cotaining parameters and clearing
      * the containr holding its pointers */
    ParamBuilder& clear();
    /** @brief count of the parameter currently pushed in */
    int count();
};


} //db
} //nkdhny

#endif // PARAMBUILDER_H
