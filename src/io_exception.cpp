#include "../include/io_exception.h"
#include <cerrno>
#include <cstring>

IOException::IOException(string context__):
    context_(context__),
    c_errno_(errno)
{
    this->message = this->context_;
    this->message += ": ";
    this->message += strerror(this->c_errno_);
}

int IOException::c_errno() const
{
    return this->c_errno_;
}

string const& IOException::context() const
{
    return this->context_;
}

char const *IOException::what() const noexcept
{
    return this->message.c_str();
}

