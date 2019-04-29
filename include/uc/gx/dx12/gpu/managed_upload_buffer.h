#pragma once

#include <memory>

namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            class gpu_resource_create_context;
            class gpu_upload_buffer;

            namespace details
            {
                struct gpu_upload_buffer_deleter
                {
                    gpu_resource_create_context* m_rc = nullptr;

                    gpu_upload_buffer_deleter() = default;
                    gpu_upload_buffer_deleter(gpu_resource_create_context* rc) : m_rc(rc) {}
                    void operator () (gpu_upload_buffer* d);
                };
            }

            using managed_gpu_upload_buffer = std::unique_ptr< gpu_upload_buffer, details::gpu_upload_buffer_deleter >;

            template <typename ...args>
            inline managed_gpu_upload_buffer create_upload_buffer(gpu_resource_create_context* rc, args&&... a)
            {
                return managed_gpu_upload_buffer(rc->create_upload_buffer(std::forward<args>(a)...), details::gpu_upload_buffer_deleter(rc));
            }
        }
    }

}
