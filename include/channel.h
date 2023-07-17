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
        class Channel;

    private:
        enum StateTag {
            UP_TO_DATE,
            NEW_MESSAGE,
            DISCONNECTED
        };

        mutex lock;
        condition_variable cond_var;
        atomic<StateTag> state_tag;
        queue<T> messages;

        Mpsc();
};

template <typename T>
class Mpsc<T>::Sender {
    friend Mpsc<T>;

    private:
        shared_ptr<Mpsc<T>> channel;
        Sender(shared_ptr<Mpsc<T>> channel_);

    public:
        Sender(Sender const& obj);
        Sender(Sender const&& obj);
        ~Sender();

        bool send(T message) const;
};

template <typename T>
class Mpsc<T>::Receiver {
    friend Mpsc<T>;

    private:
        shared_ptr<Mpsc<T>> channel;
        Receiver(shared_ptr<Mpsc<T>> channel_);

    public:
        Receiver(Receiver const& obj) = delete;
        Receiver& operator=(Receiver const& obj) = delete;
        Receiver(Receiver const&& obj);
        ~Receiver();

        optional<T> receive();
};

template <typename T>
class Mpsc<T>::Channel {
    public:
        Mpsc<T>::Sender sender;
        Mpsc<T>::Receiver receiver;

    private:
        Channel(shared_ptr<Mpsc<T>> inner_channel);

    public:
        static Mpsc<T>::Channel open();
};

template <typename T>
Mpsc<T>::Mpsc():
    state_tag(Mpsc::UP_TO_DATE)
{
}

template <typename T>
Mpsc<T>::Sender::Sender(shared_ptr<Mpsc<T>> channel_):
    channel(channel_)
{
}

template <typename T>
Mpsc<T>::Sender::Sender(Sender const& obj):
    channel(obj.channel)
{
}

template <typename T>
Mpsc<T>::Sender::Sender(Sender const&& obj):
    channel(move(obj.channel))
{
}

template <typename T>
Mpsc<T>::Sender::~Sender()
{
    if (this->channel.use_count() <= 2) {
        this->channel->state_tag = Mpsc::DISCONNECTED;
        this->channel->cond_var.notify_one();
    }
}

template <typename T>
bool Mpsc<T>::Sender::send(T message) const
{
    unique_lock<mutex> lock_guard(this->channel->lock);
    this->channel->messages.push(move(message));
    if (this->channel->state_tag == Mpsc::DISCONNECTED) {
        return false;
    }
    this->channel->state_tag = Mpsc::NEW_MESSAGE;
    this->channel->cond_var.notify_one();
    return true;
}

template <typename T>
Mpsc<T>::Receiver::Receiver(shared_ptr<Mpsc<T>> channel_):
    channel(channel_)
{
}

template <typename T>
Mpsc<T>::Receiver::Receiver(Receiver const&& obj):
    channel(move(obj.channel))
{
}

template <typename T>
Mpsc<T>::Receiver::~Receiver()
{
    this->channel->state_tag = Mpsc::DISCONNECTED;
}

template <typename T>
optional<T> Mpsc<T>::Receiver::receive()
{
    while (true) {
        unique_lock<mutex> lock_guard(this->channel->lock);
        switch (this->channel->state_tag) {
            case Mpsc::NEW_MESSAGE: {
                T message = this->channel->messages.front();
                this->channel->messages.pop();
                if (this->channel->messages.empty()) {
                    this->channel->state_tag = Mpsc::UP_TO_DATE;
                }
                return make_optional(message);
            }
            case Mpsc::UP_TO_DATE:
                this->channel->cond_var.wait(
                    lock_guard,
                    [this] () {
                        return this->channel->state_tag != Mpsc::UP_TO_DATE;
                    }
                );
                break;
            case Mpsc::DISCONNECTED:
                return optional<T>();
        }
    }
}

template <typename T>
Mpsc<T>::Channel::Channel(shared_ptr<Mpsc> inner_channel):
    sender(inner_channel),
    receiver(inner_channel)
{
}

template <typename T>
typename Mpsc<T>::Channel Mpsc<T>::Channel::open()
{
    shared_ptr<Mpsc> channel(new Mpsc());
    return Mpsc<T>::Channel(channel);
}

#endif
