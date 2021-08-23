#pragma once

#include "virtual_resource.h"
#include "descriptor_heap.h"


namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            class gpu_buffer : public gpu_virtual_resource
            {
                private:

                    using base = gpu_virtual_resource;

                public:

					gpu_buffer() = default;
					~gpu_buffer() = default;

                    gpu_buffer(ID3D12Resource* resource) :
                        base(resource)
                    {

                    }
            };

			class gpu_byteaddress_buffer : public gpu_buffer
			{
				using base = gpu_buffer;

				public:

                gpu_byteaddress_buffer() = default;
				~gpu_byteaddress_buffer() = default;

                gpu_byteaddress_buffer(ID3D12Resource* resource, descriptor_handle srv, descriptor_handle uav) :
					base(resource)
					, m_srv(srv)
					, m_uav(uav)
				{

				}

				descriptor_handle srv() const
				{
					return m_srv;
				}

				descriptor_handle uav() const
				{
					return m_uav;
				}

				private:
                descriptor_handle    m_srv;
                descriptor_handle    m_uav;
			};

			class gpu_frame_byteaddress_buffer : public gpu_byteaddress_buffer
			{
				using base = gpu_byteaddress_buffer;

    			public:

                gpu_frame_byteaddress_buffer() = default;
				~gpu_frame_byteaddress_buffer() = default;

                gpu_frame_byteaddress_buffer(ID3D12Resource* resource, descriptor_handle srv, descriptor_handle uav) :
					base(resource, srv, uav)
				{

				}
			};

            inline uint64_t size( const gpu_buffer* r )
            {
                return r->desc().Width;
            }

            inline D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view( const gpu_virtual_resource* r, size_t offset, uint32_t size, uint32_t stride)
            {
                D3D12_VERTEX_BUFFER_VIEW view = {};
                view.BufferLocation = r->virtual_address() + offset;
                view.SizeInBytes = size;
                view.StrideInBytes = stride;
                return view;
            }

            inline D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view(const gpu_buffer* r, uint32_t stride )
            {
                return vertex_buffer_view(r, 0, static_cast<uint32_t>(size(r)), stride);
            }

            inline D3D12_INDEX_BUFFER_VIEW index_buffer_view(const gpu_virtual_resource* r, size_t offset, uint32_t size, bool b32Bit = false)
            {
                D3D12_INDEX_BUFFER_VIEW view;
                view.BufferLocation = r->virtual_address() + offset;
                view.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
                view.SizeInBytes = size;
                return view;
            }

            inline D3D12_INDEX_BUFFER_VIEW index_buffer_view(const gpu_buffer* b, size_t stride ) 
            {
                return index_buffer_view(b, 0, static_cast<uint32_t>(size(b)), stride == 4);
            }
        }
    }
}
