#include "pch.h"

#include <uc/gx/dx12/gpu/resource_create_context.h>
#include <uc/gx/dx12/gpu/managed_upload_buffer.h>


namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            namespace details
            {
                void gpu_upload_buffer_deleter::operator () (gpu_upload_buffer* d)
                {
                    m_rc->free_upload_buffer(d);
                }
            }
        }
    }

}

