#pragma once

#include "libdpf/dpf_context.hpp"
#include "libdpf/misc/dpf_result.hpp"
#include "libdpf/misc/dpf_inputs.hpp"

#include <filesystem>
#include <vector>
#include <string>

namespace libdpf {
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
        /*
            Check if a file is a DPF file.

            @returns TRUE if it's a DPF file, FALSE if it isn't or there's a failure
        */
        bool is_dpf_file(const FILE_PATH& file);

        /*
            Synchronously create a DPF file containing input files.
        */
        dpf_result create(dpf_inputs& input_files, const FILE_PATH& dpf_file, dpf_context* context = nullptr);

        /*
            Asynchronously create a DPF file containing input files.
        */
        void create_async(dpf_inputs& input_files, const FILE_PATH& dpf_file, dpf_context* context = nullptr);

        /*
            Synchronously patch a dir with a DPF file.
        */
        dpf_result patch(const FILE_PATH& dpf_file, const DIR_PATH& patch_dir, dpf_context* context = nullptr);

        /*
            Asynchronously patch a dir with a DPF file.
        */
        void patch_async(const FILE_PATH& dpf_file, const DIR_PATH& patch_dir, dpf_context* context = nullptr);

        /*
            Get files packed inside a DPF file.
        */
        dpf_result get_files(const FILE_PATH& dpf_file, std::vector<std::string>& files);

        /*
            Get packed patch version.
        */
        dpf_result get_patch_version(const FILE_PATH& dpf_file, uint64_t& version);

        /*
            Get unpacked patch version.
        */
        dpf_result get_patch_version(const FILE_PATH& dpf_file, uint16_t& version_major, uint16_t& version_minor, uint16_t& version_rev);

        /*
            Check DPF file checksum.
        */
        bool check_checksum(const FILE_PATH& dpf_file);
    };
}