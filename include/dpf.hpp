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
        dpf_result create(dpf_inputs& input_files, const FILE_PATH& output_file, dpf_context* context = nullptr);
        void create_async(dpf_inputs& input_files, const FILE_PATH& output_file, dpf_context* context = nullptr);

        dpf_result extract(const FILE_PATH& input_file, const DIR_PATH& output_dir, dpf_context* context = nullptr);
        void extract_async(const FILE_PATH& input_file, const DIR_PATH& output_dir, dpf_context* context = nullptr);
    };
}