#include "parambuilder.h"

namespace nkdhny {
namespace db {


static void deleteParameter(Parameter* p) {
    delete p;
}

struct FormatBuilder {
    int operator ()(Parameter* p) {
        return p->format;
    }
};

struct SizeBuilder {
    int operator ()(Parameter* p) {
        return p->size;
    }
};

struct ValueBuilder {
    char* operator()(Parameter* p) {
        return p->value;
    }
};

ParamBuilder::ParamBuilder():
    params()
{
}

ParamBuilder::~ParamBuilder()
{
    clear();
}

std::vector<ParamBuilder::Values> ParamBuilder::values()
{
    std::vector<ParamBuilder::Values> v(params.size());

    ValueBuilder builder;
    std::transform(params.begin(), params.end(), v.begin(), builder);

    return v;
}

std::vector<ParamBuilder::Formats> ParamBuilder::formats()
{
    std::vector<ParamBuilder::Formats> f(params.size());

    FormatBuilder builder;
    std::transform(params.begin(), params.end(), f.begin(), builder);

    return f;
}

std::vector<ParamBuilder::Sizes> ParamBuilder::sizes()
{
    std::vector<ParamBuilder::Sizes> s(params.size());

    SizeBuilder builder;
    std::transform(params.begin(), params.end(), s.begin(), builder);

    return s;
}

ParamBuilder &ParamBuilder::clear()
{
    std::for_each(params.begin(), params.end(), deleteParameter);
    params.clear();
}

int ParamBuilder::count()
{
    return params.size();
}

template <>
ParamBuilder& ParamBuilder::push<std::string>(std::string _value) {
    TextParameter *parameter = new TextParameter(_value);
    params.push_back(parameter);

    return *this;
}
template <>
ParamBuilder& ParamBuilder::push<char*>(char* _value) {
    TextParameter *parameter = new TextParameter(std::string(_value));
    params.push_back(parameter);

    return *this;
}
template <>
ParamBuilder& ParamBuilder::push<int>(int _value) {
    IntegerParameter *parameter = new IntegerParameter(_value);
    params.push_back(parameter);

    return *this;
}
template <>
ParamBuilder& ParamBuilder::push<unsigned>(unsigned _value) {
    IntegerParameter *parameter = new IntegerParameter(static_cast<int>(_value));
    params.push_back(parameter);

    return *this;
}

template <>
ParamBuilder& ParamBuilder::push<long>(long _value) {
    LongParameter *parameter = new LongParameter(_value);
    params.push_back(parameter);

    return *this;
}
template <>
ParamBuilder& ParamBuilder::push<unsigned long>(unsigned long _value) {
    LongParameter *parameter = new LongParameter(static_cast<long>(_value));
    params.push_back(parameter);

    return *this;
}

template <>
ParamBuilder& ParamBuilder::push<long long>(long long _value) {
    LongLongParameter *parameter = new LongLongParameter(_value);
    params.push_back(parameter);

    return *this;
}
template <>
ParamBuilder& ParamBuilder::push<unsigned long long>(unsigned long long _value) {
    LongLongParameter *parameter = new LongLongParameter(static_cast<long long>(_value));
    params.push_back(parameter);

    return *this;
}

}
}

