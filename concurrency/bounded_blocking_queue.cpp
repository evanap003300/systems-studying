#include <vector>
#include <cstddef>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <optional>
#include <thread>

template <typename T>
class BoundedBlockingQueue {
public:
    explicit BoundedBlockingQueue(std::size_t capacity) {
        capacity_ = capacity;
        buf_.resize(capacity_);
    }

    // Blocks while the queue is full.
    // Returns false (and does not insert) if the queue has been closed.
    bool push(T value) {
        std::unique_lock lock(mux_);
        cv_push_.wait(lock, [this] { return size_locked() != capacity_ || shut_down_; } );

        if (shut_down_) {
            return false;
        }

        buf_[head_ % capacity_] = std::move(value);
        head_++;
        cv_pop_.notify_one();

        return true;
    }

    // Blocks while the queue is empty.
    // Returns the next item, or std::nullopt once the queue is closed AND empty.
    std::optional<T> pop() {
        std::unique_lock lock(mux_);
        cv_pop_.wait(lock, [this] { return size_locked() != 0 || shut_down_; } );

        if (size_locked() == 0) {
            return std::nullopt;
        }

        T elem = std::move(buf_[tail_ % capacity_]);
        tail_++;
        cv_push_.notify_one();

        return elem;
    }

    // After close(): every push fails, pops drain the remaining items,
    // and every blocked thread wakes up.
    void close() {
        {
            std::scoped_lock lock(mux_);
            shut_down_ = true;
        }

        cv_push_.notify_all();
        cv_pop_.notify_all();
    }

    std::size_t size() const {
        std::scoped_lock lock(mux_);
        return size_locked();
    }

private:
    std::vector<T> buf_;

    mutable std::mutex mux_;
    std::condition_variable cv_push_, cv_pop_;

    std::size_t head_{0};
    std::size_t tail_{0};

    std::size_t capacity_;

    bool shut_down_{false};

    std::size_t size_locked() const {
        return head_ - tail_;
    }
};

int main() {
    const constexpr std::size_t OPERATIONS = 100;
    
    const std::size_t THREADS = std::thread::hardware_concurrency(); 
    const std::size_t PRODUCER_COUNT = THREADS / 2;
    const std::size_t CONSUMER_COUNT = THREADS / 2;

    BoundedBlockingQueue<std::size_t> q(8);

    auto producer = [&]() {
        for (std::size_t op = 0; op < OPERATIONS; op++) {
            q.push(op);
        }
    };

    auto consumer = [&]() {
        for (std::size_t op = 0; op < OPERATIONS; op++) {
            q.pop();
        }
    };

    std::vector<std::thread> tasks;

    for (std::size_t p = 0; p < PRODUCER_COUNT; p++) {
        tasks.emplace_back(producer);
    }

    for (std::size_t c = 0; c < CONSUMER_COUNT; c++) {
        tasks.emplace_back(consumer);
    }

    for (auto& thread : tasks) {
        thread.join();
    }

    q.close();
}