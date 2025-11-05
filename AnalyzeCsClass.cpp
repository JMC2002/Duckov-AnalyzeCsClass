#include <fstream>
#include <filesystem>
#include <format>

#include "ClassInfo.hpp"
#include "Logger.hpp"
#include "bench_timer.hpp"
#include "IOUtils.hpp"

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) try {
    fs::path inputDir = ".\\input";
    fs::path outputDir = ".\\output";

    LOG_DEBUG("ClassLike: {}", ClassLike::getBuilder().pattern);
    LOG_DEBUG("   Method: {}",    Method::getBuilder().pattern);
    LOG_DEBUG("    Field: {}",     Field::getBuilder().pattern);
    LOG_DEBUG(" Property: {}",  Property::getBuilder().pattern);
    LOG_DEBUG(" Constant: {}",  Constant::getBuilder().pattern);
    LOG_DEBUG("    Event: {}",     Event::getBuilder().pattern);

    BENCH_SCOPE("总耗时");

    LOG_INFO("正在扫描 {}", inputDir.string());
    auto files = IOUtils::list_files(inputDir, ".cs");

    LOG_INFO("共发现 {} 个文件", files.size());

    for (const auto& file : files) {
        BENCH_SCOPE(std::format("处理文件 {}", file.filename().string()));

        string code = IOUtils::read_file(file);

        ClassInfo info(code);

        string result = std::format("{}", info);

        auto outPath = IOUtils::make_output_path(file, inputDir, outputDir);
        IOUtils::write_file(outPath, result);

        LOG_INFO("→ 已写入 {}", outPath.string());
    }

    return 0;
}
catch (const exception& e) {
    LOG_ERROR("错误: {}\n", e.what());
    return 1;
}

