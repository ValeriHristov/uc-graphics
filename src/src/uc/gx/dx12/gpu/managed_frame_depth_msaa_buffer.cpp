#include "pch.h"

#include <uc/gx/dx12/gpu/resource_create_context.h>
#include <uc/gx/dx12/gpu/managed_frame_depth_msaa_buffer.h>


namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            namespace details
            {
                void gpu_frame_msaa_depth_buffer_deleter::operator() (gpu_frame_msaa_depth_buffer* d)
                {
                    m_rc->free_frame_msaa_depth_buffer(d);
                }
            }
        }
    }

}

