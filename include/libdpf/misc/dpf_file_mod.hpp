#pragma once

#include "libdpf/enums.hpp"

#include <vector>
#include <filesystem>

namespace libdpf {
    /*
        File modification.
    */
    struct dpf_file_mod {
        std::filesystem::path path = "";
        dpf_op                op   = dpf_op::undefined;
    };
}
