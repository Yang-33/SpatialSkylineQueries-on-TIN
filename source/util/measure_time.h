#ifndef SOURCE_UTIL_MEASURE_TIME_H_
#define SOURCE_UTIL_MEASURE_TIME_H_

#include <glog/logging.h>
#include <chrono>
#include <iostream>
#include <vector>

template <typename TimeT = std::chrono::milliseconds>
struct measure {
  template <typename F, typename... Args>
  static typename TimeT::rep execution(F&& func, Args&&... args) {
    auto start = std::chrono::steady_clock::now();
    std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
    auto duration = std::chrono::duration_cast<TimeT>(
        std::chrono::steady_clock::now() - start);
    return duration.count();
  }
};

template <typename TimeT = std::chrono::milliseconds>
void time_log(const std::string& args, TimeT count, const std::string& desc) {
  LOG(INFO) << args << " took " << count << " " << desc;
}

#define MEASURE_NANOSECONDS(args...)                                      \
  [&]() {                                                                 \
    auto start = std::chrono::steady_clock::now();                        \
    auto res = args;                                                      \
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( \
        std::chrono::steady_clock::now() - start);                        \
    time_log(#args, duration.count(), "nanoseconds (10^(-9) second)");    \
    return res;                                                           \
  }()

#define MEASURE_MICROSECONDS(args...)                                      \
  [&]() {                                                                  \
    auto start = std::chrono::steady_clock::now();                         \
    auto res = args;                                                       \
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( \
        std::chrono::steady_clock::now() - start);                         \
    time_log(#args, duration.count(), "microseconds (10^(-6) second)");    \
    return res;                                                            \
  }()

#define MEASURE_MILLISECONDS(duration, args...)                         \
  [&]() {                                                               \
    auto start = std::chrono::steady_clock::now();                      \
    auto res = args;                                                    \
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(   \
        std::chrono::steady_clock::now() - start);                      \
    time_log(#args, duration.count(), "milliseconds (10^(-3) second)"); \
    return res;                                                         \
  }()

#define MEASURE_SECONDS(args...)                                      \
  [&]() {                                                             \
    auto start = std::chrono::steady_clock::now();                    \
    auto res = args;                                                  \
    auto duration = std::chrono::duration_cast<std::chrono::seconds>( \
        std::chrono::steady_clock::now() - start);                    \
    time_log(#args, duration.count(), "seconds");                     \
    return res;                                                       \
  }()

#endif  // SOURCE_UTIL_MEASURE_TIME_H_
