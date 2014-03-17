#include "parameter.h"

namespace nkdhny{
namespace db{


const int Parameter::BINARY_FORMAT = 1;
const int Parameter::TEXT_FORMAT = 0;


Parameter::Parameter(const char *_value, int _size, int _format):
    value(new char[_size]),
    size(_size),
    format(_format)
{
    memcpy(value, _value, size);
}

Parameter::Parameter(int _format):
    value(NULL),
    size(0),
    format(_format)
{
}

Parameter::~Parameter()
{
    if(value){
        delete value;
    }
}

BinaryParameter::BinaryParameter(const char *_value, int _size):
    Parameter(_value, _size, Parameter::BINARY_FORMAT)
{
}

BinaryParameter::BinaryParameter():
    Parameter(Parameter::BINARY_FORMAT)
{
}

TextParameter::TextParameter(const std::string &_text):
    Parameter(_text.c_str(), _text.size()+1, Parameter::TEXT_FORMAT)
{
}

IntegerParameter::IntegerParameter(int _value):
    BinaryParameter()
{
    uint32_t binaryIntVal = htonl(static_cast<uint32_t>(_value));
    BinaryParameter::value = new char[sizeof(uint32_t)];
    memcpy(BinaryParameter::value, &binaryIntVal, sizeof(uint32_t));
    BinaryParameter::size = sizeof(uint32_t);
}


LongParameter::LongParameter(long _value):
    BinaryParameter()
{
#ifdef DEBUG
    assert(sizeof(uint64_t) == sizeof(long));
    assert(sizeof(uint64_t) == 2*sizeof(uint32_t));
#endif
    char* binaryLongVal = new char[sizeof(uint64_t)];
    uint32_t* low_bytes = reinterpret_cast<uint32_t*>(binaryLongVal+sizeof(uint32_t));
    uint32_t* high_bytes = reinterpret_cast<uint32_t*>(binaryLongVal);

    memcpy(binaryLongVal, &_value, sizeof(uint64_t));

    *low_bytes = htonl(*low_bytes);
    *high_bytes = htonl(*high_bytes);
    std::swap(*low_bytes, *high_bytes);

    BinaryParameter::value = binaryLongVal;
    BinaryParameter::size = sizeof(uint64_t);
}

LongLongParameter::LongLongParameter(long long _value):
    LongParameter(static_cast<long>(_value))
{}





}
}

