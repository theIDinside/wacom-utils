#pragma once
#include <array>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iterator>
#include <numeric>
#include <source_location>
#include <type_traits>
#include <vector>

consteval bool IsGCC() noexcept {
#if defined(__GNUC__) && !defined(__clang__)
  return true;
#else
  return false;
#endif
}

#if defined(__GNUC__)
#include <format>
#elif defined(__clang__)
#message                                                                       \
    "clang doesn't support std::format yet. Because we can't have nice things. It's impossible."
#endif

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

template <typename DelimiterType>
constexpr std::vector<std::string_view>
split_string(std::string_view str, DelimiterType delim) noexcept {
  std::vector<std::string_view> result{};
  auto last = false;
  for (auto i = str.find(delim); i != std::string_view::npos || !last;
       i = str.find(delim)) {
    last = (i == std::string_view::npos);
    auto sub = str.substr(0, i);
    if (!sub.empty()) {
      result.push_back(sub);
    }
    if (!last) {
      str.remove_prefix(i + 1);
    }
  }
  return result;
}

consteval std::size_t strlen_constexpr(const char *str) {
  std::size_t length = 0;
  while (str[length] != '\0') {
    ++length;
  }
  return length;
}

// Template to count '{' characters in a string at compile time
template <const char *str, std::size_t N, std::size_t idx = 0>
struct InterpolationPartsCounter {
  static constexpr int value =
      (str[idx] == '%' ? 1 : 0) +
      InterpolationPartsCounter<str, N, idx + 1>::value;
};

template <const char *str, std::size_t N>
struct InterpolationPartsCounter<str, N, N> {
  static constexpr int value = 0;
};

template <size_t N> struct CompileTimeString {
  char data[N];
  static constexpr auto Size = N;

  constexpr CompileTimeString(const char (&str)[N]) {
    for (size_t i = 0; i < N; ++i) {
      data[i] = str[i];
    }
  }

  constexpr std::string_view view() const {
    return std::string_view(data, N - 1); // Exclude the null terminator
  }
};

// Helper function to create CompileTimeString
template <size_t N>
constexpr CompileTimeString<N> makeCompileTimeString(const char (&str)[N]) {
  return CompileTimeString<N>(str);
}

template <size_t N>
consteval size_t count_parameters(const CompileTimeString<N> &string) noexcept {
  auto count = 0;
  for (const auto c : string.data) {
    if (c == '%')
      count++;
  }
  return count;
}

template <size_t N, typename... Ts>
consteval std::string format(const CompileTimeString<N> &str,
                             Ts... ts) noexcept {
#if defined(__GNUC__) && !defined(__clang__)
  return std::format(str.data, ts...);
#elif defined(__clang__)
  static_assert(count_parameters(str) == sizeof...(ts),
                "Wrong number of parameters passed to snprintf");
  std::string buf{};
  const auto len = snprintf(nullptr, 4096 * 8, str.data, ts...);
  buf.resize(len + 1, 0);
  snprintf(buf.data(), len, str.data, ts...);
  return buf;
#endif
}

template <size_t N, typename... Ts>
consteval std::string format(const char (&str)[N], Ts... ts) noexcept {
#if defined(__GNUC__) && !defined(__clang__)
  return std::format(str.data, ts...);
#elif defined(__clang__)
  constexpr std::array<char, N> arr = std::to_array<const char, N>(str);
  constexpr auto fmt = CompileTimeString<N>{str};
  return format(fmt, ts...);
#endif
}

} // namespace wu