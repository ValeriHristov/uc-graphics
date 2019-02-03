#pragma once

#include <exception>
#include <Ole2.h>

namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            using exception     = std::exception;

            class com_exception : public exception
            {
                using base = exception;

                public:

                explicit com_exception(const HRESULT hr) noexcept : base("com exception")
                    , m_hr(hr)
                {

                }

            private:
                HRESULT m_hr;
            };


            class out_of_memory_exception : public exception
            {
                using base = exception;

                public:

                out_of_memory_exception() noexcept : base("out of memory")
                {

                }
                
            };

            inline void throw_if_failed( HRESULT hr )
            {
                if (FAILED(hr))
                {
                    throw com_exception(hr);
                }
            }

            inline void raise_out_of_memory_exception( )
            {
                throw out_of_memory_exception();
            }
        }
    }
}

