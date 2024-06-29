#pragma once

#include "libdpf/misc/dpf_file_mod.hpp"

#include <vector>

namespace libdpf {
    /*
        File modifications.
    */
    struct dpf_inputs {
        std::filesystem::path     base_path = "";
        uint64_t                  version   = 0U;
        std::vector<dpf_file_mod> files;
    };
}
