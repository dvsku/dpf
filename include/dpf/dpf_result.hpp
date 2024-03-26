#pragma once

#include <string>

namespace dvsku::dpf {
    /// <summary>
    /// dpf function result status
    /// </summary>
    enum class dpf_status : unsigned char {
        undefined = 0x0,
        cancelled = 0x1,
        error     = 0x2,
        ok        = 0x3
    };

    /// <summary>
    /// A common result object for dpf functions
    /// </summary>
    struct dpf_result {
        dpf_status  status  = dpf_status::undefined;
        std::string message = "";
    };
}