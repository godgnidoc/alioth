#ifndef __json_cpp__
#define __json_cpp__

#include "jsonz.hpp"

#include <math.h>

#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

#include "chainz.hpp"
#ifndef _WIN32
#include <unistd.h>
#else
#endif

namespace alioth {

const unsigned char tag_false = 'f';
const unsigned char tag_true = 't';
const unsigned char tag_integer = 'i';
const unsigned char tag_number = 'd';
const unsigned char tag_string = 's';
const unsigned char tag_array = 'a';
const unsigned char tag_object = 'o';
const unsigned char tag_end = 0;
const unsigned char tag_null = 'n';

void json::clean() {
    switch (mtype) {
        case boolean:
            delete (bool*)mdata;
            break;
        case integer:
            delete (json_integer_t*)mdata;
            break;
        case number:
            delete (json_number_t*)mdata;
            break;
        case string:
            delete (json_string_t*)mdata;
            break;
        case object:
            delete (json_object_t*)mdata;
            break;
        case array:
            delete (json_array_t*)mdata;
            break;
        default:
            break;
    }
    mtype = null;
    mdata = nullptr;
}

json::json(type t) : mtype(t) {
    switch (t) {
        case boolean:
            mdata = new bool(false);
            break;
        case integer:
            mdata = new json_integer_t(0);
            break;
        case number:
            mdata = new json_number_t(0.0);
            break;
        case string:
            mdata = new json_string_t();
            break;
        case object:
            mdata = new json_object_t();
            break;
        case array:
            mdata = new json_array_t();
            break;
        case null:
            mdata = nullptr;
            break;
        default:
            throw std::invalid_argument("invalid type for json::json(type);");
    }
}
json::json(char x) : mtype(integer), mdata(new json_integer_t(x)) {}
json::json(unsigned char x) : mtype(integer), mdata(new json_integer_t(x)) {}
json::json(short x) : mtype(integer), mdata(new json_integer_t(x)) {}
json::json(unsigned short x) : mtype(integer), mdata(new json_integer_t(x)) {}
json::json(int x) : mtype(integer), mdata(new json_integer_t(x)) {}
json::json(unsigned int x) : mtype(integer), mdata(new json_integer_t(x)) {}
json::json(long x) : mtype(integer), mdata(new json_integer_t(x)) {}
json::json(unsigned long x) : mtype(integer), mdata(new json_integer_t(x)) {}
json::json(long long x) : mtype(integer), mdata(new json_integer_t(x)) {}
json::json(unsigned long long x) : mtype(integer), mdata(new json_integer_t(x)) {}
json::json(float x) : mtype(number), mdata(new json_number_t(x)) {}
json::json(double x) : mtype(number), mdata(new json_number_t(x)) {}
json::json(long double x) : mtype(number), mdata(new json_number_t(x)) {}
// json::json( long double x ):mtype(number),mdata(new json_number_t(x)){}
json::json(const char* s) : mtype(string), mdata(new json_string_t(s)) {}
json::json(const json_string_t& s) : mtype(string), mdata(new json_string_t(s)) {}
json::json(json_string_t&& s) : mtype(string), mdata(new json_string_t(std::move(s))) {}
json::json(bool b) : mtype(boolean), mdata(new bool(b)) {}
json::json(json&& j) : mtype(j.mtype), mdata(j.mdata) {
    j.mdata = nullptr;
    j.mtype = null;
}
json::json(const json& j) : mtype(j.mtype) {
    switch (mtype) {
        case boolean:
            mdata = new bool(*(bool*)j.mdata);
            break;
        case integer:
            mdata = new json_integer_t(*(json_integer_t*)j.mdata);
            break;
        case number:
            mdata = new json_number_t(*(json_number_t*)j.mdata);
            break;
        case string:
            mdata = new json_string_t(*(json_string_t*)j.mdata);
            break;
        case object:
            mdata = new json_object_t(*(json_object_t*)j.mdata);
            break;
        case array:
            mdata = new json_array_t(*(json_array_t*)j.mdata);
            break;
        case null:
            mdata = nullptr;
            break;
    }
}
json::json(const json_object_t& map) : mtype(object), mdata(new json_object_t(map)) {}
json::json(json_object_t&& map) : mtype(object), mdata(new json_object_t(std::move(map))) {}
json::json(const json_array_t& a) : mtype(array), mdata(new json_array_t(a)) {}
json::json(json_array_t&& a) : mtype(array), mdata(new json_array_t(std::move(a))) {}
json::~json() { clean(); }

json::type json::is() const { return mtype; }
bool json::is(type t) const { return mtype == t; }

json::operator bool&() {
    if (mtype != boolean) throw std::runtime_error("cannot convert json to boolean");
    return *(bool*)mdata;
}
json::operator const bool&() const {
    if (mtype != boolean) throw std::runtime_error("cannot convert json to boolean");
    return *(const bool*)mdata;
}

json::operator char() const {
    if (mtype == integer)
        return (char)*(json_integer_t*)mdata;
    else if (mtype == number)
        return (char)*(json_number_t*)mdata;
    else
        throw std::runtime_error("cannot convert json to integer");
}

json::operator short() const {
    if (mtype == integer)
        return (short)*(json_integer_t*)mdata;
    else if (mtype == number)
        return (short)*(json_number_t*)mdata;
    else
        throw std::runtime_error("cannot convert json to integer");
}

json::operator int() const {
    if (mtype == integer)
        return (int)*(json_integer_t*)mdata;
    else if (mtype == number)
        return (int)*(json_number_t*)mdata;
    else
        throw std::runtime_error("cannot convert json to integer");
}

json::operator long() const {
    if (mtype == integer)
        return (long)*(json_integer_t*)mdata;
    else if (mtype == number)
        return (long)*(json_number_t*)mdata;
    else
        throw std::runtime_error("cannot convert json to integer");
}

json::operator unsigned char() const {
    if (mtype == integer)
        return (unsigned char)*(json_integer_t*)mdata;
    else if (mtype == number)
        return (unsigned char)*(json_number_t*)mdata;
    else
        throw std::runtime_error("cannot convert json to integer");
}

json::operator unsigned short() const {
    if (mtype == integer)
        return (unsigned short)*(json_integer_t*)mdata;
    else if (mtype == number)
        return (unsigned short)*(json_number_t*)mdata;
    else
        throw std::runtime_error("cannot convert json to integer");
}

json::operator unsigned int() const {
    if (mtype == integer)
        return (unsigned int)*(json_integer_t*)mdata;
    else if (mtype == number)
        return (unsigned int)*(json_number_t*)mdata;
    else
        throw std::runtime_error("cannot convert json to integer");
}

json::operator unsigned long() const {
    if (mtype == integer)
        return (unsigned long)*(json_integer_t*)mdata;
    else if (mtype == number)
        return (unsigned long)*(json_number_t*)mdata;
    else
        throw std::runtime_error("cannot convert json to integer");
}

json::operator unsigned long long() const {
    if (mtype == integer)
        return (unsigned long long)*(json_integer_t*)mdata;
    else if (mtype == number)
        return (unsigned long long)*(json_number_t*)mdata;
    else
        throw std::runtime_error("cannot convert json to integer");
}

json::operator long long&() {
    if (mtype != integer) throw std::runtime_error("cannot convert json to integer");
    return *(json_integer_t*)mdata;
}

json::operator const long long&() const {
    if (mtype != integer) throw std::runtime_error("cannot convert json to integer");
    return *(const json_integer_t*)mdata;
}

json::operator float() const {
    if (mtype == number)
        return (float)*(json_number_t*)mdata;
    else if (mtype == integer)
        return (float)*(json_integer_t*)mdata;
    else
        throw std::runtime_error("cannot convert json to number");
}

json::operator double() const {
    if (mtype == number)
        return (double)*(json_number_t*)mdata;
    else if (mtype == integer)
        return (double)*(json_integer_t*)mdata;
    else
        throw std::runtime_error("cannot convert json to number");
}

json::operator long double&() const {
    if (mtype != number) throw std::runtime_error("connot convert json to number");
    return *(json_number_t*)mdata;
}

json::operator json_string_t&() {
    if (mtype != string) throw std::runtime_error("cannot convert json to string");
    return *(json_string_t*)mdata;
}
json::operator const json_string_t&() const {
    if (mtype != string) throw std::runtime_error("cannot convert json to string");
    return *(const json_string_t*)mdata;
}

char json::asChar() const { return operator char(); }
short json::asShort() const { return operator short(); }
int json::asInt() const { return operator int(); }
long json::asLong() const { return operator long(); }
unsigned char json::asUnsignedChar() const { return operator unsigned char(); }
unsigned short json::asUnsignedShort() const { return operator unsigned short(); }
unsigned int json::asUnsignedInt() const { return operator unsigned int(); }
unsigned long json::asUnsignedLong() const { return operator unsigned long(); }
long long& json::asLongLong() { return operator long long&(); }
const long long& json::asLongLong() const { return operator const long long&(); }
unsigned long long json::asUnsignedLongLong() const { return operator unsigned long long(); }
bool& json::asBool() { return operator bool&(); }
const bool& json::asBool() const { return operator const bool&(); }
float json::asFloat() const { return operator float(); }
double json::asDouble() const { return operator double(); }
long double& json::asLongDouble() const { return operator long double&(); }
std::string& json::asString() { return operator std::string&(); }
const std::string& json::asString() const { return operator const std::string&(); }

json_array_t& json::asArray() {
    if (mtype != array) throw std::runtime_error("cannot convert json to array");
    return *(json_array_t*)mdata;
}
const json_array_t& json::asArray() const {
    if (mtype != array) throw std::runtime_error("cannot convert json to array");
    return *(json_array_t*)mdata;
}
json_object_t& json::asObject() {
    if (mtype != object) throw std::runtime_error("cannot convert json to object");
    return *(json_object_t*)mdata;
}
const json_object_t& json::asObject() const {
    if (mtype != object) throw std::runtime_error("cannot convert json to object");
    return *(json_object_t*)mdata;
}

json& json::operator[](const json_string_t& key) {
    if (mtype != object) throw std::runtime_error("cannot convert json to object");
    return (*(json_object_t*)mdata)[key];
}
const json& json::operator[](const json_string_t& key) const {
    if (mtype != object) throw std::runtime_error("cannot convert json to object");
    if (!((json_object_t*)mdata)->count(key)) throw std::out_of_range("attribute not exists");
    return (*(json_object_t*)mdata)[key];
}

json& json::operator[](int idx) {
    if (mtype != array) throw std::runtime_error("cannot convert json to array");
    auto& list = *(json_array_t*)mdata;

    if (idx >= 0)
        while (idx >= list.size()) list << (null);
    else
        while (-idx > list.size()) list.insert(null, 0);
    return list[idx];
}

const json& json::operator[](int idx) const {
    if (mtype != array) throw std::runtime_error("cannot convert json to array");
    auto& list = *(json_array_t*)mdata;
    auto ret = list.get(idx);
    if (!ret) throw std::out_of_range("element not exists");
    return *ret;
}

int json::count(const json_string_t& key) const {
    if (mtype != object) throw std::runtime_error("cannot convert json to object");
    return ((json_object_t*)mdata)->count(key);
}
int json::count(const json_string_t& key, type t) const {
    if (mtype != object) throw std::runtime_error("cannot convert json to object");
    if (((json_object_t*)mdata)->count(key))
        return (*this)[key].is(t);
    else
        return 0;
}
int json::count() const {
    if (mtype == array)
        return ((json_array_t*)mdata)->size();
    else if (mtype == object)
        return ((json_object_t*)mdata)->size();
    throw std::runtime_error("cannot convert json to array");
}

void json::erase(const json_string_t& key) {
    if (mtype != object) throw std::runtime_error("cannot convert json to object");
    ((json_object_t*)mdata)->erase(key);
}
void json::erase(int idx) {
    if (mtype != array) throw std::runtime_error("cannot convert json to array");
    auto& list = *(json_array_t*)mdata;
    list.remove(idx);
}

void json::push(const json& el) {
    if (mtype != array) throw std::runtime_error("cannot convert json to array");
    auto& list = *(json_array_t*)mdata;
    list.push(el);
}

json json::pop() {
    if (mtype != array) throw std::runtime_error("cannot convert json to array");
    auto& list = *(json_array_t*)mdata;
    auto ret = list.get(0);
    if (!ret) throw std::out_of_range("element not exists");
    list.pop();
    return *ret;
}

json& json::operator=(type t) {
    clean();
    operator=(json(t));
    return *this;
}
json& json::operator=(bool b) {
    if (mtype != boolean) {
        clean();
        operator=(json(b));
    } else
        *(bool*)mdata = b;
    return *this;
}
json& json::operator=(char x) {
    if (mtype != integer) {
        clean();
        operator=(json(x));
    } else
        *(json_integer_t*)mdata = x;
    return *this;
}
json& json::operator=(short x) {
    if (mtype != integer) {
        clean();
        operator=(json(x));
    } else
        *(json_integer_t*)mdata = x;
    return *this;
}
json& json::operator=(int x) {
    if (mtype != integer) {
        clean();
        operator=(json(x));
    } else
        *(json_integer_t*)mdata = x;
    return *this;
}
json& json::operator=(long x) {
    if (mtype != integer) {
        clean();
        operator=(json(x));
    } else
        *(json_integer_t*)mdata = x;
    return *this;
}
json& json::operator=(unsigned char x) {
    if (mtype != integer) {
        clean();
        operator=(json(x));
    } else
        *(json_integer_t*)mdata = x;
    return *this;
}
json& json::operator=(unsigned short x) {
    if (mtype != integer) {
        clean();
        operator=(json(x));
    } else
        *(json_integer_t*)mdata = x;
    return *this;
}
json& json::operator=(unsigned int x) {
    if (mtype != integer) {
        clean();
        operator=(json(x));
    } else
        *(json_integer_t*)mdata = x;
    return *this;
}
json& json::operator=(unsigned long x) {
    if (mtype != integer) {
        clean();
        operator=(json(x));
    } else
        *(json_integer_t*)mdata = x;
    return *this;
}
json& json::operator=(long long x) {
    if (mtype != integer) {
        clean();
        operator=(json(x));
    } else
        *(json_integer_t*)mdata = x;
    return *this;
}
json& json::operator=(unsigned long long x) {
    if (mtype != integer) {
        clean();
        operator=(json(x));
    } else
        *(json_integer_t*)mdata = x;
    return *this;
}
json& json::operator=(float x) {
    if (mtype != number) {
        clean();
        operator=(json(x));
    } else
        *(json_number_t*)mdata = x;
    return *this;
}
json& json::operator=(double x) {
    if (mtype != number) {
        clean();
        operator=(json(x));
    } else
        *(json_number_t*)mdata = x;
    return *this;
}
json& json::operator=(long double x) {
    if (mtype != number) {
        clean();
        operator=(json(x));
    } else
        *(json_number_t*)mdata = x;
    return *this;
}
json& json::operator=(const char* s) {
    if (mtype != string) {
        clean();
        operator=(json(s));
    } else
        *(json_string_t*)mdata = s;
    return *this;
}
json& json::operator=(const json_string_t& s) {
    if (mtype != string) {
        clean();
        operator=(json(s));
    } else
        *(json_string_t*)mdata = s;
    return *this;
}
json& json::operator=(json_string_t&& s) {
    if (mtype != string) {
        clean();
        operator=(json(std::move(s)));
    } else
        *(json_string_t*)mdata = std::move(s);
    return *this;
}
json& json::operator=(const json& j) {
    if (&j == this) return *this;
    auto temp = j;
    clean();
    mtype = temp.mtype;
    mdata = temp.mdata;
    temp.mtype = null;
    temp.mdata = nullptr;
    return *this;
}
json& json::operator=(json&& j) {
    if (&j == this) return *this;
    auto temp = std::move(j);
    clean();
    mtype = temp.mtype;
    mdata = temp.mdata;
    temp.mtype = null;
    temp.mdata = nullptr;
    return *this;
}

bool json::operator==(const json& an) const {
    if (an.mtype != mtype) return false;
    switch (mtype) {
        case null:
            return true;
        case boolean:
            return ((bool)*this) == (bool)an;
        case integer:
            return ((json_integer_t) * this) == (json_integer_t)an;
        case number:
            return ((json_number_t) * this) == (json_number_t)an;
        case string:
            return ((json_string_t) * this) == (json_string_t)an;
        case object: {
            if (count() != an.count()) return false;
            bool same = true;
            for (auto& it : asObject()) {
                auto& key = it.first;
                auto& value = it.second;
                if (!an.count(key)) return same = false;
                if (an[key] != value) return same = false;
            }
            return same;
        }
        case array:
            if (count() == an.count()) return false;
            for (int i = 0; i < count(); i++)
                if ((*this)[i] != an[i]) return false;
            return true;
        default:
            throw std::runtime_error("json::operator == (): impossible type");
    }
}

bool json::operator!=(const json& an) const {
    if (an.mtype != mtype) return true;
    switch (mtype) {
        case null:
            return false;
        case boolean:
            return ((bool)*this) != (bool)an;
        case integer:
            return ((json_integer_t) * this) != (json_integer_t)an;
        case number:
            return ((json_number_t) * this) != (json_number_t)an;
        case string:
            return ((json_string_t) * this) != (json_string_t)an;
        case object: {
            if (count() != an.count()) return true;
            bool same = true;
            for (auto& it : asObject()) {
                auto& key = it.first;
                auto& value = it.second;
                if (!an.count(key)) return same = false;
                if (an[key] != value) return same = false;
            }
            return !same;
        }
        case array:
            if (count() != an.count()) return true;
            for (int i = 0; i < count(); i++)
                if ((*this)[i] != an[i]) return true;
            return false;
        default:
            throw std::runtime_error("json::operator == (): impossible type");
    }
}

json json::decode_json(std::istream& is, std::tuple<int, int, int>& lco) {
    json ret;
    json_string_t tk_s;
    json_integer_t tk_i;
    unsigned gcnt = 0;
    json_number_t tk_n;
    int state = 1;
    bool stay = false;
    bool sign = false;
    int& line = std::get<0>(lco);
    int& column = std::get<1>(lco);
    int& offset = std::get<2>(lco);

    json_string_t err_pre;

    auto get = [&]() {
        auto c = is.get();
        if (c == '\n') {
            line += 1;
            column = 1;
        } else {
            column += 1;
        }
        offset += 1;
        err_pre.push_back(c);
        if (err_pre.size() > 8) err_pre.erase(err_pre.begin());
        return c;
    };

    while (state > 0) {
        const auto it = is.peek();
        switch (state) {
            case 1: {
                if (isspace(it)) {
                    break;
                } else if ('-' == it) {
                    tk_i = 0;
                    gcnt = 0;
                    sign = true;
                    state = 2;
                } else if (isdigit(it)) {
                    tk_i = 0;
                    gcnt = 0;
                    stay = true;
                    sign = false;
                    state = 2;
                } else if ('{' == it) {
                    ret = json::object;
                    sign = false;
                    state = 17;
                } else if ('[' == it) {
                    ret = json::array;
                    sign = false;
                    state = 15;
                } else if ('\"' == it) {
                    tk_s.clear();
                    state = 5;
                } else if ('/' == it) {
                    tk_i = state;
                    state = 9;
                } else if ('n' == it) {
                    state = 12;
                } else if ('f' == it) {
                    state = 13;
                } else if ('t' == it) {
                    state = 14;
                } else if (0 >= it) {
                    state = -1;
                } else {
                    state = -2;
                }
            } break;
            case 2: {
                if (isdigit(it)) {
                    tk_i = tk_i * 10 + (it - '0');
                    gcnt += 1;
                } else if ('.' == it) {
                    if (gcnt) {
                        tk_n = tk_i;
                        tk_i = 0;
                        gcnt = 0;
                        state = 3;
                    } else {
                        state = -7;
                    }
                } else if ('e' == it || 'E' == it) {
                    if (gcnt) {
                        tk_n = (sign ? -tk_i : tk_i);
                        tk_i = 0;
                        gcnt = 0;
                        sign = false;
                        state = 4;
                    } else {
                        state = -7;
                    }
                } else /* if(0 >= it) */ {
                    if (gcnt) {
                        ret = (sign ? -tk_i : tk_i);
                        stay = true;
                        state = 0;
                    } else {
                        state = -1;
                    }
                }
            } break;
            case 3: {
                if ('-' == it) {
                } else if (isdigit(it)) {
                    tk_n = tk_n * 10 + (it - '0');
                    tk_i -= 1;
                    gcnt += 1;
                } else if ('e' == it || 'E' == it) {
                    if (gcnt) {
                        tk_n = tk_n * pow(10, tk_i);
                        if (sign) tk_n = -tk_n;
                        tk_i = 0;
                        gcnt = 0;
                        sign = false;
                        state = 4;
                    } else {
                        state = -8;
                    }
                } else /* if( 0 >= it ) */ {
                    if (gcnt) {
                        tk_n = tk_n * pow(10, tk_i);
                        ret = (sign ? -tk_n : tk_n);
                        stay = true;
                        state = 0;
                    } else {
                        state = -1;
                    }
                }
            } break;
            case 4: {
                if ('-' == it) {
                    if (gcnt) {
                        state = -7;
                    } else {
                        sign = true;
                        gcnt += 1;
                    }
                } else if (isdigit(it)) {
                    tk_i = tk_i * 10 + (it - '0');
                    gcnt += 1;
                } else if (gcnt) {
                    ret = tk_n * pow(10, sign ? -tk_i : tk_i);
                    stay = true;
                    state = 0;
                } else {
                    state = -9;
                }
            } break;
            case 5: {
                if ('\\' == it) {
                    state = 6;
                } else if ('\"' == it) {
                    ret = tk_s;
                    state = 0;
                } else if (0 >= it) {
                    state = -12;
                } else {
                    tk_s += it;
                }
            } break;
            case 6: {
                switch (it) {
                    default:
                        state = -3;
                        break;
                    case '\\':
                        tk_s += '\\';
                        state = 5;
                        break;
                    case '\'':
                        tk_s += '\'';
                        state = 5;
                        break;
                    case '\"':
                        tk_s += '\"';
                        state = 5;
                        break;
                    case 'a':
                        tk_s += '\a';
                        state = 5;
                        break;
                    case 'b':
                        tk_s += '\b';
                        state = 5;
                        break;
                    case 'f':
                        tk_s += '\f';
                        state = 5;
                        break;
                    case 'n':
                        tk_s += '\n';
                        state = 5;
                        break;
                    case 'r':
                        tk_s += '\r';
                        state = 5;
                        break;
                    case 't':
                        tk_s += '\t';
                        state = 5;
                        break;
                    case 'u':
                        tk_i = 0;
                        gcnt = 4;
                        state = 7;
                        break;
                    case 'v':
                        tk_s += '\v';
                        state = 5;
                        break;
                    case 'x':
                        tk_i = 0;
                        gcnt = 2;
                        state = 7;
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7': {
                        tk_i = 0;
                        gcnt = 3;
                        state = 8;
                        stay = true;
                    } break;
                }
            } break;
            case 7: {
                if (gcnt == 0) {
                    tk_s += (char)(tk_i & 0x00FF);
                    if ((tk_i >> 8) != 0) tk_s += (char)((tk_i >> 8) & 0x00FF);
                    state = 5;
                    stay = true;
                } else if ('a' <= it && 'f' >= it) {
                    tk_i = tk_i * 16 + it - 'a' + 10;
                    gcnt -= 1;
                } else if ('A' <= it && 'F' >= it) {
                    tk_i = tk_i * 16 + it - 'A' + 10;
                    gcnt -= 1;
                } else if ('0' <= it && '9' >= it) {
                    tk_i = tk_i * 16 + it - '0';
                    gcnt -= 1;
                } else {
                    state = -3;
                }
            } break;
            case 8: {
                if (gcnt == 0) {
                    tk_s += (char)(tk_i & 0x00FF);
                    state = 5;
                    stay = true;
                } else if ('0' <= it && '7' >= it) {
                    tk_i = tk_i * 8 + it - '0';
                    gcnt -= 1;
                } else {
                    state = -3;
                }
            } break;
            case 9: {
                if ('*' == it) {
                    state = 10;
                } else {
                    state = -2;
                }
            } break;
            case 10: {
                if ('*' == it) {
                    state = 11;
                } else if (0 >= it) {
                    state = -1;
                } else {
                    /*[NOTHING TO BE DONE]*/
                }
            } break;
            case 11: {
                if ('/' == it) {
                    state = tk_i;
                } else if (0 >= it) {
                    state = -1;
                } else {
                    state = 10;
                    stay = true;
                }
            } break;
            case 12: {
                if ('u' == get() && 'l' == get() && 'l' == get()) {
                    stay = true;
                    state = 0;
                    ret = json::null;
                } else {
                    state = -2;
                }
            } break;
            case 13: {
                if ('a' == get() && 'l' == get() && 's' == get() && 'e' == get()) {
                    stay = true;
                    state = 0;
                    ret = false;
                } else {
                    state = -4;
                }
            } break;
            case 14: {
                if ('r' == get() && 'u' == get() && 'e' == get()) {
                    stay = true;
                    state = 0;
                    ret = true;
                } else {
                    state = -4;
                }
            } break;
            case 15: {
                if (']' == it) {
                    if (sign)
                        state = -11;
                    else
                        state = 0;
                } else if ('/' == it) {
                    tk_i = state;
                    state = 9;
                } else if (isspace(it)) {
                    /*[NOTHING TO BE DONE]*/
                } else if (0 >= it) {
                    state = -11;
                } else {
                    ret.push(decode_json(is, lco));
                    stay = true;
                    state = 16;
                }
            } break;
            case 16: {
                if (']' == it) {
                    state = 0;
                } else if (',' == it) {
                    sign = true;
                    state = 15;
                } else if ('/' == it) {
                    tk_i = state;
                    state = 9;
                } else if (isspace(it)) {
                    /*[NOTHONG TO BE DONE]*/
                } else if (0 >= it) {
                    state = -1;
                } else {
                    state = -2;
                }
            } break;
            case 17: {
                if ('}' == it) {
                    if (sign)
                        state = -10;
                    else
                        state = 0;
                } else if ('\"' == it) {
                    tk_s = decode_json(is, lco).asString();
                    stay = true;
                    state = 18;
                } else if ('/' == it) {
                    tk_i = state;
                    state = 9;
                } else if (isspace(it)) {
                    /*[NOTHING TO BE DONE]*/
                } else if (0 >= it) {
                    state = -1;
                } else {
                    state = -2;
                }
            } break;
            case 18: {
                if (':' == it) {
                    state = 19;
                } else if ('/' == it) {
                    tk_i = state;
                    state = 9;
                } else if (isspace(it)) {
                    /*[NOTHING TO BE DONE]*/
                } else if (0 >= it) {
                    state = -1;
                } else {
                    state = -2;
                }
            } break;
            case 19: {
                if (isspace(it)) {
                    /*[NOTHING TO BE DONE]*/
                } else if ('/' == it) {
                    tk_i = state;
                    state = 9;
                } else if (0 >= it) {
                    state = -1;
                } else {
                    ret[tk_s] = decode_json(is, lco);
                    stay = true;
                    state = 20;
                }
            } break;
            case 20: {
                if (isspace(it)) {
                    /*[NOTHING TO BE DONE]*/
                } else if ('}' == it) {
                    state = 0;
                } else if (',' == it) {
                    sign = true;
                    state = 17;
                } else if ('/' == it) {
                    tk_i = state;
                    state = 9;
                } else if (0 >= it) {
                    state = -1;
                } else {
                    state = -2;
                }
            } break;
        }

        if (stay)
            stay = false;
        else if (state >= 0)
            get();
    }

