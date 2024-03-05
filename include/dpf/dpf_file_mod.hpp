#pragma once

#include <vector>
#include <filesystem>

namespace dvsku::dpf {
    /// <summary>
    /// File modification operation
    /// </summary>
    enum class dpf_op : unsigned char {
        undefined,
        add,
        remove,
        modify
    };

    /// <summary>
    /// A file modification
    /// </summary>
    struct dpf_file_mod {
        std::filesystem::path path = "";
        dpf_op                op   = dpf_op::undefined;
    };
}