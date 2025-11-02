#pragma once
#include <string>
#include <vector>
#include <format>

#include "RegexBuilder.hpp"

using namespace std::literals;

constexpr std::string_view AccessModifier = R"(public|private|protected|internal)";
constexpr std::string_view MemberModifier = R"((?:static|readonly|volatile|virtual|override|abstract|unsafe)\s+)";
constexpr std::string_view Type = R"([\w<>\[\]]+)";
constexpr std::string_view Identifier = R"([A-Za-z_]\w*)";
constexpr std::string_view Parameters = R"(\(([^)]*)\)\s*\{)";

template <typename Derived>
struct Base {
    std::string accessModifier;    // public, private, protected, internal
    std::string memberModifier;    // static, virtual, override, sealed, abstract, unsafe
    std::string type;              // 数据类型 / 返回类型
    std::string name;              // 名称

    static inline RegexBuilder<Derived> rb 
                = RegexBuilder<Derived>()
                 .join(&Base::accessModifier, AccessModifier)
                 .join_with("\\s+", &Base::memberModifier, MemberModifier, "?")
                 .join_with("\\s*", &Base::type, Type)
                 .join_with("\\s+", &Base::name, Identifier);
};

struct Method : public Base<Method>{
    std::string parameters;
};

struct Field : public Base<Field>{
    static inline RegexBuilder<Field> rb 
                = Base<Field>::rb
                 .join_with("\\s*", "=[^;]*", "?")
                 .join(";", false)
                 .build();
};
struct Property : public Base<Property> {};

template <typename Derived>
struct std::formatter<Base<Derived>> : std::formatter<std::string> {
    // 允许格式说明符（这里我们忽略它）
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    // 实际的格式化输出逻辑
    auto format(const Base<Derived>& b, std::format_context& ctx) const {
        std::string result;
        if (!b.accessModifier.empty())
            result += b.accessModifier + " ";
        if (!b.memberModifier.empty())
            result += b.memberModifier + " ";
        result += std::format("{} {}", b.type, b.name);
        return std::formatter<std::string>::format(result, ctx);
    }
};

template <>
struct std::formatter<Method> : std::formatter<Base<Method>> {
    auto format(const Method& m, std::format_context& ctx) const {
        std::string result = std::format("{}", static_cast<const Base<Method>&>(m));
        if (!m.parameters.empty())
            result += std::format("({})", m.parameters);
        else 
			result += "()";
        return std::formatter<std::string>::format(result, ctx);
    }
};

template <>
struct std::formatter<Field> : std::formatter<Base<Field>> {};

template <>
struct std::formatter<Property> : std::formatter<Base<Property>> {};


struct ClassInfo {
    std::string name;
    std::vector<Method> methods;
    std::vector<Field> fields;
    std::vector<Property> properties;
};

template <>
struct std::formatter<ClassInfo> : std::formatter<std::string> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const ClassInfo& c, std::format_context& ctx) const {
		std::string 成员前缀 = "  ";
		std::string 内容前缀 = 成员前缀 + "    ";
        std::string out = std::format("class {}\n", c.name);


        if (!c.fields.empty()) {
            out += std::format("{}{} 个 Fields:\n", 成员前缀, c.fields.size());
            for (auto& f : c.fields)
                out += std::format("{}{}\n", 内容前缀, f);
        }

        if (!c.properties.empty()) {
			out += std::format("{}{} 个 Properties:\n", 成员前缀, c.properties.size());
            for (auto& p : c.properties)
                out += std::format("{}{}\n", 内容前缀, p);
        }

        if (!c.methods.empty()) {
			out += std::format("{}{} 个 Methods:\n", 成员前缀, c.methods.size());
            for (auto& m : c.methods)
                out += std::format("{}{}\n", 内容前缀, m);
        }

        return std::formatter<std::string>::format(out, ctx);
    }
};
