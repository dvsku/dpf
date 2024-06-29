#pragma once

#include "libdpf/enums.hpp"

#include <string>

namespace libdpf {
    /*
        Common result object for dpf functions
    */
    struct dpf_result {
        dpf_status  status  = dpf_status::undefined;
        std::string message = "";
    };
}
