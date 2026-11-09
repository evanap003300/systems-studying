#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <limits>
#include <utility>
#include <cassert>

template <typename Key, typename Value>
class OpenAddressingHashMap
{
public:
    OpenAddressingHashMap()
    {
        size_ = 0;
        capacity_ = 16;
        data_.resize(capacity_);
        is_empty_.assign(capacity_, true);
    }

    void insert(const Key &key, const Value &value)
    {
        if (contains(key))
        {
            data_[get_index(key)].second = value;
        }
        else
        {
            if (size_ + 1 >= capacity_ / 2)
            {
                grow();
            }

            size_t index = get_hash(key);

            while (!is_empty_[index])
            {
                index = (index + 1) % capacity_;
            }

            data_[index] = {key, value};
            is_empty_[index] = false;
            size_++;
        }
    }

    bool erase(const Key &key)
    {
        if (!contains(key))
        {
            return false;
        }

        std::size_t index = get_index(key);
        std::size_t hole = index;
        is_empty_[index++] = true;

        while (!is_empty_[index % capacity_])
        {
            auto &[k, v] = data_[index % capacity_];

            std::size_t current = index % capacity_;
            std::size_t home = get_hash(k);

            bool home_between = (hole < current) ? (hole < home && home <= current) : (hole < home || home <= current);

            if (!home_between)
            {
                data_[hole] = data_[current];
                is_empty_[hole] = false;
                is_empty_[current] = true;
                hole = current;
            }

            index++;
        }

        size_--;
        return true;
    }

    Value &get(const Key &key)
    {
        if (!contains(key))
        {
            throw std::out_of_range("Key doesn't exist!");
        }

        return data_[get_index(key)].second;
    }

    const Value &get(const Key &key) const
    {
        if (!contains(key))
        {
            throw std::out_of_range("Key doesn't exist!");
        }

        return data_[get_index(key)].second;
    }

    bool contains(const Key &key) const
    {
        std::size_t index = get_hash(key);

        while (!is_empty_[index % capacity_])
        {
            auto &[k, v] = data_[index % capacity_];

            if (k == key)
            {
                return true;
            }

            index++;
        }

        return false;
    }

    std::size_t get_size() const
    {
        return size_;
    }

    std::size_t get_capacity() const
    {
        return capacity_;
    }

private:
    // Data
    std::vector<std::pair<Key, Value>> data_;
    std::vector<bool> is_empty_;
    std::size_t size_;
    std::size_t capacity_;

    std::size_t get_hash(Key key) const
    {
        std::hash<Key> hasher;
        return hasher(key) % capacity_;
    }

    // Map must contain the key before this is called
    std::size_t get_index(const Key &key)
    {
        std::size_t index = get_hash(key);

        while (!is_empty_[index % capacity_])
        {
            auto &[k, v] = data_[index % capacity_];

            if (k == key)
            {
                return (index % capacity_);
            }

            index++;
        }

        return 0;
    }

    void grow()
    {
        if (capacity_ > std::numeric_limits<std::size_t>::max() / 2)
        {
            throw std::logic_error("Capacity is too large for size_t!");
        }

        capacity_ *= 2;

        std::vector<bool> old_empty = is_empty_;
        std::vector<bool> empty_(capacity_, true);

        std::vector<std::pair<Key, Value>> old_data = data_;
        std::vector<std::pair<Key, Value>> buf(capacity_);

        is_empty_ = empty_;
        data_ = buf;
        size_ = 0;

        for (std::size_t elem = 0; elem < old_data.size(); elem++)
        {
            if (!old_empty[elem])
            {
                auto &[k, v] = old_data[elem];
                insert(k, v);
            }
        }
    }
};

int main()
{
    // Example usage
    OpenAddressingHashMap<std::string, double> prices;

    // Insert some ticker prices
    prices.insert("AAPL", 185.50);
    prices.insert("MSFT", 378.20);
    prices.insert("GOOG", 141.75);

    // Retrieve a value
    double apple = prices.get("AAPL"); // 185.50
    assert(apple == 185.50);

    // Check if a key exists
    bool has_meta = prices.contains("META"); // false
    assert(has_meta == false);

    // Overwrite an existing key
    prices.insert("AAPL", 190.00);
    double updated = prices.get("AAPL"); // 190.00

    assert(updated == 190.00);

    // Size and capacity
    std::size_t count = prices.get_size();   // 3
    std::size_t cap = prices.get_capacity(); // 16

    assert(count == 3);
    assert(cap == 16);

    // Remove a key
    bool removed = prices.erase("MSFT");   // true
    bool again = prices.erase("MSFT");     // false (already removed)
    std::size_t after = prices.get_size(); // 2

    assert(removed == true);
    assert(again == false);
    assert(after == 2);

    OpenAddressingHashMap<int, int> big;
    for (int i = 0; i < 1000; i++)
        big.insert(i, i * 10);
    assert(big.get_size() == 1000);
    for (int i = 0; i < 1000; i++)
        assert(big.get(i) == i * 10);
}