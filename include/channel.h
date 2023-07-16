#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <tuple>
#include <optional>
#include <memory>
#include <forward_list>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <condition_variable>

using namespace std;

template <typename T>
class Mpsc {
    public:
        class Sender;
        class Receiver;

    private:
        enum StateTag {
            UP_TO_DATE,
            NEW_MESSAGE,
            DISCONNECTED
        };

        unique_lock<mutex> lock;
        condition_variable cond_var;
        atomic<StateTag> state_tag;
        queue<T> messages;

        Mpsc();

    public:
        static tuple<Sender, Receiver> open();
};

template <typename T>
class Mpsc<T>::Sender {
    private:
        shared_ptr<Mpsc<T>> channel;
        Sender(shared_ptr<Mpsc<T>> channel_);

    public:
        ~Sender();

        void send(T message) const;
};

template <typename T>
class Mpsc<T>::Receiver {
    private:
        shared_ptr<Mpsc<T>> channel;
        Receiver(shared_ptr<Mpsc<T>> channel_);

    public:
        Receiver(Receiver const& obj) = delete;
        Receiver& operator=(Receiver const& obj) = delete;
        ~Receiver();

        optional<T> receive();
};

template <typename T>
Mpsc<T>::Mpsc():
    state_tag(Mpsc::UP_TO_DATE)
{
}

template <typename T>
tuple<typename Mpsc<T>::Sender, typename Mpsc<T>::Receiver> Mpsc<T>::open()
{
    shared_ptr<Mpsc> channel(new Mpsc());
    return make_pair(Sender(channel), Receiver(channel));
}

template <typename T>
Mpsc<T>::Sender::Sender(shared_ptr<Mpsc<T>> channel_):
    channel(channel_)
{
}

template <typename T>
Mpsc<T>::Sender::~Sender()
{
    this->channel->state_tag = Mpsc::DISCONNECTED;
}

template <typename T>
void Mpsc<T>::Sender::send(T message) const
{
    lock_guard<unique_lock<mutex>> lock_guard(this->channel->lock);
    this->channel->queue.push(move(message));
    this->channel->state_tag = Mpsc::NEW_MESSAGE;
    this->channel->cond_var.notify_one();
}

template <typename T>
Mpsc<T>::Receiver::Receiver(shared_ptr<Mpsc<T>> channel_):
    channel(channel_)
{
}

template <typename T>
Mpsc<T>::Receiver::~Receiver()
{
    if (this->channel.use_count() <= 2) {
        this->channel->state_tag = Mpsc::DISCONNECTED;
        this->channel->cond_var.notify_one();
    }
}

template <typename T>
optional<T> Mpsc<T>::Receiver::receive()
{
    while (true) {
        this->channel->lock->lock();
        switch (this->channel->state_tag) {
            case Mpsc::NEW_MESSAGE: {
                T message = this->channel->queue.front();
                this->channel->queue.pop();
                if (this->channel->queue.empty()) {
                    this->channel->state_tag = Mpsc::UP_TO_DATE;
                }
                this->channel->lock->unlock();
                return make_optional(message);
            }
            case Mpsc::UP_TO_DATE:
                this->channel->cond_var.wait(
                    this->channel->lock,
                    [this] () {
                        return this->channel->state_tag != Mpsc::UP_TO_DATE;
                    }
                );
                break;
            case Mpsc::DISCONNECTED:
                this->channel->lock->unlock();
                return optional<T>();
        }
    }
}

#endif
