#include <atomic>

class SpinLock
{
public:
    void lock() noexcept
    {
        for (;;)
        {
            if (!flag_.exchange(true, std::memory_order_acquire))
                return;

            while (flag_.load(std::memory_order_relaxed))
            {
                // spin
            }
        }
    }

    void unlock() noexcept
    {
        flag_.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> flag_;
};