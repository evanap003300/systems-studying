#include <cstddef>
#include <initializer_list>
#include <cstring>
#include <utility>

namespace getcracked
{

    struct string_long
    {
        char *m_buffer_ptr; // 8 bytes
        size_t m_size;      // 8 bytes
        size_t m_capacity;  // 8 bytes
    };

    struct string_short
    {
        static constexpr size_t capacity{23uz};
        char m_buffer[capacity]; // 23 bytes
        unsigned char m_size;    // 1 byte
    };

    class string
    {
        using iterator = char *;
        using const_iterator = const char *;

    private:
        union
        {
            string_short s;
            string_long l;
        } m_data;

    public:
        // ---------------------------- constructors -------------------------

        // default constructor
        string()
        {
            for (size_t i = 0; i < 23; i++)
                m_data.s.m_buffer[i] = '\0';

            m_data.s.m_size = 1;
        }

        // from literal constructor
        string(const char *chars_array)
        {
            size_t size_ = strlen(chars_array);
            bool is_small = size_ < 23;

            if (is_small)
            {
                for (size_t i = 0; i < size_; i++)
                {
                    m_data.s.m_buffer[i] = chars_array[i];
                }

                m_data.s.m_buffer[size_] = '\0';
                m_data.s.m_size = (size_ << 1) + 1;
            }
            else
            {
                size_t cap = ((size_ + 1) & 1) == 1 ? (size_ + 2) : (size_ + 1);
                char *data_ = new char[cap];
                for (size_t i = 0; i < size_; i++)
                {
                    data_[i] = chars_array[i];
                }
                data_[size_] = '\0';

                m_data.l.m_buffer_ptr = data_;
                m_data.l.m_size = size_;
                m_data.l.m_capacity = cap;
            }
        }

        // copy constructor
        string(const string &other)
        {
            bool other_small = other.m_data.s.m_size & 1;

            if (other_small)
            {
                m_data.s = other.m_data.s;
            }
            else
            {
                if (other.m_data.l.m_capacity == 0 && other.m_data.l.m_buffer_ptr == nullptr)
                {
                    for (size_t i = 0; i < 23; i++)
                        m_data.s.m_buffer[i] = '\0';

                    m_data.s.m_size = 1;
                    return;
                }

                char *data_ = new char[other.m_data.l.m_capacity];
                for (size_t i = 0; i <= other.size(); i++)
                {
                    data_[i] = other.m_data.l.m_buffer_ptr[i];
                }

                m_data.l.m_buffer_ptr = data_;
                m_data.l.m_capacity = other.m_data.l.m_capacity;
                m_data.l.m_size = other.m_data.l.m_size;
            }
        }

        // move constructor
        string(string &&other)
        {
            bool other_small = other.m_data.s.m_size & 1;

            if (other_small)
            {
                m_data.s = other.m_data.s;
            }
            else
            {
                m_data.l.m_capacity = std::exchange(other.m_data.l.m_capacity, 0);
                m_data.l.m_size = std::exchange(other.m_data.l.m_size, 0);
                m_data.l.m_buffer_ptr = std::exchange(other.m_data.l.m_buffer_ptr, nullptr);
            }
        }

        // swap
        void swap(string &other);
        friend void swap(string &lhs, string &rhs);

        // copy assignment
        string &operator=(const string &other);

        // move assignment
        string &operator=(string &&other);

        // construct a string from an arbitrary range [first, last)
        template <typename InputIt>
        string(InputIt first, InputIt last);

        // construct a string from an initializer list
        string(std::initializer_list<char> list);

        // constructs a string with count copies of the character ch
        string(size_t count, char ch);

        // destructor
        ~string()
        {
            bool is_big = (m_data.s.m_size & 1) == 0;

            if (is_big)
                delete[] m_data.l.m_buffer_ptr;
        }

        // ---------------------- capacity functions -------------------------
        // size of the string
        size_t size()
        {
            bool is_small = m_data.s.m_size & 1;
            return is_small ? (m_data.s.m_size >> 1) : m_data.l.m_size;
        }

        // const version
        size_t size() const
        {
            bool is_small = m_data.s.m_size & 1;
            return is_small ? (m_data.s.m_size >> 1) : m_data.l.m_size;
        }

        // capacity
        size_t capacity()
        {
            bool is_small = m_data.s.m_size & 1;
            return is_small ? 22 : m_data.l.m_capacity - 1;
        }

        // checks if string is empty
        bool empty()
        {
            return size() == 0;
        }

        // allocate new storage
        void reserve(size_t new_capacity);

        // ---------------------------- Element Access ------------------------

        // data() return the pointer to the first character of the string
        char *data()
        {
            bool is_small = m_data.s.m_size & 1;
            return is_small ? &m_data.s.m_buffer[0] : m_data.l.m_buffer_ptr;
        }

        // const version
        const char *data() const
        {
            bool is_small = m_data.s.m_size & 1;
            return is_small ? &m_data.s.m_buffer[0] : m_data.l.m_buffer_ptr;
        }

        // Array subscript operator[]
        char &operator[](size_t pos);

        // const version
        const char &operator[](size_t pos) const;

        // indexed access with bounds checking
        char &at(size_t pos);

        // Returns a reference to the first character of the string
        char &front();

        // Returns a reference to the last character of the string
        char &back();

        // ---------------------------------- Iterators -----------------------------

        char *begin();
        const char *begin() const;
        char *end();
        const char *end() const;

        // ------------------------------- Modifiers -----------------------------

        // Appends the given character ch to the end of the string
        void push_back(char ch);

        // removes a character from the end of the string
        void pop_back();

        // inserts the characters from the range [first, last)
        // before the element (if any) pointed to by pos
        template <typename InputIt>
        void insert(const_iterator pos, InputIt first, InputIt last);

        // Removes the characters in the range [first, last)
        iterator erase(const_iterator first, const_iterator last);
    };

} // namespace getcracked

int main()
{
    getcracked::string s;
}