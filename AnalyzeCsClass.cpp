#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <filesystem>
#include <format>
#include <print>
#include <sstream>
#include <locale>
#include "ClassInfo.hpp"
#include "Logger.hpp"
#include "bench_timer.hpp"
#include "RegexBuilder.hpp"

using namespace std;

namespace fs = std::filesystem;

string read_file(const fs::path& path) {
    ifstream file(path);
    if (!file)
        throw runtime_error(format("无法打开文件：{}", path.string()));
    return string(istreambuf_iterator<char>(file), {});
}

void write_file(const fs::path& path, const string& content) {
    ofstream file(path);
    if (!file)
        throw runtime_error(format("无法写入文件：{}", path.string()));
    file << content;
}

ClassInfo analyze_cs_class(const string& code) {
    ClassInfo info;
    smatch m;

    // 匹配类名
    regex classRegex(R"(class\s+([A-Za-z0-9_]+))");
    if (regex_search(code, m, classRegex))
        info.name = m[1];
    else
        throw runtime_error("未找到 class 定义");

    // 匹配方法
    regex methodRegex(
        R"((public|private|protected|internal)?\s*(?:static\s+)?([\w<>]+)\s+([\w_]+)\s*\(([^)]*)\)\s*\{)"
    );
    for (sregex_iterator it(code.begin(), code.end(), methodRegex), end; it != end; ++it) {
        Method method;
		method.accessModifier   = (*it)[1];
        method.type       = (*it)[2];
        method.name       = (*it)[3];
        method.parameters = (*it)[4];
        if (!method.type.empty() && method.name != "class")
            info.methods.push_back(method);
    }

    bench::Timer test1("测试1耗时");
    info.fields = RegexBuilder<Field>()
                .join_with("", &Field::accessModifier, AccessModifier)
                .join(&Field::memberModifier, MemberModifier, "?")
                .join_with("\\s*", &Field::type, Type)
                .join(&Field::name, Identifier)
                .join_with("\\s*", "=[^;]*", "?")
                .join_with("", ";", false)
                .build()
                .match(code)
                | ranges::to<std::vector>();
    test1.stop();


    //auto rb = RegexBuilder<Field>()
    //    .join_with("", &Field::accessModifier, AccessModifier)
    //    .join(&Field::memberModifier, MemberModifier, "?")
    //    .join_with("\\s*", &Field::type, Type)
    //    .join(&Field::name, Identifier)
    //    .join_with("\\s*", "=[^;]*", "?")
    //    .join_with("", ";", false)
    //    .build();
    //LOG_INFO("{}", rb.pattern);
    //auto r = rb.match(code);
    //LOG_INFO("test");
    //try {
    //    info.fields = r | std::ranges::to<vector<Field>>();
    //}
    //catch (const exception& e) {
    //    LOG_FATAL(e.what());
    //}

    //LOG_INFO("test2");

    std::vector<Field> fields;
    // fields.reserve(info.fields.size());
    bench::Timer test2("测试2");
    // 匹配字段（含类型）
    std::regex fieldRegex(
        R"((public|private|protected|internal|static|readonly|volatile)\s+([\w<>\[\]]+)\s+([A-Za-z_]\w*)\s*(?:=[^;]*)?;)"
    );
    for (std::sregex_iterator it(code.begin(), code.end(), fieldRegex), end; it != end; ++it) {
        Field field;
		field.accessModifier = (*it)[1];
        field.type           = (*it)[2];
        field.name           = (*it)[3];
        fields.push_back(field);
    }
    test2.stop();

    // 匹配属性（自动属性、表达式属性等）
    std::regex propertyRegex(
        R"((public|private|protected|internal|static|virtual|override)\s+([\w<>\[\]]+)\s+([A-Za-z_]\w*)\s*\{\s*(?:get|set|init)\b)"
    );
    for (std::sregex_iterator it(code.begin(), code.end(), propertyRegex), end; it != end; ++it) {
        Property prop;
		prop.accessModifier = (*it)[1];
        prop.type     = (*it)[2];
        prop.name     = (*it)[3];
        info.properties.push_back(prop);
    }

    return info;
}

string format_result(const ClassInfo& info) {
	return std::format("{}", info);
}

int main(int argc, char* argv[]) try {
    // std::locale::global(std::locale("en_US.UTF-8"));

    // ::simplelog::defaultLogger().log(::simplelog::Level::INFO, "你好");
    // LOG_DEBUG("debug测试");
    // LOG_ERROR("error测试");
    // LOG_FATAL("fatal测试");
    LOG_INFO("info测试");
    LOG_INFO("1 + 1 = {}", 1 + 1);
    LOG_TRACE("trace测试");

    fs::path input = ".\\test.txt";
    fs::path output = ".\\out.txt";

    BENCH_SCOPE("总耗时");

    println("正在读取 {}", input.string());
    bench::Timer timer("读取耗时");
    string code = read_file(input);
    timer.stop();

    println("正在分析类结构...");

    BENCH_SCOPE("分析耗时", simplelog::Level::DEBUG);
    BENCH_DEBUG("分析耗时2");
    ClassInfo info = analyze_cs_class(code);
    string result = format_result(info);
    write_file(output, result);

    println("已分析类: {}", info.name);
    println("结果已写入: {}", output.string());
    return 0;

}
catch (const exception& e) {
    cerr << "❌ 错误: " << e.what() << "\n";
    return 1;
}
