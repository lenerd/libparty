#include <boost/asio/thread_pool.hpp>
#include "ot.hpp"


std::vector<std::pair<bytes_t, bytes_t>> RandomOT::parallel_send(size_t number_ots, size_t number_threads)
{
    boost::asio::thread_pool thread_pool(number_threads);
    auto output = parallel_send(number_ots, number_threads, thread_pool);
    thread_pool.join();
    return output;
}

std::vector<bytes_t> RandomOT::parallel_recv(const std::vector<bool>& choices, size_t number_threads)
{
    boost::asio::thread_pool thread_pool(number_threads);
    auto output = parallel_recv(choices, number_threads, thread_pool);
    thread_pool.join();
    return output;
}
