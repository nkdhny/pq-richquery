#include "row.h"

namespace nkdhny{
namespace db{


Row::Row(PGresult *_result, int _rowno):
    res(_result),
    rowno(_rowno)
{}

bool Row::operator ==(const Row &other) const
{
    return rowno == other.rowno && res == other.res;
}

bool Row::operator !=(const Row &other) const
{
    return !((*this) == other);
}

bool Row::operator <(const Row &other) const
{
    assert(res == other.res);
    return rowno < other.rowno;
}

const Row &Row::operator ++()
{
    rowno++;
    return *this;
}


template <>
std::string Row::get<std::string> (int colno) {
    assert(rowno < PQntuples(res));
    assert(colno < PQnfields(res));
    return std::string(PQgetvalue(res, rowno, colno));
}


template <>
int Row::get<int> (int colno) {
    assert(rowno < PQntuples(res));
    assert(colno < PQnfields(res));
    return static_cast<int>(ntohl((*reinterpret_cast<uint32_t*>(PQgetvalue(res, rowno, colno)))));
}

template <>
unsigned Row::get<unsigned> (int colno) {
    assert(rowno < PQntuples(res));
    assert(colno < PQnfields(res));
    return static_cast<unsigned>(ntohl((*reinterpret_cast<uint32_t*>(PQgetvalue(res, rowno, colno)))));
}

template <>
long Row::get<long> (int colno) {
    assert(rowno < PQntuples(res));
    assert(colno < PQnfields(res));

    char* longBinary = PQgetvalue(res, rowno, colno);
#ifdef DEBUG
    int n_bytes = PQgetlength(res, rowno, colno);
    assert(n_bytes == sizeof(long));
    assert(sizeof(long) == 2*sizeof(uint32_t));
#endif
    long ret = 0;
    uint32_t rec_low_bytes = *(reinterpret_cast<uint32_t*>(longBinary+sizeof(uint32_t)));
    uint32_t rec_high_bytes = *(reinterpret_cast<uint32_t*>(longBinary));

    uint32_t* ret_low_bytes = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(&ret) + sizeof(uint32_t));
    uint32_t* ret_high_bytes = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(&ret));

    *ret_high_bytes = ntohl(rec_low_bytes);
    *ret_low_bytes = ntohl(rec_high_bytes);

    return ret;
}

template <>
unsigned long Row::get<unsigned long> (int colno) {    

    return static_cast<unsigned long>(this->get<long>(colno));
}

template <>
long long Row::get<long long> (int colno) {
    assert(rowno < PQntuples(res));
    assert(colno < PQnfields(res));

    char* longBinary = PQgetvalue(res, rowno, colno);
#ifdef DEBUG
    int n_bytes = PQgetlength(res, rowno, colno);
    assert(n_bytes == sizeof(long));
    assert(sizeof(long long) == 2*sizeof(uint32_t));
#endif
    long ret = 0;
    uint32_t rec_low_bytes = *(reinterpret_cast<uint32_t*>(longBinary+sizeof(uint32_t)));
    uint32_t rec_high_bytes = *(reinterpret_cast<uint32_t*>(longBinary));

    uint32_t* ret_low_bytes = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(&ret) + sizeof(uint32_t));
    uint32_t* ret_high_bytes = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(&ret));

    *ret_high_bytes = ntohl(rec_low_bytes);
    *ret_low_bytes = ntohl(rec_high_bytes);

    return ret;
}

template <>
unsigned long long Row::get<unsigned long long> (int colno) {

    return static_cast<unsigned long long>(this->get<long long>(colno));
}

}
}
