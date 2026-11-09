#include <cstddef>
#include <iostream>
#include <new>
#include <vector>

template <typename T, std::size_t StartingCapacity>
class Pool
{
private:
    union Node
    {
        alignas(T) std::byte data_[sizeof(T)];
        Node *next_;
    };

public:
    Pool() : mem_(alloc(StartingCapacity))
    {
        used_.assign(StartingCapacity, false);

        for (std::size_t i = 0; i < StartingCapacity; i++)
        {
            mem_[i].next_ = i + 1 < StartingCapacity ? &mem_[i + 1] : nullptr;
        }

        if (StartingCapacity > 0)
            head_ = &mem_[0];
    }

    template <typename... Args>
    T *acquire(Args &&...args)
    {
        if (head_ == nullptr)
        {
            return nullptr;
        }

        Node *node = head_;
        Node *next = head_->next_;

        T *obj = ::new (node->data_) T(std::forward<Args>(args)...);
        head_ = next;

        used_[node - mem_] = true;
        in_use_++;

        return obj;
    }

    bool release(T *ptr) noexcept
    {
        if (!ptr)
        {
            return false;
        }

        std::uintptr_t start_addr = reinterpret_cast<std::uintptr_t>(mem_);
        std::uintptr_t end_addr = reinterpret_cast<std::uintptr_t>(mem_ + capacity_);
        std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(ptr);

        // out of range
        if (addr < start_addr || addr >= end_addr)
        {
            return false;
        }

        // alignment doesn't match
        if ((addr - start_addr) % sizeof(Node) > 0)
        {
            return false;
        }

        std::size_t index = (addr - start_addr) / sizeof(Node);
        if (!used_[index])
        {
            return false;
        }

        ptr->~T();
        used_[index] = false;
        Node *node = reinterpret_cast<Node *>(ptr);
        node->next_ = head_;
        head_ = node;
        in_use_--;
        return true;
    }

    ~Pool()
    {
        for (std::size_t i = 0; i < StartingCapacity; i++)
        {
            if (used_[i])
            {
                std::launder(reinterpret_cast<T *>(mem_[i].data_))->~T();
            }
        }

        ::operator delete(mem_, std::align_val_t(alignof(Node)));
    }

    Pool(const Pool &) = delete;
    Pool &operator=(const Pool &) = delete;

private:
    Node *mem_ = nullptr, *head_ = nullptr;
    std::vector<bool> used_;
    std::size_t in_use_ = 0;
    std::size_t capacity_ = StartingCapacity;

    static Node *alloc(std::size_t n)
    {
        return reinterpret_cast<Node *>(::operator new(n * sizeof(Node), std::align_val_t(alignof(Node))));
    }
};

struct A
{
    A()
    {
        a = new int(5);
        b = new int(6);
    }

    ~A()
    {
        delete a;
        delete b;
    }

    int *a;
    int *b;
};

int main()
{
    Pool<A, 5> pool{};

    union U
    {
        // alignas(int)
        std::byte data_[sizeof(int)];
        U *nxt_;
    };

    U u1, u2;
    ::new (u1.data_) int(50);
    u2.nxt_ = &u1;

    std::cout << *reinterpret_cast<int *>(u2.nxt_->data_) << "\n";
}