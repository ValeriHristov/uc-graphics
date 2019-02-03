#include "pch.h"

#include <uc/gx/dx12/cmd/command_context.h>
#include <pix3.h>

namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            void gpu_command_context::pix_begin_event(const wchar_t* label)
            {
                PIXBeginEvent(list(), 0, label);
            }

            void gpu_command_context::pix_end_event(void)
            {
                PIXEndEvent(list());
            }

            void gpu_command_context::pix_set_marker(const wchar_t* label)
            {
                PIXSetMarker(list(), 0, label);
            }
        }
    }

}

