#pragma once

#include <memory>

namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            class gpu_resource_create_context;
            class gpu_frame_byteaddress_buffer;

            namespace details
            {
                struct frame_byteaddress_gpu_buffer_deleter
                {
                    gpu_resource_create_context* m_rc = nullptr;

                    frame_byteaddress_gpu_buffer_deleter() = default;
                    frame_byteaddress_gpu_buffer_deleter(gpu_resource_create_context* rc) : m_rc(rc) {}
                    void operator () (gpu_frame_byteaddress_buffer* d);
                };
            }

            using managed_gpu_frame_byteaddress_buffer  = std::unique_ptr< gpu_frame_byteaddress_buffer, details::frame_byteaddress_gpu_buffer_deleter >;

            template <typename ...args>
            inline managed_gpu_frame_byteaddress_buffer create_frame_byteaddress_buffer(gpu_resource_create_context* rc, args&& ... a)
            {
                return managed_gpu_frame_byteaddress_buffer(rc->create_frame_byteaddress_buffer(std::forward<args>(a)...), details::frame_byteaddress_gpu_buffer_deleter(rc));
            }
        }
    }

}
