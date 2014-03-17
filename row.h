#ifndef ROW_H
#define ROW_H

#include <string>
#include <vector>
#include <postgresql/libpq-fe.h>
#include "assert.h"
#include <netinet/in.h>
#include <sstream>
#include <iostream>

namespace nkdhny{
namespace db{

/** wrapper class to iterate through results of query execution */
struct Row {
    int rowno;
    PGresult* res;


    Row(PGresult* _result, int _rowno = 0);

    /** Typed get the content of the column of the result
      * One could define ones own specializations of this method to
      * get user defined datatypes as a query results. For example if `Foo`
      * can be created from int and if int is stored in place where `Foo` is implied
      * one could define `get<Foo>` as follows
      * @verbatim
      * template<>
      * Foo Row::get<Foo>(int colno){
      *  return Foo(this->get<int>(colno));
      *}
      * @endverbatim
      * More general way is to load binary representation of object with `PQgetvalue`
      * and constract an object based on its binary representation
      */
    template <typename _T>
    _T get(int);

    /** gets the content of named column in a row*/
    template <typename _T>
    _T get(const std::string&);

    bool operator ==(const Row& other) const;
    bool operator !=(const Row& other) const;
    bool operator < (const Row& other) const;

    /** iterate to next row in a result set*/
    const Row& operator ++();


};

template <typename T>
T Row::get(const std::string& colname) {
    int colno = PQfnumber(res, colname.c_str());
    return get<T>(colno);
}


}} //db //nkdhny

#endif // ROW_H