    if (state == 0) return ret;

    json_string_t desc;
    json_string_t err_suf = "\033[1;31m";
    err_suf += is.get();
    err_suf += "\033[0m";
    auto prefix = std::to_string(line) + ":" + std::to_string(column) + ":";
    for (int count = 7; count-- > 0 && is.peek() != EOF; err_suf += is.get())
        ;

    if (state == -1)
        desc = "unexpected end of content:";
    else if (state == -2)
        desc = "unexcepted illegal character:";
    else if (state == -3)
        desc = "invalid escape character:";
    else if (state == -4)
        desc = "illegal character in boolean:";
    else if (state == -5)
        desc = "illegal character in number:";
    else if (state == -6)
        desc = "illegal character in integer:";
    else if (state == -7)
        desc = "illegal character after sign:";
    else if (state == -8)
        desc = "illegal character after dot:";
    else if (state == -9)
        desc = "illegal character after e:";
    else if (state == -10)
        desc = "unexpected end of object:";
    else if (state == -11)
        desc = "unexpected end of array:";
    else if (state == -12)
        desc = "unexpected end of string:";
    else /*if( state < 0 )*/
        desc = "fail to parse json:";

    throw std::runtime_error(prefix + desc + err_pre + err_suf);
}

json json::decode_json(std::istream& is) {
    auto lco = std::tuple<int, int, int>(1, 1, 0);
    return decode_json(is, lco);
}

