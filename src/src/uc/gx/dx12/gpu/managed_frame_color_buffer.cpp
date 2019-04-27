#include "pch.h"

#include <uc/gx/dx12/gpu/resource_create_context.h>
#include <uc/gx/dx12/gpu/managed_frame_color_buffer.h>

namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            namespace details
            {
                void gpu_frame_color_buffer_deleter::operator() (gpu_frame_color_buffer* d)
                {
                    m_rc->free_frame_color_buffer(d);
                }
            }
        }
    }

}

