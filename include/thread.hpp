#ifndef THREAD_HPP_
#define THREAD_HPP_

#include <pthread.h>
#include <tuple>
#include <exception>
#include <memory>
#include <string>
#include <functional>
#include <optional>

class ThreadStartException: public std::exception {
    virtual char const *what() const noexcept;
};

class ThreadJoinException: public std::exception {
    virtual char const *what() const noexcept;
};

class ThreadJoinedException: public std::exception {
    virtual char const *what() const noexcept;
};

template <typename T>
class Thread {
private:
    std::shared_ptr<std::optional<T>> return_ptr;
    pthread_t posix_thread;
    bool joined;

    typedef std::tuple<std::shared_ptr<std::optional<T>>, std::function<T()>>
        StartArgs;
    static void *start_adapter(void *arg);

public:
    template<typename F>
    Thread(F task);

    ~Thread();

    Thread (const Thread&) = delete;
    Thread& operator= (const Thread&) = delete;

    T join();
};

template <typename T>
void *Thread<T>::start_adapter(void *arg)
{
    Thread<T>::StartArgs *start_args = (Thread<T>::StartArgs *) arg;
    T return_value = std::get<1>(*start_args)();
    std::get<0>(*start_args)->emplace(return_value);
    delete start_args;
    return nullptr;
}

template <typename T>
template <typename F>
Thread<T>::Thread(F task):
    joined(false),
    return_ptr(std::shared_ptr<std::optional<T>>(new std::optional<T>()))
{
    Thread<T>::StartArgs *start_args = new Thread::StartArgs(
        this->return_ptr,
        std::function<T()>(task)
    );
    int result = pthread_create(
        &this->posix_thread,
        nullptr,
        Thread::start_adapter,
        (void *) start_args
    );
    if (result < 0) {
        throw ThreadStartException();
    }
}

template <typename T>
Thread<T>::~Thread()
{
    if (!this->joined) {
        pthread_detach(this->posix_thread);
    }
}

template <typename T>
T Thread<T>::join()
{
    if (this->joined) {
        throw ThreadJoinedException();
    }
    int result = pthread_join(this->posix_thread, nullptr);
    if (result < 0) {
        throw ThreadJoinException();
    }
    this->joined = true;
    return std::move(this->return_ptr->value());
}

#endif
