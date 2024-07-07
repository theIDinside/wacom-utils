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

#if defined(__GNUC__) && !defined(__clang__)
#include <format>
#elif defined(__clang__)
#warning                                                                       \
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

template <typename T> struct TypeFmtMapper {};

template <> struct TypeFmtMapper<int> {
  static constexpr const char Fmt[] = "d";
  static constexpr size_t N = sizeof(Fmt);
};

template <size_t N> struct FormatString {
  std::array<char, N> data{};
  static constexpr auto Size = N;

  template <size_t M, typename... Ts>
  constexpr FormatString(const char (&str)[M], Ts...) {
    for (size_t i = 0; i < M; ++i) {
      if (str[i] == '{') {
        data[i] = '%';
        data[i + 1] = 'd';
        i++;
      } else {
        data[i] = str[i];
      }
    }
  }

  constexpr FormatString(const char (&str)[N]) {
    for (size_t i = 0; i < N; ++i) {
      if (str[i] == '{') {
        data[i] = '%';
        data[i + 1] = 'd';
        i++;
      } else {
        data[i] = str[i];
      }
    }
  }

  constexpr FormatString(const std::array<char, N> &str) {
    for (size_t i = 0; i < N; ++i) {
      if (str[i] == '{') {
        data[i] = '%';
        data[i + 1] = 'd';
        i++;
      } else {
        data[i] = str[i];
      }
    }
  }

  constexpr std::string_view view() const {
    return std::string_view(data.data(), N - 1); // Exclude the null terminator
  }
};

template <size_t N> constexpr auto makeFormatString(const char (&str)[N]) {
  return FormatString<N>(str);
}

template <size_t N, typename... Ts>
constexpr auto makeFormatString(const char (&str)[N], Ts...) {
  return FormatString<N>(str);
}

template <size_t N>
constexpr auto makeFormatString(const std::array<char, N> &str) {
  return FormatString<N>(str);
}

template <size_t N>
consteval size_t count_parameters(const FormatString<N> &string) noexcept {
  auto count = 0;
  for (const auto c : string.data) {
    if (c == '%')
      count++;
  }
  return count;
}

template <typename... Ts> consteval auto count_ts(const Ts &...) noexcept {
  return sizeof...(Ts);
}

#if defined(__GNUC__) && !defined(__clang__)
#define wu_format(FmtString, ...)
std::format(FmtString, __VA_ARGS__)
#elif defined(__clang__)
#define wu_format(FmtString, ...)                                              \
  [&]() {                                                                      \
    constexpr auto ts = wu::count_ts(__VA_ARGS__);                             \
    constexpr auto compileTimeString = wu::makeFormatString(FmtString);        \
    static_assert(wu::count_parameters(compileTimeString) == ts,               \
                  "Wrong number of parameters passed to snprintf");            \
    std::string buf{};                                                         \
    std::cout << "FmtString='" << FmtString << "' turned into '"               \
              << compileTimeString.data.data() << "'" << std::endl;            \
    const auto len =                                                           \
        snprintf(nullptr, 0, compileTimeString.data.data(), __VA_ARGS__);      \
    buf.resize(len + 1, 0);                                                    \
    snprintf(buf.data(), len + 1, compileTimeString.data.data(), __VA_ARGS__); \
    std::cout << "formatted: '" << buf << "'" << std::endl;                    \
    return buf;                                                                \
  }()
#endif

} // namespace wu