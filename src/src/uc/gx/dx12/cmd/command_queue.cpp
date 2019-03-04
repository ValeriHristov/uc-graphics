#include "pch.h"

#include <uc/gx/dx12/cmd/command_queue.h>
#include <pix3.h>

namespace uc
{
    namespace gx
    {
        namespace dx12
        {
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

