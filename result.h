#ifndef RESULT_H
#define RESULT_H


#include "row.h"
#include <assert.h>

namespace nkdhny {
namespace db{

/** wrapper on top of the PGResult
  * instance of Result class owns its inner result
  * and ownership is transfered while copying
  * when destructed owned PGResult object is freed
  */
class Result
{
private:
    PGresult* result;
    /** ownership of the inner result
      * owner will release inner result in process of destruction
      * like with auto_ptr ownership is transfered when object is copied **/
    bool owner;
public:
    Result(const Result& other);
    Result(PGresult* _result);
    Result();
    ~Result();

    const Result& operator=(Result& other);

    /** first row of the result set*/
    Row begin();
    /** row "just after" the last one,
      * in fact it is not a real row, and
      * could be used only to check that end of result set is reached
      * like this `row != result.end()` or `row < result.end` */
    Row end();

    int count();

    /** returns true if thes wrapper owns its inner PGResult */
    bool isDefined();

 };

}
}

#endif // RESULT_H
