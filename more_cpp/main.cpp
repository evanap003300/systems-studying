#include <future>
#include <iostream>
#include <thread>
#include <atomic>
#include <cstddef>
#include <vector>

/*
template <class F, class... Args>
auto run_async(F&& f, Args&&... args) -> std::future {
    auto R = std::invoke_result_t<F, Args...>;

    auto funct = []
    std::packaged_task<R()> task(funct);
    return task.get_future(); 
}
*/

namespace getcracked 
{
    template <typename T>
    class SPSCQ
    {
    public:
        SPSCQ(size_t capacity) {
            if (capacity < 2 || __builtin_popcount(capacity) != 1) {
                throw std::logic_error("capacity must be a power of 2 and >= 2.");
            }

            capacity_ = capacity;
            buf_.resize(capacity);
        }
        
        bool push(const T& item)
        {
            if (full()) {
                return false;
            }

            buf_[write_index_ & (capacity_ - 1)] = item;
            write_index_.fetch_add(1, std::memory_order_relaxed);
            size_.fetch_add(1, std::memory_order_acq_rel);
            return true;
        }

        bool pop(T& item)
        {
            if (empty()) {
                return false;
            }

            item = buf_[read_index_ & (capacity_ - 1)];
            read_index_.fetch_add(1, std::memory_order_relaxed);
            size_.fetch_add(-1, std::memory_order_acq_rel);
            return true;
        }

        [[nodiscard]] bool full() const
        {
            return size() == capacity_;
        }

        [[nodiscard]] size_t size() const
        {
            return size_.load(std::memory_order_acquire);
        }

        [[nodiscard]] bool empty() const
        {
            return size() == 0;
        }

    private:
        std::vector<T> buf_;
        std::size_t capacity_;

        std::atomic<std::size_t> size_ = 0; 
        std::atomic<std::size_t> write_index_;
        std::atomic<std::size_t>read_index_;
    };
}

int main() {
    getcracked::SPSCQ<int> q(4);
    q.push(5);
    int res;
    q.pop(res);
    q.size();
    q.empty();
    q.full();
    return 0;
}