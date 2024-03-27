#include "dpf.hpp"
#include "utilities_assert.hpp"

#include <fstream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>

using namespace dvsku::dpf;

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

    inputs.base_path = std::string(base) + std::string("/tests/resources/patch");

    inputs.files.push_back(
        { std::string(base) + std::string("/tests/resources/patch/1.txt"), dpf_op::add }
    );

    inputs.files.push_back(
        { std::string(base) + std::string("/tests/resources/patch/subfolder/2.txt"), dpf_op::modify }
    );

    inputs.files.push_back(
        { std::string(base) + std::string("/tests/resources/patch/subfolder/3.txt"), dpf_op::remove }
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

static int patch(const std::string& base) {
    dpf dpf;

    try {
        ASSERT(copy_directory(std::string(base) + std::string("/tests/resources/original/"), "./to_patch/"));
        
        auto result = dpf.patch("./patch.dpf", "./to_patch/");

        ASSERT(result.status == dpf_status::ok);

        bool valid = compare_files("./to_patch/1.txt",           std::string(base) + std::string("/tests/resources/patch/1.txt")) &&
                     compare_files("./to_patch/subfolder/2.txt", std::string(base) + std::string("/tests/resources/patch/subfolder/2.txt")) &&
                     !std::filesystem::exists("./to_patch/subfolder/3.txt");

        ASSERT(valid);
    }
    catch (...) { return 1; }

    return 0;
}

static int check_checksum(const std::string& base) {
    dpf dpf;

    ASSERT(dpf.check_checksum(PATCH_FILE));
    ASSERT(!dpf.check_checksum(std::string(base) + std::string("/tests/resources/patch/1.txt")));

    return 0;
}

static int get_files(const std::string& base) {
    dpf                      dpf;
    std::vector<std::string> files;

    auto result = dpf.get_files(PATCH_FILE, files);

    ASSERT(result.status == dpf_status::ok);
    ASSERT(files.size() == 3);

    return 0;
}

static int is_dpf_file(const std::string& base) {
    dpf dpf;

    ASSERT(dpf.is_dpf_file(PATCH_FILE));
    ASSERT(!dpf.is_dpf_file(std::string(base) + std::string("/tests/resources/patch/1.txt")));

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        ASSERT(create_patch_file(argv[2]));

        switch (std::stoi(argv[1])) {
            case 0:  return patch(argv[2]);
            case 1:  return check_checksum(argv[2]);
            case 2:  return get_files(argv[2]);
            case 3:  return is_dpf_file(argv[2]);
            default: break;
        }
    }

    return 0;
}