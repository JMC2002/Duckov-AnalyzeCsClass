#pragma once
#include <chrono>
#include <string_view>
#include <source_location>
#include "Logger.hpp"

namespace bench {

    using namespace std::literals;
    class Timer {
    public:
        using clock = std::chrono::steady_clock;

        explicit Timer(std::string_view name,
            simplelog::Level lvl = simplelog::Level::INFO,
            const std::source_location& loc = std::source_location::current()) noexcept
            : name_(name), level_(lvl), loc_(loc), start_(clock::now()) {
        }

        // 打印耗时（析构自动打印）
        ~Timer() noexcept {
            stop();
        }

        void stop() noexcept {
            if (stopped_) return;
            stopped_ = true;
            using namespace std::chrono;
            auto end = clock::now();
            auto us = duration_cast<microseconds>(end - start_).count();
            auto ms = us / 1000.0;

            simplelog::defaultLogger().log(level_,
                "[{}] 耗时: {:.3f} ms ({} μs)", loc_,
                name_, ms, us);
        }

    private:
        std::string name_;
        simplelog::Level level_;
        std::source_location loc_;
        clock::time_point start_;
        bool stopped_ = false;
    };

#define CONCAT_IMPL(prefix, line) prefix##line
#define CONCAT(prefix, line) CONCAT_IMPL(prefix, line)

    /// 方便直接创建作用域计时器的宏
#define BENCH_SCOPE(name, ...) \
    ::bench::Timer CONCAT(_bench_timer_instance_, __LINE__){ name __VA_OPT__(,) __VA_ARGS__ }

#define BENCH_DEBUG(name) \
    ::bench::Timer CONCAT(_bench_debug_timer_instance_, __LINE__){ name, simplelog::Level::DEBUG }
} // namespace bench