json json::decode_json(const json_string_t& s) {
    auto lco = std::tuple<int, int, int>(1, 1, 0);
    return decode_json(s, lco);
}

json json::decode_json(const json_string_t& s, std::tuple<int, int, int>& lco) {
    std::stringstream ss(s);
    return decode_json(ss, lco);
}

json json::decode_bson(char* bson, size_t* len) {
    if (!bson) throw std::invalid_argument("nullptr passed as bson");
    switch (*(unsigned char*)bson) {
        case tag_null: {
            if (len) *len = 1;
            return json::null;
        } break;
        case tag_false: {
            if (len) *len = 1;
            return false;
        } break;
        case tag_true: {
            if (len) *len = 1;
            return true;
        } break;
        case tag_integer: {
            if (len) *len = sizeof(json_integer_t) + 1;
            json_integer_t ret = 0;
            ret |= ((unsigned char*)bson)[1];
            ret |= ((unsigned long long)((unsigned char*)bson)[2]) << 8;
            ret |= ((unsigned long long)((unsigned char*)bson)[3]) << 16;
            ret |= ((unsigned long long)((unsigned char*)bson)[4]) << 24;
            ret |= ((unsigned long long)((unsigned char*)bson)[5]) << 32;
            ret |= ((unsigned long long)((unsigned char*)bson)[6]) << 40;
            ret |= ((unsigned long long)((unsigned char*)bson)[7]) << 48;
            ret |= ((unsigned long long)((unsigned char*)bson)[8]) << 56;
            return ret;
        } break;
        case tag_number: {
            if (len) *len = sizeof(double) + 1;
            return *(double*)((unsigned char*)bson + 1);
        } break;
        case tag_string: {
            auto ret = std::string((char*)bson + 1);
            if (len) *len = ret.size() + 2;
            return ret;
        } break;
        case tag_object: {
            json ret = json::object;
            size_t off = 1;
            size_t of;
            while (((unsigned char*)bson)[off] != tag_end) {
                auto key = std::string((char*)bson + off);
                off += key.size() + 1;
                ret[key] = decode_bson((char*)bson + off, &of);
                off += of;
            }
            if (len) *len = off + 1;
            return ret;
        } break;
        case tag_array: {
            json ret = json::array;
            size_t off = 1;
            while (((unsigned char*)bson)[off] != tag_end) {
                size_t of;
                ret.push(decode_bson((char*)bson + off, &of));
                off += of;
            }
            if (len) *len = off + 1;
            return ret;
        } break;
        default:
            throw std::runtime_error("failed to parse bson");
    }
}

