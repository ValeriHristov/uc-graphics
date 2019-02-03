#include "pch.h"

#include <uc/gx/geo/object_allocator.h>
#include <boost/pool/object_pool.hpp>

namespace uc
{
    namespace gx
    {
        namespace geo
        {
            class object_allocator::impl
            {
                public:

                impl(count max_object_count);
                ~impl();

                handle allocate(count object_count);
                handle allocate(count object_count, std::nothrow_t) noexcept;
                void   free(handle);

                template<class ElementType>          using object_pool = boost::object_pool<ElementType>;
                object_pool<object_allocation>       m_memory;
                object_allocation*                   m_allocations;
            };

            object_allocator::impl::impl(count max_object_count)
            {
                m_allocations = m_memory.construct(0, max_object_count, true);
            }

            object_allocator::impl::~impl()
            {
            }

            object_allocator::handle object_allocator::impl::allocate(count object_count)
            {
                handle h = allocate(object_count, std::nothrow_t());

                if (!h)
                {
                    throw std::bad_alloc();
                }

                return h;
            }

            object_allocator::handle object_allocator::impl::allocate(count object_count, std::nothrow_t) noexcept
            {
                object_allocation* d = m_allocations;

                while (d)
                {
                    if (d->is_free())
                    {
                        if (d->count() > object_count)
                        {
                            //allocate at the end
                            object_allocation* n = m_memory.construct(d->offset() + d->m_count - object_count, object_count, false);
#ifdef _DEBUG
                            int64_t test = d->offset() + d->m_count - object_count;
                            assert(test > -1LL);
#endif
                            if (n)
                            {
                                d->m_count -= object_count;
                                d->m_next = n;
                                n->m_previous = d;

                                return n;
                            }
                            else
                            {
                                return nullptr;
                            }
                        }
                        else if (d->count() == object_count)
                        {
                            d->set_free(false);
                            return d;
                        }
                    }
                    d = d->m_previous;
                }

                return d;
            }

            void   object_allocator::impl::free(object_allocator::handle h)
            {
                if (h)
                {
                    h->set_free(true);

                    object_allocation* d = h;

                    //coalesce free memory
                    while (d && d->m_next != nullptr && d->m_next->is_free())
                    {
                        object_allocation* n = d->m_next;

                        //d->m_offset -= offset(n->m_count);
                        d->m_count += n->m_count;
                        d->m_next = n->m_next;

                        if (d->m_next)
                        {
                            d->m_next->m_previous = d;
                        }

                        m_memory.destroy(n);

                        d = d->m_next;
                    }

                    //coalesce free memory
                    while (d && d->m_previous != nullptr && d->m_previous->is_free())
                    {
                        object_allocation* n = d->m_previous;

                        //n->m_offset -= offset(n->m_count);
                        n->m_count += d->m_count.value();
                        n->m_next = d->m_next;

                        if (n->m_next)
                        {
                            n->m_next->m_previous = n;
                        }

                        m_memory.destroy(d);

                        d = n;
                    }
                }
            }

            //////////////////////////
            object_allocator::object_allocator(count max_object_count) : m_impl(max_object_count)
            {
            }

            object_allocator::~object_allocator()
            {

            }

            object_allocator::handle object_allocator::allocate(count object_count)
            {
                return m_impl->allocate(object_count);
            }

            object_allocator::handle object_allocator::allocate(count object_count, std::nothrow_t n) noexcept
            {
                return m_impl->allocate(object_count, n);
            }

            void   object_allocator::free(object_allocator::handle h)
            {
                return m_impl->free(h);
            }
        }
    }
}

