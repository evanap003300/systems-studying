# systems-studying

Small C++ exercises I write while studying how the standard library and
concurrency primitives work underneath. One file per exercise.

This is a working notebook. Most files have a `main` with asserts and run as
they are, a few are just the class, and a couple are still in progress.

**concurrency/**
- `spsc.cpp`: lock-free single-producer/single-consumer ring buffer, with head
  and tail on separate cache lines
- `mpmc.cpp`: bounded multi-producer/multi-consumer queue, a sequence number
  per slot and CAS on the indices
- `bounded_blocking_queue.cpp`: mutex and two condition variables, with a
  `close()` that lets consumers drain what is left
- `reader_writer_lock.cpp`: reader/writer lock that gives waiting writers
  priority
- `barrier.cpp`: reusable barrier that releases a full group of threads together

**stl/**
- `vector.cpp`: vector with iterators, insert and erase
- `string.cpp`: string with a 23-byte small-string buffer
- `deque.cpp`: deque built on a map of fixed-size blocks
- `open_addressing_hashmap.cpp`: linear-probing hash map

**more_cpp/**
- `threaded_object_pool.cpp`: object pool on an intrusive free list
- `skip_list.cpp`: skip list
- `implement_new.cpp`: `new` and `delete` rebuilt from `malloc` and placement
  new
- `spinlock.cpp`: test-and-test-and-set spinlock
- `stop_token.cpp`: a minimal `stop_source` / `stop_token`
- `memory_defragmentation.cpp`: compacting live objects inside a byte region
  (in progress)

To run one:

```sh
g++ -std=c++20 -pthread concurrency/spsc.cpp -o spsc && ./spsc
```
