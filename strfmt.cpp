//
// Author: Dmitry Melnik
//

#include "strfmt.h"

#include <stdarg.h>


std::string strfmt(const char* fmt, ... ) {
    const char* inp = ( fmt == nullptr )? "" : fmt;
    char buf[1024];
    va_list args;
    va_start (args, inp);
    vsnprintf (buf,sizeof(buf),inp, args);
    va_end (args);
    return (std::string)buf;
}

std::string strfmt(const std::string& fmt, ... ) {
    char buf[1024];
    va_list args;
    va_start (args, fmt);
    vsnprintf (buf,sizeof(buf), fmt.c_str(), args);
    va_end (args);
    return (std::string)buf;
}
