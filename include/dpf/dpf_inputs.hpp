#pragma once

#include "dpf_file_mod.hpp"

#include <vector>

namespace dvsku::dpf {
    /// <summary>
    /// A collection of file modifications
    /// </summary>
    struct dpf_inputs {
        std::filesystem::path     base_path = "";
        std::vector<dpf_file_mod> files;
    };
}