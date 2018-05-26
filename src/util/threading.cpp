#include <future>
#include <boost/asio/post.hpp>
#include "threading.hpp"

std::pair<size_t, size_t> get_interval(size_t num_stuff, size_t num_threads, size_t thread_id)
{
    auto base_len = num_stuff / num_threads;
    auto rest = num_stuff % num_threads;
    auto start = thread_id * base_len;
    start += thread_id < rest ? thread_id : rest;
    auto end = (thread_id + 1) * base_len;
    end += (thread_id + 1) < rest ? (thread_id + 1) : rest;
    return {start, end};
}

void compute(boost::asio::thread_pool& thread_pool, size_t num_stuff, size_t num_threads, std::function<void(size_t index)> func)
{
    std::vector<std::promise<void>> promises(num_threads);
    for (size_t thread_id = 0; thread_id < num_threads; ++thread_id)
    {
        auto interval = get_interval(num_stuff, num_threads, thread_id);
        boost::asio::post(thread_pool, [interval, func, &promises, thread_id]
            {
                for (size_t i = interval.first; i < interval.second; ++i)
                {
                    func(i);
                }
                promises[thread_id].set_value();
            });
    }
    for (size_t thread_id = 0; thread_id < num_threads; ++thread_id)
    {
        promises[thread_id].get_future().get();
    }
}
