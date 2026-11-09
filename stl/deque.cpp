#include <cstddef>
#include <stdexcept>
#include <new>
#include <iostream>
#include <cassert>

namespace getcracked
{
    template <typename T>
    class deque
    {
    public:
        /*
            map         -> block
            end         -> last element placed
            start_index -> last element placed

            back won't point at the first element
        */

        deque()
        {
            cap_ = 8;

            size_t center = 3;
            start_block_ = center;
            end_block_ = center;

            mp_ = static_cast<T **>(::operator new(sizeof(T *) * cap_));
            mp_[start_block_] = alloc_block();
        }

        ~deque()
        {
            if (start_block_ == end_block_)
            {
                for (size_t index = start_index_; index < end_index_; index++)
                {
                    mp_[start_block_][index].~T();
                }
                dealloc_block(mp_[start_block_]);
            }
            else
            {
                for (size_t index = start_index_; index < B; index++)
                {
                    mp_[start_block_][index].~T();
                }

                dealloc_block(mp_[start_block_]);

                for (size_t block = start_block_ + 1; block < end_block_; block++)
                {
                    for (size_t index = 0; index < B; index++)
                    {
                        mp_[block][index].~T();
                    }
                    dealloc_block(mp_[block]);
                }

                for (size_t index = 0; index < end_index_; index++)
                {
                    mp_[end_block_][index].~T();
                }

                dealloc_block(mp_[end_block_]);
            }

            ::operator delete(mp_);
        }

        void push_back(const T &value)
        {
            if (end_index_ == B && end_block_ + 1 == cap_)
            {
                grow();
            }

            if (end_index_ == B)
            {
                mp_[++end_block_] = alloc_block();
                end_index_ = 0;
            }

            new (&mp_[end_block_][end_index_++]) T(value);
        }

        void pop_back()
        {
            if (end_index_ == 0)
            {
                dealloc_block(mp_[end_block_--]);
                end_index_ = B;
            }

            mp_[end_block_][--end_index_].~T();
        }

        void push_front(const T &value)
        {
            if (start_index_ == 0 && start_block_ == 0)
            {
                grow();
            }

            if (start_index_ == 0)
            {
                mp_[--start_block_] = alloc_block();
                start_index_ = B;
            }

            new (&mp_[start_block_][--start_index_]) T(value);
        }

        void pop_front()
        {
            mp_[start_block_][start_index_].~T();
            if (++start_index_ == B)
            {
                if (start_block_ != end_block_)
                {
                    dealloc_block(mp_[start_block_++]);
                    start_index_ = 0;
                }
                else
                {
                    start_index_ = end_index_ = 0;
                }
            }
        }

        T &at(size_t index)
        {
            if (index >= get_size())
            {
                throw std::logic_error("Out of bounds!");
            }

            size_t pos = index + start_index_;
            size_t block = start_block_ + pos / B;
            size_t block_index = pos % B;

            return mp_[block][block_index];
        }

        size_t get_size() const
        {
            if (start_block_ == end_block_)
            {
                return end_index_ - start_index_;
            }

            size_t between = (end_block_ - start_block_ - 1) * B;
            size_t in_start = B - start_index_;
            size_t in_end = end_index_;

            return between + in_start + in_end;
        }

    private:
        T **mp_ = nullptr;
        size_t B = 8;

        size_t start_block_ = 0, start_index_ = 0;
        size_t end_block_ = 0, end_index_ = 0;

        size_t cap_;

        void grow()
        {
            size_t new_cap_ = cap_ * 2;
            T **buf = static_cast<T **>(::operator new(sizeof(T *) * new_cap_));

            size_t blocks = end_block_ - start_block_ + 1;
            size_t center = new_cap_ / 2 - 1;
            size_t start = center - blocks / 2;
            size_t offset = 0;

            while (offset < blocks)
            {
                buf[start + offset] = mp_[start_block_ + offset];
                offset++;
            }

            ::operator delete(mp_);

            start_block_ = start;
            end_block_ = start + blocks - 1;
            cap_ = new_cap_;

            mp_ = buf;
        }

        T *alloc_block()
        {
            return static_cast<T *>(::operator new(sizeof(T) * B));
        }

        void dealloc_block(T *block)
        {
            ::operator delete(block);
        }
    };
}

int main()
{
    getcracked::deque<int> dq{};

    dq.push_back(1);
    dq.push_back(2);
    dq.push_front(3);

    // Current deque: [3, 1, 2]
    assert(dq.get_size() == 3);
    assert(dq.at(0) == 3);
    assert(dq.at(1) == 1);
    assert(dq.at(2) == 2);

    dq.pop_back(); // deque: [3, 1]
    assert(dq.at(0) == 3);
    assert(dq.at(1) == 1);

    dq.pop_front();
    assert(dq.at(0) == 1); // deque: [1]

    dq.pop_front();
    // dq.at(0); // should throw
}