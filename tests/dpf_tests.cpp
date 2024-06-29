#include "libdpf.hpp"

#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>

using namespace libdpf;

#define PATCH_FILE "./patch.dpf"

static bool compare_files(const std::string& p1, const std::string& p2) {
    std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

    if (f1.fail() || f2.fail()) {
        return false;
    }

    if (f1.tellg() != f2.tellg()) {
        return false;
    }

    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
        std::istreambuf_iterator<char>(),
        std::istreambuf_iterator<char>(f2.rdbuf()));
}

static bool create_patch_file(const std::string& base) {
    dpf        dpf;
    dpf_inputs inputs;

    inputs.base_path = std::string(base) + std::string("/resources/patch");

    inputs.files.push_back(
        { std::string(base) + std::string("/resources/patch/1.txt"), dpf_op::add }
    );

    inputs.files.push_back(
        { std::string(base) + std::string("/resources/patch/subfolder/2.txt"), dpf_op::modify }
    );

    inputs.files.push_back(
        { std::string(base) + std::string("/resources/patch/subfolder/3.txt"), dpf_op::remove }
    );

    auto result = dpf.create(inputs, "./patch.dpf");

    return result.status == dpf_status::ok;
}

static bool copy_directory(const std::filesystem::path& source, const std::filesystem::path& destination) {
    try {
        if (std::filesystem::exists(source) && std::filesystem::is_directory(source)) {
            if (!std::filesystem::exists(destination)) {
                std::filesystem::create_directories(destination);
            }

            for (const auto& entry : std::filesystem::directory_iterator(source)) {
                std::filesystem::path current_destination = destination / entry.path().filename();

                if (std::filesystem::is_directory(entry.status())) {
                    copy_directory(entry.path(), current_destination);
                }
                else {
                    std::filesystem::copy(entry.path(), current_destination, std::filesystem::copy_options::overwrite_existing);
                }
            }
        }
        else {
            return false;
        }
    }
    catch (...) {
        return false;
    }

    return true;
}

TEST(dpf, patching) {
    dpf dpf;

    ASSERT_TRUE(create_patch_file(BASE_PATH));
    ASSERT_TRUE(copy_directory(std::string(BASE_PATH) + std::string("/resources/original/"), "./to_patch/"));

    auto result = dpf.patch("./patch.dpf", "./to_patch/");

    ASSERT_TRUE(result.status == dpf_status::ok);
    ASSERT_TRUE(compare_files("./to_patch/1.txt", std::string(BASE_PATH) + std::string("/resources/patch/1.txt")));
    ASSERT_TRUE(compare_files("./to_patch/subfolder/2.txt", std::string(BASE_PATH) + std::string("/resources/patch/subfolder/2.txt")));
    ASSERT_FALSE(std::filesystem::exists("./to_patch/subfolder/3.txt"));
}

TEST(dpf, checksum) {
    dpf dpf;

    ASSERT_TRUE(create_patch_file(BASE_PATH));
    ASSERT_TRUE(dpf.check_checksum(PATCH_FILE));
    ASSERT_FALSE(dpf.check_checksum(std::string(BASE_PATH) + std::string("/resources/patch/1.txt")));
}

TEST(dpf, get_files) {
    dpf                      dpf;
    std::vector<std::string> files;

    ASSERT_TRUE(create_patch_file(BASE_PATH));

    auto result = dpf.get_files(PATCH_FILE, files);

    ASSERT_TRUE(result.status == dpf_status::ok);
    ASSERT_TRUE(files.size() == 3);
}

TEST(dpf, is_dpf_file) {
    dpf dpf;

    ASSERT_TRUE(create_patch_file(BASE_PATH));
    ASSERT_TRUE(dpf.is_dpf_file(PATCH_FILE));
    ASSERT_FALSE(dpf.is_dpf_file(std::string(BASE_PATH) + std::string("/resources/patch/1.txt")));
}
