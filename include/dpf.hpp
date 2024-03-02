#pragma once

#include "dpf\dpf_context.hpp"
#include "dpf\dpf_result.hpp"
#include "dpf\dpf_input_file.hpp"

#include <filesystem>

namespace dvsku::dpf {
    class dpf {
    public:
        using FILE_PATH = std::filesystem::path;
        using DIR_PATH  = std::filesystem::path;

    public:
        dpf()           = default;
        dpf(const dpf&) = delete;
        dpf(dpf&&)      = delete;

        dpf& operator=(const dpf&) = delete;
        dpf& operator=(dpf&&)      = delete;

    public:
        dpf_result create(std::vector<dpf_input_file>& input_files, const FILE_PATH& output_file, dpf_context* context = nullptr);
        void create_async(std::vector<dpf_input_file>& input_files, const FILE_PATH& output_file, dpf_context* context = nullptr);
    };
}