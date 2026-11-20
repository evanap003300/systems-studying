#include <vector>
#include <span>
#include <cstddef>
#include <span>
#include <array>
#include <iostream>
#include <cstdint>

/*

obj[i] -> some bytes in region
if (&region[i] = obj) {
    for (each byte in t)
        move t[i] -> head // head++

}

*/

struct CompactionDetails
{
    int Count{};
};

template <typename T>
CompactionDetails Compact(std::vector<T *> &objects, std::span<std::byte> region)
{
    CompactionDetails counter{};
    std::byte *data_ = region.data();

    if (region.empty())
    {
        throw std::logic_error("region must be non-empty.");
    }

    sort(objects.begin(), objects.end());

    for (auto &obj : objects)
    {
        if (obj == nullptr)
        {
            throw std::logic_error("object can't be a nullptr.");
        }
    }

    for (std::size_t elem = 0; elem < objects.size(); elem++)
    {
        T *obj = objects[elem];
        std::byte *mem = data_ + elem * sizeof(T);

        // if not equal:
        if (static_cast<void *>(obj) != static_cast<void *>(mem))
        {
            // move construct at new address
            T *new_elem = ::new (mem) T(std::move(*obj));

            // destroy the original
            obj->~T();

            // update pointer in vector to new address
            objects[elem] = new_elem;

            // update count
            counter.Count++;
        }
    }

    return counter;
}

int main()
{
    std::vector<int *> vec(5);
    vec[1] = new int(1);
    vec[3] = new int(3);
    vec[4] = new int(4);

    const std::size_t len = 5 * sizeof(int);
    std::array<std::byte, len> buf;

    CompactionDetails res = Compact<int>(vec, buf);
    std::cout << res.Count << "\n";
    return 0;
}