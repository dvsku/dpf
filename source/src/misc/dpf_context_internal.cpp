#include "dpf_context_internal.hpp"

using namespace libdpf;

dpf_context_internal::dpf_context_internal(dpf_context* context)
    : m_context(context) {}

void dpf_context_internal::invoke_start() const {
    if (m_context && m_context->start_callback)
        m_context->start_callback();
}

void dpf_context_internal::invoke_finish(dpf_result& result) const {
    if (m_context && m_context->finish_callback)
        m_context->finish_callback(result);
}

void dpf_context_internal::invoke_update(float change) const {
    if (m_context && m_context->update_callback)
        m_context->update_callback(change);
}

dpf_result dpf_context_internal::invoke_buf_process(const dpf_file_mod& file, std::vector<uint8_t>& buffer) const {
    dpf_result result;

    if (!m_context || !m_context->buf_process_fn) {
        result.status = dpf_status::ok;
        return result;
    }

    return m_context->buf_process_fn(file, buffer);
}

bool dpf_context_internal::is_cancelled() const {
    if (!m_context || !m_context->cancel) return false;
    return m_context->cancel->load();
}

void dpf_context_internal::invoke_cancel() const {
    dpf_result result;
    result.status = dpf_status::cancelled;

    invoke_finish(result);
}