char* json::encode_bson(size_t& len) const {
    len = encode_wrt(nullptr);
    auto buf = new char[len];
    encode_wrt(buf);
    return buf;
}

size_t json::encode_wrt(char* buf) const {
    size_t len;
    switch (mtype) {
        case null:
            len = 1;
            break;
        case boolean:
            len = 1;
            break;
        case integer:
            len = sizeof(json_integer_t) + 1;
            break;
        case number:
            len = sizeof(double) + 1;
            break;
        case string:
            len = (*(json_string_t*)mdata).length() + 2;
            break;
        case object: {
            len = 2;
            for (auto& item : *(json_object_t*)mdata) len += 1 + item.first.size() + item.second.encode_wrt(nullptr);
        } break;
        case array: {
            len = 2;
            for (auto& item : *(json_array_t*)mdata) len += item.encode_wrt(nullptr);
        } break;
    }
    if (!buf) return len;

    switch (mtype) {
        case null:
            *(unsigned char*)buf = tag_null;
            break;
        case boolean:
            *(unsigned char*)buf = *(bool*)mdata ? tag_true : tag_false;
            break;
        case integer:
            *(unsigned char*)buf = tag_integer;
            ((unsigned char*)buf)[1] = (*(json_integer_t*)mdata) & 0x0FF;
            ((unsigned char*)buf)[2] = ((*(json_integer_t*)mdata) >> 8) & 0x0FF;
            ((unsigned char*)buf)[3] = ((*(json_integer_t*)mdata) >> 16) & 0x0FF;
            ((unsigned char*)buf)[4] = ((*(json_integer_t*)mdata) >> 24) & 0x0FF;
            ((unsigned char*)buf)[5] = ((*(json_integer_t*)mdata) >> 32) & 0x0FF;
            ((unsigned char*)buf)[6] = ((*(json_integer_t*)mdata) >> 40) & 0x0FF;
            ((unsigned char*)buf)[7] = ((*(json_integer_t*)mdata) >> 48) & 0x0FF;
            ((unsigned char*)buf)[8] = ((*(json_integer_t*)mdata) >> 56) & 0x0FF;
            break;
        case number:
            *(unsigned char*)buf = tag_number;
            *(double*)((unsigned char*)buf + 1) = (double)(*(json_number_t*)mdata);
            break;
        case string:
            *(unsigned char*)buf = tag_string;
#ifdef _WIN32
            strcpy_s((char*)buf + 1, (*(json_string_t*)mdata).length() + 1, (*(json_string_t*)mdata).data());
#else
            strcpy((char*)buf + 1, (*(json_string_t*)mdata).data());
#endif
            break;
        case object: {
            *(unsigned char*)buf = tag_object;
            size_t off = 1;
            for (auto& item : *(json_object_t*)mdata) {
#ifdef _WIN32
                strcpy_s((char*)buf + off, item.first.length() + 1, item.first.data());
#else
                strcpy((char*)buf + off, item.first.data());
#endif
                off += item.first.size() + 1;
                off += item.second.encode_wrt((char*)buf + off);
            }
            ((unsigned char*)buf)[off] = tag_end;
        } break;
        case array: {
            *(unsigned char*)buf = tag_array;
            size_t off = 1;
            for (auto& item : *(json_array_t*)mdata) off += item.encode_wrt((char*)buf + off);
            ((unsigned char*)buf)[off] = tag_end;
        }
    }
    return len;
}

