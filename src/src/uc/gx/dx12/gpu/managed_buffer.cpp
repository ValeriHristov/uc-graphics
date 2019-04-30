#include "pch.h"

#include <uc/gx/dx12/gpu/resource_create_context.h>
#include <uc/gx/dx12/gpu/managed_buffer.h>


namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            namespace details
            {
                void gpu_buffer_deleter::operator () (gpu_buffer* d)
                {
                    m_rc->free_buffer(d);
                }

				void byteaddress_gpu_buffer_deleter::operator () (byteaddress_gpu_buffer* d)
				{
					m_rc->free_byteaddress_buffer(d);
				}
            }
        }
    }

}

