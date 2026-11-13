#include <memory>
#include <atomic>

namespace getcracked
{

    class stop_source; // Do not remove this forward declaration.

    class stop_token
    {
    public:
        stop_token()
        {
            state_ = std::make_shared<std::atomic<bool>>(false);
        }

        stop_token(std::shared_ptr<std::atomic<bool>> state) : state_{state} {}

        // Has the stop been requested?
        bool stop_requested() const
        {
            return state_->load(std::memory_order_acquire);
        }

    private:
        // Set the cancelled state, returns the previously existing state.
        // You can only call cancel through stop_source.
        bool cancel()
        {
            return !state_->exchange(true, std::memory_order_acq_rel);
        }

        std::shared_ptr<std::atomic<bool>> state_;
        friend class stop_source;
    };

    class stop_source
    {
    public:
        stop_source() = default;

        // Asks for cancellation, returns the previous cancellation state.
        bool request_stop()
        {
            return token.cancel();
        }

        // Returns whether a cancellation has been requested.
        bool stop_requested() const
        {
            return token.stop_requested();
        }

        stop_token get_token()
        {
            return stop_token(token.state_);
        }

    private:
        stop_token token;
    };
}