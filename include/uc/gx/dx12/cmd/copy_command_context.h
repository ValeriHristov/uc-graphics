#pragma once

#include <uc/gx/dx12/gpu/pipeline_state.h>
#include "command_context.h"

namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            struct gpu_copy_command_context : public gpu_command_context
            {
                using base = gpu_command_context;
            };
        }
    }
}