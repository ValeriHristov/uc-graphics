#include "pch.h"

#include <uc/gx/dx12/cmd/command_context.h>
#include <pix3.h>
#include <uc/gx/dx12/gpu/resource_utils.h>

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

			void gpu_command_context::upload_resource(gpu_resource* r, uint32_t first_sub_resource, uint32_t sub_resource_count, D3D12_SUBRESOURCE_DATA sub_resource_data[])
			{
				uint64_t upload_size = GetRequiredIntermediateSize(r->resource(), first_sub_resource, sub_resource_count);

				if (upload_size > 0 && sub_resource_count < 64)
				{
					auto allocation = m_upload_allocator.allocate(upload_size, 512);
					UpdateSubresources<64>(list(), r->resource(), allocation.resource()->resource(), allocation.offset(), first_sub_resource, sub_resource_count, sub_resource_data);
				}
			}
        }
    }

}

