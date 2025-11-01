#pragma once
#include <iostream>
#include <mutex>
#include <string>
#include <format>
#include <chrono>
#include <ctime>
#include <atomic>
#include <stacktrace>
#include <source_location>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace simplelog {
    enum class Level {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    enum class FunctionFormat {
        None,   // 不显示函数名
        Short,  // 只显示函数名
        Full    // 完整函数签名（默认）
    };

    struct LogConfig {
        bool show_time = true;
        bool show_location = true;
        bool show_level = true;
        bool short_path = true;   // 仅显示文件名，不含路径
        FunctionFormat function_format = FunctionFormat::Short;
    };

    inline constexpr std::string_view toString(Level lvl) noexcept {
        switch (lvl) {
        case Level::TRACE: return "追踪";
        case Level::DEBUG: return "调试";
        case Level::INFO:  return "信息";
        case Level::WARN:  return "警告";
        case Level::ERROR: return "错误";
        case Level::FATAL: return "致命";
        }
        return "未知";
    }

    class Logger {
    public:
        Logger(std::ostream& os = std::clog)
            : out(os), minLevel(Level::TRACE), printStacktraceFrom(Level::ERROR) {
        }

        void setMinLevel(Level lvl) noexcept {
            minLevel.store(lvl, std::memory_order_relaxed);
        }

        void setPrintStacktraceFrom(Level lvl) noexcept {
            printStacktraceFrom.store(lvl, std::memory_order_relaxed);
        }

        void setConfig(const LogConfig& cfg) noexcept {
            std::lock_guard lock(mutex_);
            config_ = cfg;
        }

        // 警惕悬垂
        void setOutput(std::ostream& newOut) noexcept {
            std::lock_guard lock(mutex_);
            out.rdbuf(newOut.rdbuf());
        }

        template <typename... Args>
        void log(Level lvl,
            std::string_view fmt,
            const std::source_location& loc,
            Args&&... args
        ) noexcept
        {
            if (lvl < minLevel.load(std::memory_order_relaxed))
                return;

            try {
                std::ostringstream prefix;
                if (config_.show_time) {
                    auto now = std::chrono::system_clock::now();
                    auto t = std::chrono::system_clock::to_time_t(now);
                    std::tm tm{};
#if defined(_WIN32)
                    localtime_s(&tm, &t);
#else
                    localtime_r(&t, &tm);
#endif
                    prefix << std::put_time(&tm, "[%H:%M:%S] ");
                }

                if (config_.show_level)
                    prefix << std::format("[{}] ", toString(lvl));

                if (config_.show_location) {
                    std::string file = config_.short_path
                        ? std::filesystem::path{ loc.file_name() }.filename().string()
                        : std::string{ loc.file_name() };
                    prefix << file << ':' << loc.line();
                    if (config_.function_format != FunctionFormat::None) {
                        if (config_.function_format == FunctionFormat::Full) {
                            prefix << ' ' << loc.function_name();
                        }
                        else {
                            prefix << ' ' << extractShortFunction(loc.function_name());
                        }
                    }
                    prefix << ": ";
                }

                std::string msg = std::vformat(fmt, std::make_format_args(args...));

                // 减小粒度
                std::string output = prefix.str() + msg + '\n';
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    out << output;
                    if (lvl >= printStacktraceFrom.load()) {
                        out << "---- Stacktrace ----\n" << std::stacktrace::current() << '\n';
                    }
                }
            }
            catch (const std::exception& e) {
                std::cerr << "[LoggerError] Failed to format log message: " << e.what()
                    << " (fmt=" << fmt << ")\n";
            }
        }

    private:
        std::ostream& out;
        std::mutex mutex_;
        LogConfig config_;
        std::atomic<Level> minLevel;
        std::atomic<Level> printStacktraceFrom;

        // 从完整函数签名中提取函数名
        inline std::string extractShortFunction(std::string_view fullName) noexcept {
            // 找到第一个 '('
            size_t paren = fullName.find('(');
            if (paren == std::string_view::npos)
                paren = fullName.size();

            // 从 '(' 向前扫描，跳过空格、*、&、:
            size_t end = paren;
            size_t i = end;
            while (i > 0 && (std::isspace((unsigned char)fullName[i - 1]) ||
                fullName[i - 1] == '*' || fullName[i - 1] == '&' ||
                fullName[i - 1] == ':'))
                --i;

            // 此时 i 指向函数名末尾
            size_t j = i;
            while (j > 0 && (std::isalnum((unsigned char)fullName[j - 1]) || fullName[j - 1] == '_'))
                --j;

            if (i > j)
                return std::string(fullName.substr(j, i - j));

            // fallback：未匹配则原样返回
            return std::string(fullName);
        }
    };

    // 全局默认 logger
    inline Logger& defaultLogger() {
        static Logger logger{};
        return logger;
    }

    // 宏封装（自动捕获 source_location）
#define LOG_TRACE(fmt, ...) ::simplelog::defaultLogger().log(::simplelog::Level::TRACE, fmt, std::source_location::current() __VA_OPT__(,) __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) ::simplelog::defaultLogger().log(::simplelog::Level::DEBUG, fmt, std::source_location::current() __VA_OPT__(,) __VA_ARGS__)
#define LOG_INFO(fmt,  ...) ::simplelog::defaultLogger().log(::simplelog::Level::INFO,  fmt, std::source_location::current() __VA_OPT__(,) __VA_ARGS__)
#define LOG_WARN(fmt,  ...) ::simplelog::defaultLogger().log(::simplelog::Level::WARN,  fmt, std::source_location::current() __VA_OPT__(,) __VA_ARGS__)
#define LOG_ERROR(fmt, ...) ::simplelog::defaultLogger().log(::simplelog::Level::ERROR, fmt, std::source_location::current() __VA_OPT__(,) __VA_ARGS__)
#define LOG_FATAL(fmt, ...) ::simplelog::defaultLogger().log(::simplelog::Level::FATAL, fmt, std::source_location::current() __VA_OPT__(,) __VA_ARGS__)

} // namespace simplelog
