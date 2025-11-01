#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <filesystem>
#include <format>
#include <print>
#include <sstream>
#include "ClassInfo.hpp"

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

    // 匹配字段（含类型）
    std::regex fieldRegex(
        R"((public|private|protected|internal|static|readonly|volatile)\s+([\w<>\[\]]+)\s+([A-Za-z_]\w*)\s*(?:=[^;]*)?;)"
    );
    for (std::sregex_iterator it(code.begin(), code.end(), fieldRegex), end; it != end; ++it) {
        Field field;
		field.accessModifier = (*it)[1];
        field.type     = (*it)[2];
        field.name     = (*it)[3];
        info.fields.push_back(field);
    }

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
    string output;
    output += format("Class: {}\n\n", info.name);

    output += "Methods:\n";
    for (auto& m : info.methods)
        output += format("\t{}\n", m);

    output += "\nProperties:\n";
    for (auto& p : info.properties)
        output += format("\t{}\n", p);

    output += "\nFields:\n";
    for (auto& f : info.fields)
        output += format("\t{}\n", f);

    return output;
}

int main(int argc, char* argv[]) try {
    fs::path input = ".\\test.txt";
    fs::path output = ".\\out.txt";

    println("📖 正在读取 {}", input.string());
    string code = read_file(input);
    println("🔍 正在分析类结构...");
    ClassInfo info = analyze_cs_class(code);
    string result = format_result(info);
    write_file(output, result);

    println("✅ 已分析类: {}", info.name);
    println("📄 结果已写入: {}", output.string());
    return 0;

}
catch (const exception& e) {
    cerr << "❌ 错误: " << e.what() << "\n";
    return 1;
}
