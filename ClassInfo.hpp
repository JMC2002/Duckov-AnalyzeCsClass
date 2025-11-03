#pragma once
#include <string>
#include <vector>
#include <format>

#include "RegexBuilder.hpp"

using namespace std::literals;

constexpr std::string_view Type = R"([\w<>.,\[\]\s?]+)";
constexpr std::string_view Identifier = R"([A-Za-z_]\w*)";
constexpr std::string_view GenericName = R"([A-Za-z_]\w*(?:<[\w,\s<>]+>)?)";
constexpr std::string_view Parameters = R"([^)]*)";

// 暂时不匹配没有modifier的语句
constexpr std::string_view FieldModifier     = R"((?:(?:new|public|protected|internal|private|static|readonly|volatile|unsafe)\s+)+)";
constexpr std::string_view EventModifier     = R"((?:(?:new|public|protected|internal|private|static|readonly|volatile|unsafe)\s+)+\s*event)";
constexpr std::string_view ClassModifier     = R"((?:(?:new|public|protected|internal|private|abstract|sealed|static|unsafe)\s+)+)";
// Method、Property、Event的Modifier限制都一样
constexpr std::string_view Modifier    = R"((?:(?:new|public|protected|internal|private|static|virtual|sealed|override|abstract|extern|readonly|unsafe)\s+)+)";
constexpr std::string_view ConstantModifier  = R"((?:(?:new|public|protected|internal|private)\s+)*\s*const)";
constexpr std::string_view Super = R"((?:\s*:\s*[\w<>,\.\s]*)?)";

template <typename Derived>
struct Base {
    std::string modifier;
    std::string type;              // 数据类型 / 返回类型
    std::string name;              // 名称

    static inline constexpr std::string_view modifierRe   = Modifier;
    static inline constexpr std::string_view typeRe       = Type;
    static inline constexpr std::string_view identifierRe = Identifier;

    static auto& getBuilder() {
        static auto rb = RegexBuilder<Derived>()
                              .join(&Derived::modifier, modifierRe)
                              .join_with("\\s*", &Derived::type, typeRe)
                              .join_with("\\s+", &Derived::name, identifierRe);
        return rb;
    }
};                                                               

struct Method : public Base<Method>{
    std::string parameters;
    
    static inline constexpr std::string_view identifierRe = GenericName;
    static auto& getBuilder() {
        static auto rb = Base<Method>::getBuilder()
                        .join_with(R"(\s*\()", &Method::parameters, Parameters)
                        .join(R"(\)\s*(?:\{|=>|;))", false)
                        .build();
        return rb;
    }
};

struct Field : public Base<Field>{
    static inline constexpr std::string_view modifierRe = FieldModifier;
    static auto& getBuilder() {
        static auto rb = Base<Field>::getBuilder()
                        .join_with("\\s*", "=[^>;]*", "?")
                        .join(";", false)
                        .build();
        return rb;
    }
};

struct Constant : public Base<Field> {
    static inline constexpr std::string_view modifierRe = ConstantModifier;
    static auto& getBuilder() {
        static auto rb = Base<Constant>::getBuilder()
                        .join_with("\\s*", "=[^>;]*", "?")
                        .join(";", false)
                        .build();
        return rb;
    }
};

struct Property : public Base<Property> {
    static auto& getBuilder() {
        static auto rb = Base<Property>::getBuilder()
                        .join_with("\\s*", R"(\{\s*(?:get|set|init)\b|=>)", false)
                        .build();
        return rb;
    }
};

struct Event : public Base<Event> {
    static inline constexpr std::string_view modifierRe = EventModifier;

    static auto& getBuilder() {
        static auto rb = Base<Event>::getBuilder()
                        .join_with("\\s*", R"(\{\s*(?:add|remove)\b|=>)", false)
                        .build();
        return rb;
    }
};

template <typename Derived>
struct std::formatter<Base<Derived>> : std::formatter<std::string> {
    // 允许格式说明符（这里我们忽略它）
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    // 实际的格式化输出逻辑
    auto format(const Base<Derived>& b, std::format_context& ctx) const {
        std::string result;
        if (!b.modifier.empty())
            result += b.modifier;   // 现在的捕获会捕获最后一个空格，输出直接就不加空格了
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


struct ClassInfo : public Base<ClassInfo> {
    static inline constexpr std::string_view modifierRe = ClassModifier;
    static inline constexpr std::string_view typeRe = "class";
    static inline constexpr std::string_view identifierRe = GenericName;

    std::string super;            // 父类

    static auto& getBuilder() {
        static auto rb = Base<ClassInfo>::getBuilder()
                        .join_with("\\s*", &ClassInfo::super, Super)
                        .join_with("\\s*", "\\{", false)
                        .build();
        return rb;
    }


    std::vector<Method> methods;
    std::vector<Field> fields;
    std::vector<Property> properties;
    std::vector<Constant> constants;
    std::vector<Event> events;

    void match(const std::string& code) {

    }
};

template <>
struct std::formatter<ClassInfo> : std::formatter<std::string> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const ClassInfo& c, std::format_context& ctx) const {
		std::string 成员前缀 = "  ";
		std::string 内容前缀 = 成员前缀 + "    ";
        std::string out = std::format("{}class {}", c.modifier, c.name);
        if (c.super.size())
            out += std::format("{}", c.super);
        out += '\n';

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
