#pragma once

#include "libdpf/misc/dpf_result.hpp"
#include "libdpf/misc/dpf_file_mod.hpp"

#include <functional>
#include <vector>
#include <atomic>

namespace libdpf {
    /*
        dpf context object

        Contains user bound callbacks and work cancelling.
    */
    struct dpf_context {
        using start_callback_t  = std::function<void()>;
        using finish_callback_t = std::function<void(dpf_result)>;
        using update_callback_t = std::function<void(float)>;
        using buf_process_fn_t  = std::function<dpf_result(const dpf_file_mod& file, std::vector<uint8_t>& buffer)>;

        /*
            Start callback.

            @param void()
        */
        start_callback_t start_callback = nullptr;

        /*
            Finish callback.

            @param void(dpf_result)
        */
        finish_callback_t finish_callback = nullptr;

        /*
            Update callback.

            @param void(float) -> progress change
        */
        update_callback_t update_callback = nullptr;

        /*
            Buffer processing before compression / after decompression.
            Considered successful if return status is finished.

            @param dpf_result(const dpf_file_mod&, std::vector<uint8_t>&)
        */
        buf_process_fn_t buf_process_fn  = nullptr;

        /*
            Cancel token.
        */
        std::atomic_bool* cancel = nullptr;
    };
}
