#include <mutex>
#include <condition_variable>

namespace getcracked
{
    class ReaderWriterLock
    {
    public:
        unsigned ReaderLock()
        {
            std::unique_lock lock_(mux_);
            reader_.wait(lock_, [&]
                         { return !active_writer_ && writers_pending_ == 0; });
            return reader_count_++;
        }

        unsigned ReaderUnlock()
        {
            std::scoped_lock lock_(mux_);
            unsigned readers = --reader_count_;

            if (readers == 0)
            {
                writer_.notify_one();
            }

            return readers;
        }

        void WriterLock()
        {
            std::unique_lock lock_(mux_);
            writers_pending_++;
            writer_.wait(lock_, [&]
                         { return !active_writer_ && reader_count_ == 0; });
            writers_pending_--;
            active_writer_ = true;
        }

        void WriterUnlock()
        {
            std::scoped_lock lock_(mux_);
            active_writer_ = false;
            if (writers_pending_ > 0)
            {
                writer_.notify_one();
            }
            else
            {
                reader_.notify_all();
            }
        }

    private:
        std::mutex mux_;
        std::condition_variable reader_, writer_;
        unsigned reader_count_ = 0;
        unsigned writers_pending_ = 0;
        bool active_writer_ = false;
    };
}