#pragma once

#include "dpf\dpf_context.hpp"
#include "dpf\dpf_result.hpp"
#include "dpf\dpf_inputs.hpp"

#include <filesystem>
#include <vector>
#include <string>

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
        /// <summary>
        /// Check if a file is a DPF file
        /// </summary>
        /// <returns>TRUE if it's a DPF file, FALSE if it isn't or there's an error</returns>
        bool is_dpf_file(const FILE_PATH& file);

        /// <summary>
        /// Synchronously create a DPF file containing input files
        /// </summary>
        dpf_result create(dpf_inputs& input_files, const FILE_PATH& dpf_file, dpf_context* context = nullptr);

        /// <summary>
        /// Asynchronously create a DPF file containing input files
        /// </summary>
        void create_async(dpf_inputs& input_files, const FILE_PATH& dpf_file, dpf_context* context = nullptr);

        /// <summary>
        /// Synchronously patch a dir with a DPF file
        /// </summary>
        dpf_result patch(const FILE_PATH& dpf_file, const DIR_PATH& patch_dir, dpf_context* context = nullptr);

        /// <summary>
        /// Asynchronously patch a dir with a DPF file
        /// </summary>
        void patch_async(const FILE_PATH& dpf_file, const DIR_PATH& patch_dir, dpf_context* context = nullptr);

        /// <summary>
        /// Get files packed inside a DPF file
        /// </summary>
        dpf_result get_files(const FILE_PATH& dpf_file, std::vector<std::string>& files);

        /// <summary>
        /// Get packed patch version
        /// </summary>
        dpf_result get_patch_version(const FILE_PATH& dpf_file, uint64_t& version);

        /// <summary>
        /// Get unpacked patch version
        /// </summary>
        dpf_result get_patch_version(const FILE_PATH& dpf_file, uint16_t& version_major, uint16_t& version_minor, uint16_t& version_rev);

        /// <summary>
        /// Check DPF file checksum
        /// </summary>
        bool check_checksum(const FILE_PATH& dpf_file);
    };
}