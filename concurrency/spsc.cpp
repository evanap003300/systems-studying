#include <cstddef>
#include <iostream>
#include <atomic>

/*
push then move head forward
pop then move tail forward
head and tail must be on different cache lines to prevent
both are indicies
*/

template <typename T>
class SPSCQueue
{
public:
    explicit SPSCQueue(std::size_t capacity)
    {
        if (capacity <= 2 || __builtin_popcount(capacity) > 1)
        {
            throw std::logic_error("Queue must be a power of 2 and capacity > 2!");
        }

        capacity_ = capacity;
        buf_.resize(capacity);
    }

    bool push(T element)
    {
        std::size_t current = head_.load(std::memory_order_relaxed);

        if (full(current))
        {
            return false;
        }

        buf_[current & (capacity_ - 1)] = std::move(element);
        head_.store(current + 1, std::memory_order_release);
        return true;
    }

    bool pop(T &out)
    {
        std::size_t current = tail_.load(std::memory_order_relaxed);

        if (empty(current))
        {
            return false;
        }

        out = buf_[current & (capacity_ - 1)];
        tail_.store(current + 1, std::memory_order_release);
        return true;
    }

private:
    std::vector<T> buf_;
    std::size_t capacity_;
    alignas(64) std::atomic<std::size_t> head_{};
    alignas(64) std::atomic<std::size_t> tail_{};

    inline bool full(std::size_t current)
    {
        return current - tail_.load(std::memory_order_acquire) == capacity_;
    }

    inline bool empty(std::size_t current)
    {
        return head_.load(std::memory_order_acquire) - current == 0;
    }
};

int main()
{
    SPSCQueue<int> q(8);
    q.push(1);
    int res;
    q.pop(res);
    std::cout << res << "\n";
}
