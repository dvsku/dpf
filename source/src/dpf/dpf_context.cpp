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

void dpf_context::invoke_error(dpf_result& result) {
    if (callback_error)
        callback_error(result);
}

void dpf_context::invoke_buf_process(dpf_input_file& file, std::vector<char>& buffer) {
    if (buf_process_fn)
        buf_process_fn(file, buffer);
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
