#pragma once

#include <vector>
#include <filesystem>

namespace dvsku::dpf {
    enum class dpf_op : unsigned char {
        undefined,
        add,
        remove,
        modify
    };

    struct dpf_input_file {
        std::filesystem::path path = "";
        dpf_op                op   = dpf_op::undefined;
    };

    struct dpf_inputs {
        std::filesystem::path       base_path = "";
        std::vector<dpf_input_file> files;
    };
}