#ifndef BUFFERED_CHANNEL_H_
#define BUFFERED_CHANNEL_H_

#include <vector>
#include <utility>
#include <stdexcept>
#include <pthread.h>

template<class T>
class BufferedChannel {
public:
    explicit BufferedChannel(std::size_t buffer_size)
        : buffer_(buffer_size), capacity_(buffer_size),
        head_(0), tail_(0), count_(0), closed_(false) {
        pthread_mutex_init(&mutex_, nullptr);
        pthread_cond_init(&can_send_, nullptr);
        pthread_cond_init(&can_recv_, nullptr);
    }

    ~BufferedChannel() {
        pthread_cond_destroy(&can_recv_);
        pthread_cond_destroy(&can_send_);
        pthread_mutex_destroy(&mutex_);
    }

    BufferedChannel(const BufferedChannel&) = delete;
    BufferedChannel& operator=(const BufferedChannel&) = delete;
    BufferedChannel(BufferedChannel&&) = delete;
    BufferedChannel& operator=(BufferedChannel&&) = delete;

    void Send(T value) {
        pthread_mutex_lock(&mutex_);

        while (count_ == capacity_) {
            if (closed_) {
                pthread_mutex_unlock(&mutex_);
                throw std::runtime_error("send on closed channel");
            }
            pthread_cond_wait(&can_send_, &mutex_);
            if (closed_) {
                pthread_mutex_unlock(&mutex_);
                throw std::runtime_error("send on closed channel");
            }
        }

        buffer_[tail_] = std::move(value);
        tail_ = (tail_ + 1) % capacity_;
        ++count_;

        pthread_cond_signal(&can_recv_);
        pthread_mutex_unlock(&mutex_);
    }

    std::pair<T, bool> Recv() {
        pthread_mutex_lock(&mutex_);

        while (count_ == 0 && !closed_) {
            pthread_cond_wait(&can_recv_, &mutex_);
        }

        if (count_ == 0 && closed_) {
            pthread_mutex_unlock(&mutex_);
            return { T{}, false };
        }

        T val = std::move(buffer_[head_]);
        head_ = (head_ + 1) % capacity_;
        --count_;

        pthread_cond_signal(&can_send_);
        pthread_mutex_unlock(&mutex_);
        return { std::move(val), true };
    }

    void Close() {
        pthread_mutex_lock(&mutex_);
        if (!closed_) {
            closed_ = true;
            pthread_cond_broadcast(&can_send_);
            pthread_cond_broadcast(&can_recv_);
        }
        pthread_mutex_unlock(&mutex_);
    }

private:
    std::vector<T> buffer_;
    const std::size_t capacity_;
    std::size_t head_;
    std::size_t tail_;
    std::size_t count_;
    bool closed_;

    pthread_mutex_t mutex_;
    pthread_cond_t  can_send_;
    pthread_cond_t  can_recv_;
};

#endif
