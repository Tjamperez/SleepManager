#include "../include/thread.hpp"

char const *ThreadStartException::what() const noexcept
{
    return "thread start failed (libc)";
}

char const *ThreadJoinException::what() const noexcept
{
    return "thread join failed (libc)";
}

char const *ThreadJoinedException::what() const noexcept
{
    return "thread already joined failed";
}
