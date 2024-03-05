#pragma once

#include "dpf_result.hpp"
#include "dpf_file_mod.hpp"

#include <functional>
#include <filesystem>
#include <vector>

namespace dvsku::dpf {
    struct dpf_context {
        using start_callback_t  = std::function<void()>;
        using finish_callback_t = std::function<void(dpf_result)>;
        using update_callback_t = std::function<void(float)>;
        using error_callback_t  = std::function<void(dpf_result)>;
        using buf_process_t     = std::function<void(dpf_file_mod& file, std::vector<char>& buffer)>;

        start_callback_t  callback_start  = nullptr;
        finish_callback_t callback_finish = nullptr;
        update_callback_t callback_update = nullptr;
        error_callback_t  callback_error  = nullptr;
        buf_process_t     buf_process_fn  = nullptr;
        bool*             cancel          = nullptr;

        void invoke_start();
        void invoke_finish(dpf_result& result);
        void invoke_update(float progress);
        void invoke_error(dpf_result& result);
        void invoke_buf_process(dpf_file_mod& file, std::vector<char>& buffer);

        bool invoke_cancel();
    };
}