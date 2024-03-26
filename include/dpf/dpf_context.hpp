#pragma once

#include "dpf_result.hpp"
#include "dpf_file_mod.hpp"

#include <functional>
#include <filesystem>
#include <vector>

namespace dvsku::dpf {
    /// <summary>
    /// dpf context object
    /// </summary>
    struct dpf_context {
        using start_callback_t  = std::function<void()>;
        using finish_callback_t = std::function<void(dpf_result)>;
        using update_callback_t = std::function<void(float)>;

        // Perform additional buffer processing before compression / after decompression
        // Considered successful if return status is finished, else it's an error
        using buf_process_t = std::function<dpf_result(const dpf_file_mod& file, std::vector<uint8_t>& buffer)>;

        start_callback_t  callback_start  = nullptr;
        finish_callback_t callback_finish = nullptr;
        update_callback_t callback_update = nullptr;
        buf_process_t     buf_process_fn  = nullptr;
        bool*             cancel          = nullptr;

        void invoke_start();
        void invoke_finish(dpf_result& result);
        void invoke_update(float progress);
        dpf_result invoke_buf_process(const dpf_file_mod& file, std::vector<uint8_t>& buffer);

        bool invoke_cancel();
    };
}