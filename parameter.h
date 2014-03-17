#ifndef PARAMETER_H
#define PARAMETER_H

#include <vector>
#include <string>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <sstream>

namespace nkdhny{
namespace db{


/** The parameter class
  * is the base class to represent parameters of a query in the DB
  * This class knows how to represent user defined datatype in a binary form
  * sutable to be used in DB
  * Parameters are not copyable
*/
class Parameter {

private:
    Parameter(const Parameter&);
    Parameter& operator=(const Parameter&);

public:
    /** actual binary representation of the parameter*/
    char* value;
    /** size of the binary data (ignored for null terminated string see `TextParameter`) */
    int   size;
    /** 1 for binary parameter 0 - for text (null terminated string); see libpq documentation (`ExecParams` method)*/
    int   format;

    /** @brief Construct parameter of a given format with an
      * empty (NULL) value and zero size */
    Parameter(int _format);
    /** @brief explicitly constructs a parameter by copying given
      * binary value */
    Parameter(const char* _value, int _size, int _format);
    ~Parameter();

    static const int BINARY_FORMAT;
    static const int TEXT_FORMAT;
};
/** Base class to be used to represent binary (e.g. non text parameters) */
class BinaryParameter: public Parameter {
protected:
    BinaryParameter(const char* _value, int _size);
    BinaryParameter();
};
/** Class (where is the `final` keyword?! :)) to represent null terminated strings */
class TextParameter: public Parameter {
public:
    /** Constructs a `TextParameter` holding a copy of string contents */
    TextParameter(const std::string& _text);
};

class IntegerParameter: public BinaryParameter {
public:
    IntegerParameter(int _value);
};

class LongParameter: public BinaryParameter {
public:
    LongParameter(long _value);
};

class LongLongParameter: public LongParameter {
public:
    LongLongParameter(long long _value);
};

} //db
} //nkdhny

#endif // PARAMETER_H