json_string_t json::encode_json() const {
    switch (mtype) {
        case null:
            return "null";
        case boolean:
            return *(bool*)mdata ? "true" : "false";
        case integer:
            return std::to_string(*(json_integer_t*)mdata);
        case number:
            return std::to_string(*(json_number_t*)mdata);
        case object: {
            const auto& map = *(json_object_t*)mdata;
            auto count = map.size();
            if (count == 0) return "{}";
            json_string_t ret = "{";
#if __cplusplus >= 201703
            for (auto [key, value] : map) {
#else
            for (auto i = map.begin(); i != map.end(); i++) {
                auto& key = i->first;
                auto& value = i->second;
#endif
                ret += "\"" + key + "\":" + value.encode_json();
                if (count-- > 1) ret += ',';
            }
            ret += "}";
            return ret;
        }
        case array: {
            const auto& list = *(json_array_t*)mdata;
            auto count = list.size();
            if (count == 0) return "[]";
            json_string_t ret = "[";
            for (auto value : list) {
                ret += value.encode_json();
                if (count-- > 1) ret += ',';
            }
            ret += "]";
            return ret;
        }
        case string: {
            json_string_t ret = "\"";
            for (auto i : *(json_string_t*)mdata) switch (i) {
                    case '\"':
                        ret += "\\\"";
                        break;
                    case '\\':
                        ret += "\\\\";
                        break;
                    // case '/': ret += "\\/";break;
                    case '\a':
                        ret += "\\a";
                        break;
                    case '\b':
                        ret += "\\b";
                        break;
                    case '\f':
                        ret += "\\f";
                        break;
                    case '\n':
                        ret += "\\n";
                        break;
                    case '\r':
                        ret += "\\r";
                        break;
                    case '\t':
                        ret += "\\t";
                        break;
                    case '\v':
                        ret += "\\v";
                        break;
                    default:
                        if( iscntrl(i) ) {
                            char buf[8];
                            sprintf(buf, "\\u00%02x", 0x00FF&i);
                            ret += buf;
                        } else {
                            ret += i;
                        }
                        /*if( isprint(*i) )*/
                        /*else if( *(i+1) != '\0' ) {
                            char buf[8];
                            sprintf(buf,"%02x%02x", 0x00FF&*(i+1), 0x00FF&*i );
                            ret += "\\u" + json_string_t(buf);
                            i += 1;
                        } else {
                            char buf[8];
                            sprintf(buf,"00%02x", 0x00FF&*i );
                            ret += "\\u" + json_string_t(buf);
                        }*/
                }
            ret += "\"";
            return ret;
        }
        default:
            throw std::runtime_error("internal error of json type");
    }
}

}  // namespace alioth
#endif