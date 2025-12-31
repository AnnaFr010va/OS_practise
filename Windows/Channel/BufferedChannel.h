#ifndef BUFFERED_CHANNEL_H_
#define BUFFERED_CHANNEL_H_

#include <vector>
#include <utility>
#include <stdexcept>
#include <windows.h>

template<class T>
class BufferedChannel {
public:
    explicit BufferedChannel(std::size_t buffer_size)
        : buffer_(buffer_size), capacity_(buffer_size),
        head_(0), tail_(0), count_(0), closed_(false) {
        InitializeSRWLock(&lock_);
        InitializeConditionVariable(&can_send_);
        InitializeConditionVariable(&can_recv_);
    }

    ~BufferedChannel() = default;

    BufferedChannel(const BufferedChannel&) = delete;
    BufferedChannel& operator=(const BufferedChannel&) = delete;
    BufferedChannel(BufferedChannel&&) = delete;
    BufferedChannel& operator=(BufferedChannel&&) = delete;

    void Send(T value) {
        AcquireSRWLockExclusive(&lock_);

        while (count_ == capacity_) {
            if (closed_) {
                ReleaseSRWLockExclusive(&lock_);
                throw std::runtime_error("send on closed channel");
            }
            SleepConditionVariableSRW(&can_send_, &lock_, INFINITE, 0);
            if (closed_) {
                ReleaseSRWLockExclusive(&lock_);
                throw std::runtime_error("send on closed channel");
            }
        }

        buffer_[tail_] = std::move(value);
        tail_ = (tail_ + 1) % capacity_;
        ++count_;

        WakeConditionVariable(&can_recv_);
        ReleaseSRWLockExclusive(&lock_);
    }

    std::pair<T, bool> Recv() {
        AcquireSRWLockExclusive(&lock_);

        while (count_ == 0 && !closed_) {
            SleepConditionVariableSRW(&can_recv_, &lock_, INFINITE, 0);
        }

        if (count_ == 0 && closed_) {
            ReleaseSRWLockExclusive(&lock_);
            return { T{}, false };
        }

        T val = std::move(buffer_[head_]);
        head_ = (head_ + 1) % capacity_;
        --count_;

        WakeConditionVariable(&can_send_);
        ReleaseSRWLockExclusive(&lock_);
        return { std::move(val), true };
    }

    void Close() {
        AcquireSRWLockExclusive(&lock_);
        if (!closed_) {
            closed_ = true;
            WakeAllConditionVariable(&can_send_);
            WakeAllConditionVariable(&can_recv_);
        }
        ReleaseSRWLockExclusive(&lock_);
    }

private:
    std::vector<T> buffer_;
    const std::size_t capacity_;
    std::size_t head_;
    std::size_t tail_;
    std::size_t count_;
    bool closed_;

    SRWLOCK lock_;
    CONDITION_VARIABLE can_send_;
    CONDITION_VARIABLE can_recv_;
};

#endif