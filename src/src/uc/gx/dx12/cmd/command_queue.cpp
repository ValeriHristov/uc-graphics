#include "pch.h"

#include <uc/gx/dx12/cmd/command_queue.h>
#include <pix3.h>

namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            void gpu_command_queue::signal_fence(ID3D12Fence* f, uint64_t value) 
            {
                throw_if_failed(m_queue->Signal(f, value));
            }

            void gpu_command_queue::signal_fence(uint64_t value) 
            {
                throw_if_failed(m_queue->Signal(m_fence.Get(), value));
            }

            void gpu_command_queue::pix_begin_event(const wchar_t* label)
            {
                PIXBeginEvent(m_queue.Get(), 0, label);
            }

            void gpu_command_queue::pix_end_event(void)
            {
                PIXEndEvent(m_queue.Get());
            }

            void gpu_command_queue::pix_set_marker(const wchar_t* label)
            {
                PIXSetMarker(m_queue.Get(), 0, label);
            }
        }
    }

}

