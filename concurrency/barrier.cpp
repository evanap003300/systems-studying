#include <thread>
#include <vector>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <condition_variable>

/*

building up phase
through phase

*/


class Barrier {
public:
    explicit Barrier(std::size_t count) {
        start_ = count_ = count;
    }

    // Blocks until `count` threads have called arrive_and_wait() in this round.
    // Then all of them are released together, and the barrier resets
    // automatically for the next round.
    void arrive_and_wait() {
        std::unique_lock lock(mux_);

        cv_build_.wait(lock, [this] { return count_ > 0 && !through_; } );
        count_--;
        std::cout << count_ << " I'm waiting for others!\n";
        
        cv_through_.wait(lock, [this] { return count_ == 0 || through_; } );
        std::cout << count_ << " I'm through!\n";
        through_ = true;

        if (++count_ == start_) {
            through_ = false;
            cv_build_.notify_all();
        }

        cv_through_.notify_all();
    }

private:
    std::mutex mux_;
    std::condition_variable cv_build_, cv_through_;

    bool through_ = false;

    std::size_t start_;
    std::size_t count_;
};

int main() {
    Barrier barrier(2);

    std::vector<std::thread> threads;

    for (std::size_t t = 0; t < 4; t++) {
        threads.emplace_back(&Barrier::arrive_and_wait, &barrier);
    }

    for (auto& thread : threads) {
        thread.join();
    }


    return 0;
}