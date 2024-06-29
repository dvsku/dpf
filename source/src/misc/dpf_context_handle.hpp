#pragma once

#include "libdpf/dpf_context.hpp"

namespace libdpf {
    class dpf_context_handle {
    public:
        dpf_context_handle() = delete;
        dpf_context_handle(dpf_context* context);

    public:
        void       invoke_start() const;
        void       invoke_finish(dpf_result& result) const;
        void       invoke_update(float change) const;
        dpf_result invoke_buf_process(const dpf_file_mod& file, std::vector<uint8_t>& buffer) const;

        bool is_cancelled() const;
        void invoke_cancel() const;

    private:
        dpf_context* m_context = nullptr;
    };
}
