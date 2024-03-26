#include "dpf\dpf_context.hpp"

using namespace dvsku::dpf;

void dpf_context::invoke_start() {
    if (callback_start)
        callback_start();
}

void dpf_context::invoke_finish(dpf_result& result) {
    if (callback_finish)
        callback_finish(result);
}

void dpf_context::invoke_update(float progress) {
    if (callback_update)
        callback_update(progress);
}

dpf_result dpf_context::invoke_buf_process(const dpf_file_mod& file, std::vector<uint8_t>& buffer) {
    dpf_result result;
    
    if (!buf_process_fn) {
        result.status = dpf_status::ok;
        return result;
    }

    return buf_process_fn(file, buffer);
}

bool dpf_context::invoke_cancel() {
    if (cancel && *cancel) {
        dpf_result result;
        result.status = dpf_status::cancelled;

        invoke_finish(result);
        return true;
    }

    return false;
}
