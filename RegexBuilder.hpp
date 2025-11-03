#pragma once
#include <regex>
#include <string>
#include <vector>
#include <optional>
#include <sstream>
#include <functional>
#include <ranges>
#include <algorithm>

#include "Logger.hpp"
using namespace std::literals;

template <typename  T, typename Member = std::string>
    requires std::is_assignable_v<Member&, const Member&>
struct RegexPart {
    std::string_view pattern;      // 实际的正则内容
    std::string_view suffix;       // 后缀，如?、+等

    bool grouped;      // 是否加分组

    using FieldType = std::remove_cvref_t<Member>;
    std::optional<FieldType T::*> member; // 捕获的目标

    template<typename B>    // 防止const char*匹配到bool
        requires std::is_same_v<std::remove_cvref_t<B>, bool>
    RegexPart(std::string_view pattern, B grouped)
        : pattern(pattern), grouped(grouped) {
        LOG_TRACE("调用构造1，pattern = {}, grouped = {}", pattern, grouped);
    }

    // 如果需要后缀或者捕获，一定会分组
    RegexPart(std::string_view pattern, std::string_view suffix = ""sv)
        : RegexPart(pattern, true) {
        LOG_TRACE("调用构造2，pattern = {}, suffix = {}", pattern, suffix.size() ? suffix : "空"sv);
        this->suffix = suffix;
    }

    RegexPart(Member T::* mem, std::string_view pattern, std::string_view suffix = ""sv)
        : RegexPart(pattern, suffix) {
        LOG_TRACE("调用构造3，pattern = {}, suffix = {}", pattern, suffix.size() ? suffix : "空"sv);
        member = mem;
    }

    std::string toRegex()const {
        if (!grouped)
            return std::string(pattern);

        return "("s
             + (capturing() ? "" : "?:")
             + std::string(pattern)
             + ")"
             + std::string(suffix);
    }

    bool capturing()const noexcept {
        return member.has_value();
    }

    void assign(T& obj, const Member& value) const noexcept {
        if (member)
            obj.*(*member) = value;
    }
};

template <typename T>
class RegexBuilder {
    std::vector<RegexPart<T>> parts;
    std::regex re;
    std::string default_delimiter;
public:
    std::string pattern;
    // 设置前缀
    RegexBuilder(std::string_view prefix = ""sv, std::string_view default_delimiter = ""sv)
        : pattern(prefix), default_delimiter(default_delimiter) {}

    template <typename... Val>
        requires(sizeof...(Val) >= 1)
    RegexBuilder& join_with(std::string_view pre, Val&&... val) {
        LOG_TRACE("调用join_with，pre = {}", pre.size() ? pre : "空"sv);
        parts.emplace_back(std::forward<Val>(val)...);
        pattern += std::string(pre) + parts.back().toRegex();
        return *this;
    }

    template <typename... Val>
    RegexBuilder& join(Val&&... val) {
        return join_with(default_delimiter, std::forward<Val>(val)...);
    }

    RegexBuilder& build() {
        re = std::regex(pattern);
        return *this;
    }

    // 匹配文本并将该捕获的捕获进去
    auto match(const std::string& code) const {
        std::sregex_iterator beg(code.cbegin(), code.cend(), re), end;
        return std::ranges::subrange(beg, end)
             | std::views::transform([&](const auto& m) {
                 T obj;
                 for (auto&& [i, p] : parts | std::views::filter([](auto&& p) { return p.capturing(); })
                                            | std::views::enumerate) {
                     p.assign(obj, m[i + 1]); // 0号为整组
                 }
                 return obj;
             });
    }
};
