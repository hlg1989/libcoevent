// file encoding: UTF-8

#include <string>
#include <stddef.h>
#include <stdexcept>
#include <sys/time.h>
#include <vector>

#ifndef __CPP_TOOLS_H__
#define __CPP_TOOLS_H__

namespace andrewmc {
namespace cpptools {

// ==========
// necessary definitions
#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef BOOL
#define BOOL    int
#endif

class Data;

// ==========
// string tools
std::string dump_data_to_string(const void *data, size_t size);
std::string dump_data_to_string(const Data &data);
std::vector<std::string> split_string(const std::string &string, const std::string &seperator);

// ==========
// system-up time
time_t sys_up_time(void);
struct timeval sys_up_timeval(void);

// ==========
// data classes
class Data {
protected:
    void        *_data_buff;
    size_t      _data_len;
    size_t      _buff_size;
    size_t      _ensured_buff_size;
    unsigned    _coefficient;
public:
    // initialization
    Data();
    Data(const void *c_data, size_t length);
    Data(const Data &data_to_copy);
    virtual ~Data();

    // access
    size_t size() const;
    size_t length() const;
    const void *c_data() const;
    const void *bytes() const;

    // modifying
    void clear();
    void turncate(size_t length);
    void expand(size_t length);
    void set_length(size_t length);
    void copy(const Data &data);
    void copy(const void *c_data, size_t length);
    void replace(size_t index, const Data &data);
    void replace(size_t index, const void *c_data, size_t length);
    void append(const Data &data);
    void append(const void *c_data, size_t length);
    void append_nul(void);

    // raw access
    void ensure_buff_capacity(size_t capacity);
    size_t buff_capacity(void);
    void *mutable_raw_data(void);
    void set_raw_data_length(size_t length);

protected:
    size_t _appropriate_buff_size(size_t length = 0);
    void _check_coefficient();
    BOOL _resize(size_t length);
};


}   // end of namespace cpptools
}   // end of namespace andrewmc

#endif  // EOF, end of __CPP_TOOLS_H__
