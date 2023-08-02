#ifndef LOCK_H_
#define LOCK_H_

template <typename M>
class SharedLockGuard {
    private:
        M& rw_lock;
    public:
        SharedLockGuard(M& rw_lock_): rw_lock(rw_lock_)
        {
            this->rw_lock.lock_shared();
        }
        ~SharedLockGuard()
        {
            this->rw_lock.unlock_shared();
        }
};

#endif
