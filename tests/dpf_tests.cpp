#include "dpf.hpp"
#include "utilities_assert.hpp"

#include <fstream>
#include <string>
#include <algorithm>
#include <filesystem>

using namespace dvsku::dpf;

bool compare_files(const std::string& p1, const std::string& p2) {
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

bool copy_directory(const std::filesystem::path& source, const std::filesystem::path& destination) {
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

int create_and_extract(const std::string& base) {
    dpf dpf;
    
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

    try {
        

        auto result = dpf.create(inputs, "./patch.dpf");
        bool valid  = result.status == dpf_status::finished;
        
        if (!valid)
            std::remove("./patch.dpf");

        ASSERT(valid);
        ASSERT(copy_directory(std::string(base) + std::string("/tests/resources/original/"), "./to_patch/"));
        
        result = dpf.patch("./patch.dpf", "./to_patch/");
        valid  = result.status == dpf_status::finished;

        if (!valid) {
            std::remove("./patch.dpf");
            std::filesystem::remove_all("./to_patch/");
        }

        ASSERT(valid);
        std::remove("./patch.dpf");

        valid = compare_files("./to_patch/1.txt",           std::string(base) + std::string("/tests/resources/patch/1.txt")) &&
                compare_files("./to_patch/subfolder/2.txt", std::string(base) + std::string("/tests/resources/patch/subfolder/2.txt")) &&
                !std::filesystem::exists("./to_patch/subfolder/3.txt");

        if (!valid)
            std::filesystem::remove_all("./to_patch/");

        ASSERT(valid);
        std::filesystem::remove_all("./to_patch/");
    }
    catch (...) { return 1; }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        switch (std::stoi(argv[1])) {
            case 0:  return create_and_extract(argv[2]);
            default: break;
        }
    }

    return 0;
}