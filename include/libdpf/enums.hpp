#pragma once

namespace libdpf {
    /*
        File modification operation.
    */
    enum class dpf_op : unsigned char {
        undefined,
        add,
        remove,
        modify
    };

    /*
        Result status.
    */
    enum class dpf_status : unsigned char {
        undefined = 0,
        ok        = 1,
        cancelled = 2,
        failure   = 3
    };
}
