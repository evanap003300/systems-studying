#include <iostream>
#include <memory>
#include <utility>
#include <stdexcept>
#include <new>

template <class T>
class Vector
{
public:
    // ---- Construction and destruction ----
    Vector() = default;

    explicit Vector(std::size_t count) : Vector()
    {
        resize(count);
    }

    Vector(std::size_t count, const T &value) : Vector()
    {
        resize(count, value);
    }

    Vector(const Vector &other)
    {
        T *temp = allocate(other.cap_);
        size_t built = 0;

        try
        {
            for (size_t i = 0; i < other.size_; i++)
            {
                ::new (temp + i) T(std::move_if_noexcept(other.data_[i]));
                built++;
            }
        }
        catch (...)
        {
            std::destroy(temp, temp + built);
            deallocate(temp);
            throw;
        }

        data_ = temp;
        size_ = other.size_;
        cap_ = other.cap_;
    }

    Vector(Vector &&other) noexcept : data_(std::exchange(other.data_, nullptr)),
                                      size_(std::exchange(other.size_, 0)),
                                      cap_(std::exchange(other.cap_, 0)) {}

    ~Vector()
    {
        reset();
        size_ = 0;
    }

    // --- Assignment ---
    Vector &operator=(const Vector &other)
    {
        if (this == &other)
        {
            return *this;
        }

        T *temp = allocate(other.cap_);
        size_t built = 0;

        try
        {
            for (size_t i = 0; i < other.size_; i++)
            {
                ::new (temp + i) T(std::move_if_noexcept(other.data_[i]));
                built++;
            }
        }
        catch (...)
        {
            std::destroy(temp, temp + built);
            deallocate(temp);
            throw;
        }

        reset();

        data_ = temp;
        size_ = other.size_;
        cap_ = other.cap_;

        return *this;
    }

    Vector &operator=(Vector &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        reset();

        data_ = std::exchange(other.data_, nullptr);
        size_ = std::exchange(other.size_, 0);
        cap_ = std::exchange(other.cap_, 0);

        return *this;
    }

    // ---- Element access ----
    T &operator[](std::size_t i) { return data_[i]; }
    const T &operator[](std::size_t i) const { return data_[i]; }

    T &at(std::size_t i)
    {
        if (i >= size_)
        {
            throw std::out_of_range("Index out of bounds!");
        }

        return data_[i];
    }

    const T &at(std::size_t i) const
    {
        if (i >= size_)
        {
            throw std::out_of_range("Index out of bounds!");
        }

        return data_[i];
    }

    T &front() { return data_[0]; }
    const T &front() const { return data_[0]; }
    T &back() { return data_[size_ - 1]; }
    const T &back() const { return data_[size_ - 1]; }

    T *data() noexcept { return data_; }
    const T *data() const noexcept { return data_; }

