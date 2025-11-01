#pragma once
#include <string>
#include <vector>
#include <format>

struct Base {
    std::string accessModifier;    // public, private, protected, internal
    std::string memberModifier;    // static, virtual, override, sealed, abstract, unsafe
    std::string type;              // 数据类型 / 返回类型
    std::string name;              // 名称
};

struct Method : public Base {
    std::string parameters;
};

struct Field : public Base {};
struct Property : public Base {};

template <>
struct std::formatter<Base> : std::formatter<std::string> {
    // 允许格式说明符（这里我们忽略它）
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    // 实际的格式化输出逻辑
    auto format(const Base& b, std::format_context& ctx) const {
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
struct std::formatter<Method> : std::formatter<Base> {
    auto format(const Method& m, std::format_context& ctx) const {
        std::string result = std::format("{}", static_cast<const Base&>(m));
        if (!m.parameters.empty())
            result += std::format("({})", m.parameters);
        else 
			result += "()";
        return std::formatter<std::string>::format(result, ctx);
    }
};

template <>
struct std::formatter<Field> : std::formatter<Base> {};

template <>
struct std::formatter<Property> : std::formatter<Base> {};


struct ClassInfo {
    std::string name;
    std::vector<Method> methods;
    std::vector<Field> fields;
    std::vector<Property> properties;
};
