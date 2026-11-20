#include <new>
#include <cstddef>
#include <iostream>

namespace getcracked
{

    template <typename T>
    void my_delete(T *);

    template <typename T>
    void my_delete_array(T *, size_t);

    template <typename T, typename... Args>
    T *my_new(Args &&...args)
    {
        void *raw = std::malloc(sizeof(T));

        if (raw == nullptr)
        {
            throw std::bad_alloc();
        }

        T *data_ = static_cast<T *>(raw);

        try
        {
            ::new (data_) T(std::forward<Args>(args)...);
        }
        catch (...)
        {
            std::free(data_);
            throw;
        }

        return data_;
    }

    template <typename T>
    void my_delete(T *ptr)
    {
        if (ptr != nullptr)
        {
            ptr->~T();
            std::free(ptr);
        }
    }

    template <typename T>
    T *my_new_array(size_t n)
    {
        void *raw = std::malloc(n * sizeof(T));

        if (raw == nullptr)
        {
            throw std::bad_alloc();
        }

        T *data_ = static_cast<T *>(raw);
        std::size_t k = 0;

        try
        {
            for (std::size_t i = 0; i < n; i++)
            {
                ::new (data_ + i) T();
                k++;
            }
        }
        catch (...)
        {
            my_delete_array(data_, k);
            throw;
        }

        return data_;
    }

    template <typename T>
    void my_delete_array(T *ptr, size_t n)
    {
        if (ptr == nullptr || n == 0)
            return;

        for (int i = n - 1; i >= 0; i--)
        {
            T *mem = ptr + i;
            mem->~T();
        }

        std::free(ptr);
    }

} // namespace getcracked

int main()
{
    int *x = getcracked::my_new<int>(5);
    std::cout << *x << "\n";
    getcracked::my_delete(x);
    return 0;
}