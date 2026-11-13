template <typename T>
class MPMCQueue
{
public:
	struct entry
	{
		alignas(T) std::byte data_[sizeof(T)];
		std::atomic<std::size_t> seq_;

		entry() = default;
		entry(std::size_t seq) : seq_(seq) {}
	};

	explicit MPMCQueue(std::size_t capacity) : capacity_(capacity), buf_(capacity)
	{
		if (capacity <= 2 || __builtin_popcount(capacity) != 1)
		{
			throw std::logic_error("Capacity must be > 2 and power of 2.");
		}

		for (std::size_t i = 0; i < capacity; i++)
		{
			buf_[i].seq_.store(i << 1, std::memory_order_relaxed);
		}
	}

	~MPMCQueue()
	{
		for (std::size_t i = 0; i < capacity_; i++)
		{
			std::size_t seq = buf_[i].seq_.load(std::memory_order_relaxed);

			if (seq & 1)
			{
				T *p = std::launder(reinterpret_cast<T *>(buf_[i].data_));
				std::destroy_at(p);
			}
		}
	}

	bool push(T element)
	{
		while (true)
		{
			std::size_t w_index = write_index_.load();
			std::size_t seq = buf_[w_index & (capacity_ - 1)].seq_.load();

			if (seq == (w_index << 1))
			{
				if (write_index_.compare_exchange_strong(w_index, w_index + 1))
				{
					::new (&buf_[w_index & (capacity_ - 1)].data_) T(std::move(element));
					buf_[w_index & (capacity_ - 1)].seq_ = (w_index << 1) | 1;
					return true;
				}
			}
			else if (seq == ((w_index << 1) | 1) || seq == ((w_index + capacity_) << 1))
			{
				write_index_.compare_exchange_strong(w_index, w_index + 1);
			}
			else if (read_index_.load() + capacity_ <= w_index && seq + (capacity_ << 1) == ((w_index << 1) | 1))
			{
				break;
			}
		}

		return false;
	}

	bool pop(T &out)
	{
		while (true)
		{
			std::size_t r_index = read_index_.load();
			std::size_t seq = buf_[r_index & (capacity_ - 1)].seq_.load();

			if (seq == ((r_index << 1) | 1))
			{
				if (read_index_.compare_exchange_strong(r_index, r_index + 1))
				{
					T *p = std::launder(reinterpret_cast<T *>(buf_[r_index & (capacity_ - 1)].data_));
					out = std::move(*p);
					std::destroy_at(p);
					buf_[r_index & (capacity_ - 1)].seq_ = (r_index + capacity_) << 1;
					return true;
				}
			}
			else if (seq == (r_index + capacity_) << 1)
			{
				read_index_.compare_exchange_strong(r_index, r_index + 1);
			}
			else if (write_index_.load() <= r_index && seq == (r_index << 1))
			{
				break;
			}
		}

		return false;
	}

private:
	std::size_t capacity_;
	std::atomic<std::size_t> write_index_ = 0, read_index_ = 0;
	std::vector<entry> buf_;
};