#pragma once
#include <string>
#include <vector>
#include <format>
#include <variant>

#include "RegexBuilder.hpp"

using namespace std::literals;

constexpr std::string_view Type = R"([\w<>.,\[\]\s?]+)";
constexpr std::string_view AntiConstType = R"((?!const\b|event\b)[\w<>.,\[\]\s?]+)";
constexpr std::string_view Identifier = R"([A-Za-z_]\w*)";
constexpr std::string_view GenericName = R"([A-Za-z_]\w*(?:<[\w,\s<>]+>)?)";
constexpr std::string_view Parameters = R"([^)]*)";

// 暂时不匹配没有modifier的语句
constexpr std::string_view FieldModifier     = R"((?:(?:new|public|protected|internal|private|static|readonly|volatile|unsafe)\s+)+)";
constexpr std::string_view EventModifier     = R"((?:(?:new|public|protected|internal|private|static|readonly|volatile|unsafe)\s+)+\s*event\s+)";
constexpr std::string_view ClassModifier     = R"((?:(?:new|public|protected|internal|private|abstract|sealed|static|unsafe)\s+)+)";
// Method、Property、Event的Modifier限制都一样
constexpr std::string_view Modifier    = R"((?:(?:new|public|protected|internal|private|static|virtual|sealed|override|abstract|extern|readonly|unsafe)\s+)+)";
constexpr std::string_view ConstantModifier  = R"((?:(?:new|public|protected|internal|private)\s+)*\s*const\s+)";
constexpr std::string_view Super = R"((?:\s*:\s*[\w<>,\.\s]*)?)";

template <typename T>
using RegexMatchView = decltype(std::declval<RegexBuilder<T>>().match(std::declval<std::string>()));

template <typename Derived>
struct Base {
    std::string modifier;
    std::string type;              // 数据类型 / 返回类型
    std::string name;              // 名称

    static inline constexpr std::string_view modifierRe   = Modifier;
    static inline constexpr std::string_view typeRe       = AntiConstType;
    static inline constexpr std::string_view identifierRe = Identifier;

    static auto& getBuilder() {
        static auto rb = RegexBuilder<Derived>()
                              .join(&Derived::modifier, Derived::modifierRe)
                              .join_with("\\s*", &Derived::type, Derived::typeRe)
                              .join_with("\\s+", &Derived::name, Derived::identifierRe);
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

struct Constant : public Base<Constant> {
    static inline constexpr std::string_view modifierRe = ConstantModifier;
    static inline constexpr std::string_view typeRe = Type;    // 默认情况不会匹配event和const
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
    static inline constexpr std::string_view typeRe = Type;

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

template <>
struct std::formatter<Constant> : std::formatter<Base<Constant>> {};

template <>
struct std::formatter<Event> : std::formatter<Base<Event>> {};


// 存储view很影响format性能，留作备用
//using AnyMatchView = std::variant<
//    RegexMatchView<Method>,
//    RegexMatchView<Field>,
//    RegexMatchView<Property>,
//    RegexMatchView<Constant>,
//    RegexMatchView<Event>
//>;
//
//using MemberArr = std::array<std::pair<std::string_view, std::optional<AnyMatchView>>, std::variant_size_v<AnyMatchView>>;

using AnyMatchView = std::variant<
    std::vector<Method>,
    std::vector<Field>,
    std::vector<Property>,
    std::vector<Constant>,
    std::vector<Event>
>;

using MemberArr = std::array<std::pair<std::string_view, AnyMatchView>, std::variant_size_v<AnyMatchView>>;

struct ClassInfo : public Base<ClassInfo> {
    static inline constexpr std::string_view modifierRe = ClassModifier;
    static inline constexpr std::string_view typeRe = "class";
    static inline constexpr std::string_view identifierRe = GenericName;

    std::string namespaceName; // 命名空间
    std::string super;            // 父类
    
    static auto& getBuilder() {
        static auto rb = Base<ClassInfo>::getBuilder()
            .join_with("\\s*", &ClassInfo::super, Super)
            .join_with("\\s*", "\\{", false)
            .build();
        return rb;
    }

    MemberArr members;
    ClassInfo() = default;
    ClassInfo(const std::string& code) {

        auto r = ClassInfo::getBuilder().match(code) | std::ranges::to<std::vector>();
        if (r.size() && r[0].name.size())
            *this = r[0];
        else {
            LOG_WARN("未找到 class 定义");
        }

        if (auto ns = matchNamespace(code)) {
            namespaceName = *ns;
            LOG_DEBUG("namespace : {}", namespaceName);
        }
        else
            LOG_DEBUG("没找到namespace");

        members = MemberArr{
            std::pair{   "Method"sv,   Method::getBuilder().match(code) | std::ranges::to<std::vector>() },
            std::pair{    "Field"sv,    Field::getBuilder().match(code) | std::ranges::to<std::vector>() },
            std::pair{ "Property"sv, Property::getBuilder().match(code) | std::ranges::to<std::vector>() },
            std::pair{ "Constant"sv, Constant::getBuilder().match(code) | std::ranges::to<std::vector>() },
            std::pair{    "Event"sv,    Event::getBuilder().match(code) | std::ranges::to<std::vector>() },
        };
    }

    static std::optional<std::string> matchNamespace(const std::string& code) {
        static const std::regex nsRe(R"(namespace\s+([\w\.]+)\s*\{)");
        std::smatch m;
        if (std::regex_search(code, m, nsRe))
            return m[1].str();
        return std::nullopt;
    }
};

template <>
struct std::formatter<ClassInfo> : std::formatter<std::string> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const ClassInfo& c, std::format_context& ctx) const {
		std::string 成员前缀 = "  ";
		std::string 内容前缀 = 成员前缀 + "    ";
        std::string out;
        if (!c.namespaceName.empty())
            out += std::format("namespace {}\n", c.namespaceName);
        
        out += std::format("{}class {}", c.modifier, c.name);
        if (c.super.size())
            out += std::format("{}", c.super);
        out += '\n';

        for (auto&& [name, m] : c.members) {
            std::visit([&](auto&& v) {
                if (v.empty()) return;
                out += std::format("{}{} 个 {}:\n", 成员前缀, v.size(), name);
                for (auto&& mem : v)
                    out += std::format("{}{}\n", 内容前缀, mem);
                }, m);
        }

        return std::formatter<std::string>::format(out, ctx);
    }
};
