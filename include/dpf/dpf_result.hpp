#pragma once

#include <string>

namespace dvsku::dpf {
    enum class dpf_status : unsigned char {
        undefined = 0x0,
        cancelled = 0x1,
        error     = 0x2,
        finished  = 0x3
    };

    struct dpf_result {
        dpf_status  status  = dpf_status::undefined;
        std::string message = "";
    };
}