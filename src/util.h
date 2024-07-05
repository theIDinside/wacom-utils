#pragma once
#include <cstring>
#include <iostream>
#include <iterator>
#include <numeric>
#include <source_location>
#include <type_traits>
#include <vector>

#define FATAL(msg)                                                             \
  const auto curr = std::source_location::current();                           \
  std::cerr << "[FATAL in " << curr.function_name() << "]: " << msg << " "     \
            << curr.file_name() << ":" << curr.line()                          \
            << "\nError: " << strerror(errno) << std::endl;                    \
  std::abort();

namespace wu {
template <typename C, typename Fn>
constexpr auto accumulate(const C &c, Fn &&fn, auto start) noexcept {
  return std::accumulate(std::begin(c), std::end(c), start, std::move(fn));
}

template <typename C, typename Fn>
constexpr auto accumulate(const C &container, Fn &&fn) noexcept {
  return accumulate(container, std::move(fn), 0);
}

template <typename C, typename Fn>
constexpr auto copy(const C &container, Fn &&fn) noexcept {
  std::vector<std::invoke_result_t<Fn, typename C::reference_type>> result{};
  result.reserve(container.size());

  for (const auto &el : container) {
    result.push_back(fn(el));
  }
  return result;
}

} // namespace wu