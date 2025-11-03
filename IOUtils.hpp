#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <format>
#include <stdexcept>

namespace IOUtils {

    namespace fs = std::filesystem;
    using namespace std::string_literals;

    /// 读取整个文件到字符串
    inline std::string read_file(const fs::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            throw std::runtime_error(std::format("无法打开文件：{}", path.string()));
        return std::string(std::istreambuf_iterator<char>(file), {});
    }

    /// 写字符串到文件（自动创建父目录）
    inline void write_file(const fs::path& path, const std::string& content) {
        fs::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::binary);
        if (!file)
            throw std::runtime_error(std::format("无法写入文件：{}", path.string()));
        file << content;
    }

    /// 扫描目录下所有指定扩展名的文件（默认 `.txt`）
    template <bool Recursive = true>
    inline std::vector<fs::path> list_files(
        const fs::path& root,
        const std::string& extension = ".txt")
    {
        std::vector<fs::path> results;
        if (!fs::exists(root))
            throw std::runtime_error(std::format("路径不存在：{}", root.string()));

        using DirIter = std::conditional_t<Recursive, fs::recursive_directory_iterator, fs::directory_iterator>;
        for (const auto& entry : DirIter(root)) {
            if (entry.is_regular_file() && entry.path().extension() == extension)
                results.push_back(entry.path());
        }

        return results;
    }

    /// 根据输入文件路径生成输出文件路径（保持层级）
    inline fs::path make_output_path(
        const fs::path& inputFile,
        const fs::path& inputRoot,
        const fs::path& outputRoot,
        const std::string& newExt = ".out.txt")
    {
        fs::path rel = fs::relative(inputFile, inputRoot);
        rel.replace_extension(newExt);
        return outputRoot / rel;
    }

} // namespace IOUtils
