#pragma once

#include <memory>

namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            class gpu_resource_create_context;
            class gpu_frame_color_buffer;

            namespace details
            {
                struct gpu_frame_color_buffer_deleter
                {
                    gpu_resource_create_context* m_rc = nullptr;
                    gpu_frame_color_buffer_deleter() noexcept = default;
                    gpu_frame_color_buffer_deleter(gpu_resource_create_context* rc) noexcept : m_rc(rc) {}
                    void operator () (gpu_frame_color_buffer* d);
                };
            }

            using managed_gpu_frame_color_buffer = std::unique_ptr< gpu_frame_color_buffer, details::gpu_frame_color_buffer_deleter >;

            template <typename ...args>
            inline gpu_frame_color_buffer create_frame_color_buffer(gpu_resource_create_context* rc, args&&... a)
            {
                return managed_gpu_frame_color_buffer(rc->create_frame_color_buffer(std::forward<args>(a)...), details::gpu_frame_color_buffer_deleter(rc));
            }
        }
    }

}
