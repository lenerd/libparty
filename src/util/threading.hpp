#ifndef THREADING_HPP
#define THREADING_HPP

#include <cstddef>
#include <functional>
#include <utility>
#include <boost/asio/thread_pool.hpp>

std::pair<size_t, size_t> get_interval(size_t num_stuff, size_t num_threads, size_t thread_id);

void compute(boost::asio::thread_pool& thread_pool, size_t num_stuff, size_t num_threads, std::function<void(size_t index)> func);

#endif // THREADING_HPP
