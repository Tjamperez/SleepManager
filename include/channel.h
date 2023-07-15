#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <memory>
#include <forward_list>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <condition_variable>

using namespace std;

template <typename T>
class Broadcast {
    using WriteLock = unique_lock<shared_ptr<shared_mutex>>;
    using ReadLock = shared_lock<shared_ptr<shared_mutex>>;

    private:
        class SubscriberInner {
            public:
                atomic<bool> has_message;
                queue<T> msg_queue;
        };

        class ChannelInner {
            public:
                WriteLock subscribers_write_lock;
                ReadLock subscribers_read_lock;
                condition_variable_any publish_cond_var;
                forward_list<weak_ptr<SubscriberInner>> subscribers;
        };
    public:
        class Subscriber;

        class Publisher;

    private:
        shared_ptr<ChannelInner> inner_state;

    public:
        Broadcast();

        Subscriber make_subscriber();
        Publisher make_publisher();
};


template <typename T>
class Broadcast<T>::Subscriber {
    friend Subscriber Broadcast<T>::make_subscriber();

    private:
        shared_ptr<SubscriberInner> inner_state;
        Broadcast<T> channel_;

        Subscriber(Broadcast<T> channel);

    public:
        T subscribe();

        Broadcast<T> channel() const;
};

template <typename T>
class Broadcast<T>::Publisher {
    friend Publisher Broadcast<T>::make_publisher();

    private:
        Broadcast<T> channel_;

        Publisher(Broadcast<T> channel);

    public:
        void publish(T message);

        Broadcast<T> channel() const;
};

template <typename T>
Broadcast<T>::Broadcast():
    inner_state(Broadcast<T>::ChannelInner())
{
}

template <typename T>
typename Broadcast<T>::Subscriber Broadcast<T>::make_subscriber()
{
    return Subscriber(*this);
}

template <typename T>
typename Broadcast<T>::Publisher Broadcast<T>::make_publisher()
{
    return Publisher(*this);
}

template <typename T>
Broadcast<T>::Subscriber::Subscriber(Broadcast<T> channel):
    channel_(channel),
    inner_state(SubscriberInner())
{
    lock_guard<WriteLock> subscribers_lock(
        this->channel_.inner_state.subscribers_write_lock
    );
    this->channel_.inner_state.subscribers(this->inner_state);
}

template <typename T>
T Broadcast<T>::Subscriber::subscribe()
{
    this->channel_.inner_state.subscribers_read_lock.lock();
    if (this->inner_state.msg_queue.empty()) {
        this->inner_state.has_message = false;
        this->channel_.inner_state.condition_variable_any.wait(
            this->channel_.inner_state.subscribers_read_lock,
            [inner_state = this->inner_state] () {
                return inner_state.has_message;
            }
        );
        this->channel_.inner_state.subscribers_read_lock.lock();
    }
    T message = this->inner_state.msg_queue.pop();
    this->channel_.inner_state.subscribers_read_lock.unlock();
    return message;
}

template <typename T>
Broadcast<T> Broadcast<T>::Subscriber::channel() const
{
    return this->channel_;
}

template <typename T>
Broadcast<T>::Publisher::Publisher(Broadcast<T> channel):
    channel_(channel)
{
}

template <typename T>
void Broadcast<T>::Publisher::publish(T message)
{
    lock_guard<WriteLock> subscribers_lock(
        this->channel_.inner_state.subscribers_write_lock
    );

    for (auto it = this->channel_.inner_state.subscribers.before_begin();
             it + 1 != this->channel_.inner_state.subscribers.end();
             it++)
    {
        shared_ptr<SubscriberInner> subs_inner = (it + 1)->lock();
        if (subs_inner.empty()) {
            this->channel_.inner_state.subscribers.remove_after(it);
        } else {
            subs_inner.msg_queue.push(message);
            subs_inner.has_message = true;
        }
    }

    this->channel_.inner_state.publish_cond_var.notify_all();
}

template <typename T>
Broadcast<T> Broadcast<T>::Publisher::channel() const
{
    return this->channel_;
}

#endif