    // ---- Capacity ----
    bool empty() const noexcept { return size_ == 0; }
    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return cap_; }

    void reserve(std::size_t new_cap)
    {
        if (new_cap <= cap_)
        {
            return;
        }

        T *temp = allocate(new_cap);
        size_t built = 0;

        try
        {
            for (size_t i = 0; i < size_; i++)
            {
                ::new (temp + i) T(std::move_if_noexcept(data_[i]));
                built++;
            }
        }
        catch (...)
        {
            std::destroy(temp, temp + built);
            deallocate(temp);
            throw;
        }

        std::destroy(data_, data_ + size_);
        deallocate(data_);

        data_ = temp;
        cap_ = new_cap;
    }

    void shrink_to_fit()
    {
        if (size_ == 0)
        {
            reset();
            return;
        }

        T *temp = allocate(size_);
        size_t prev_size = size_, built = 0;

        try
        {
            for (size_t i = 0; i < size_; i++)
            {
                ::new (temp + i) T(std::move_if_noexcept(data_[i]));
                built++;
            }
        }
        catch (...)
        {
            std::destroy(temp, temp + built);
            deallocate(temp);
            throw;
        }

        reset();

        data_ = temp;
        size_ = prev_size;
        cap_ = prev_size;
    }

    // ---- Modifiers ----
    void clear() noexcept
    {
        std::destroy(data_, data_ + size_);
        size_ = 0;
    }

    template <class... Args>
    T &emplace_back(Args &&...args)
    {
        if (size_ == cap_)
        {
            T temp(std::forward<Args>(args)...);
            grow();
            new (data_ + size_) T(std::move_if_noexcept(temp));
        }
        else
        {
            new (data_ + size_) T(std::forward<Args>(args)...);
        }

        size_++;
        return data_[size_ - 1];
    }

    void push_back(const T &value)
    {
        emplace_back(value);
    }

    void push_back(T &&value)
    {
        emplace_back(std::move(value));
    }

    void pop_back()
    {
        size_--;
        std::destroy_at(data_ + size_);
    }

    void resize(std::size_t count)
    {
        resize_impl(count, [](T *p)
                    { ::new (p) T(); });
    }

    void resize(std::size_t count, const T &value)
    {
        resize_impl(count, [&value](T *p)
                    { ::new (p) T(value); });
    }

    // ---- Iterators ----
    T *begin() noexcept { return data_; }
    T *end() noexcept { return data_ + size_; }
    const T *begin() const noexcept { return data_; }
    const T *end() const noexcept { return data_ + size_; }

    // --- insert / erase ---
    T *insert(const T *pos, const T &value)
    {
        return insert_impl(pos, value);
    }

    T *insert(const T *pos, T &&value)
    {
        return insert_impl(pos, std::move(value));
    }

    T *erase(const T *pos)
    {
        return erase(pos, pos + 1);
    }

    T *erase(const T *first, const T *last)
    {
        size_t index_first = first - data_;
        size_t elements_removed = last - first;

        if (elements_removed == 0)
            return data_ + index_first;

        for (size_t i = index_first + elements_removed; i < size_; i++)
        {
            data_[i - elements_removed] = std::move(data_[i]);
        }

        std::destroy(data_ + size_ - elements_removed, data_ + size_);
        size_ -= elements_removed;
        return data_ + index_first;
    }

private:
    T *data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t cap_ = 0;

    void grow()
    {
        size_t new_capacity = cap_ == 0 ? 1 : cap_ * 2;
        reserve(new_capacity);
    }

    T *insert_impl(const T *pos, T temp)
    {
        size_t index = pos - data_;
        if (index == size_)
        {
            emplace_back(std::move(temp));
            return data_ + index;
        }

        if (size_ == cap_)
        {
            grow();
        }

        ::new (data_ + size_) T(std::move(data_[size_ - 1]));
        for (size_t i = size_ - 1; i > index; i--)
        {
            data_[i] = std::move(data_[i - 1]);
        }

        data_[index] = std::move(temp);
        size_++;

        return (data_ + index);
    }

    static T *allocate(std::size_t n)
    {
        if (n == 0)
            return nullptr;

        return static_cast<T *>(::operator new(sizeof(T) * n, std::align_val_t(alignof(T))));
    }

    static void deallocate(T *p) noexcept
    {
        if (p != nullptr)
            ::operator delete(p, std::align_val_t(alignof(T)));
    }

    void reset()
    {
        clear();
        deallocate(data_);
        data_ = nullptr;
        cap_ = 0;
    }

    template <class Construct>
    void resize_impl(std::size_t count, Construct construct)
    {
        if (count <= size_)
        {
            std::destroy(data_ + count, data_ + size_);
            size_ = count;
            return;
        }

        if (count <= cap_)
        {
            std::size_t built = size_;

            try
            {
                for (; built < count; built++)
                {
                    construct(data_ + built);
                }
            }
            catch (...)
            {
                std::destroy(data_ + size_, data_ + built);
                throw;
            }
            size_ = count;
            return;
        }

        T *buf = allocate(count);
        std::size_t built_new = size_;
        std::size_t moved = 0;

        try
        {
            for (; built_new < count; built_new++)
            {
                construct(buf + built_new);
            }
            for (; moved < size_; moved++)
            {
                ::new (buf + moved) T(std::move_if_noexcept(data_[moved]));
            }
        }
        catch (...)
        {
            std::destroy(buf, buf + moved);
            std::destroy(buf + size_, buf + built_new);
            deallocate(buf);
            throw;
        }

        std::destroy(data_, data_ + size_);
        deallocate(data_);
        data_ = buf;
        cap_ = count;
        size_ = count;
    }
};

int main()
{
    Vector<int> vec;
}