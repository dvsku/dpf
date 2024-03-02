#pragma once

#include "dpf_result.hpp"
#include "dpf_input_file.hpp"

#include <functional>
#include <filesystem>
#include <vector>

namespace dvsku::dpf {
    struct dpf_context {
        using start_callback_t  = std::function<void()>;
        using finish_callback_t = std::function<void(dpf_result)>;
        using update_callback_t = std::function<void(float)>;
        using error_callback_t  = std::function<void(dpf_result)>;
        using pre_process_t     = std::function<void(dpf_input_file& file, std::vector<char>& buffer)>;

        start_callback_t  callback_start  = nullptr;
        finish_callback_t callback_finish = nullptr;
        update_callback_t callback_update = nullptr;
        error_callback_t  callback_error  = nullptr;
        pre_process_t     pre_process_fn  = nullptr;
        bool*             cancel          = nullptr;

        std::filesystem::path base_path = "";

        void invoke_start();
        void invoke_finish(dpf_result& result);
        void invoke_update(float progress);
        void invoke_error(dpf_result& result);
        void invoke_pre_process(dpf_input_file& file, std::vector<char>& buffer);

        bool invoke_cancel();
    };
}