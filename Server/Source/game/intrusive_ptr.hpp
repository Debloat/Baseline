#ifndef BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED
#define BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED

//
//  intrusive_ptr.hpp
//
//  Copyright (c) 2001, 2002 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/ for documentation.
//

#include <functional>           // for std::less
#include <iosfwd>               // for std::basic_ostream
#include <cstddef>
#include <cassert>
#include <type_traits>

namespace boost
{

    //
    //  intrusive_ptr
    //
    //  A smart pointer that uses intrusive reference counting.
    //
    //  Relies on unqualified calls to
    //  
    //      void intrusive_ptr_add_ref(T * p);
    //      void intrusive_ptr_release(T * p);
    //
    //          (p != 0)
    //
    //  The object is responsible for destroying itself.
    //

    template<class T> class intrusive_ptr
    {
    private:

        typedef intrusive_ptr this_type;

    public:

        typedef T element_type;

        constexpr intrusive_ptr() noexcept : px(0)
        {
        }

        constexpr intrusive_ptr(T* p, bool add_ref = true) : px(p)
        {
            if (px != 0 && add_ref) intrusive_ptr_add_ref(px);
        }

        template<class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
        constexpr intrusive_ptr(intrusive_ptr<U> const& rhs)
            : px(rhs.get())
        {
            if (px != 0) intrusive_ptr_add_ref(px);
        }

        constexpr intrusive_ptr(intrusive_ptr const& rhs) : px(rhs.px)
        {
            if (px != 0) intrusive_ptr_add_ref(px);
        }

        constexpr ~intrusive_ptr()
        {
            if (px != 0) intrusive_ptr_release(px);
        }

        template<class U> constexpr intrusive_ptr& operator=(intrusive_ptr<U> const& rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }

        // Move support

        constexpr intrusive_ptr(intrusive_ptr&& rhs) noexcept : px(rhs.px)
        {
            rhs.px = 0;
        }

        constexpr intrusive_ptr& operator=(intrusive_ptr&& rhs) noexcept
        {
            this_type(static_cast<intrusive_ptr&&>(rhs)).swap(*this);
            return *this;
        }

        template<class U> friend class intrusive_ptr;

        template<class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
        constexpr intrusive_ptr(intrusive_ptr<U>&& rhs) noexcept
            : px(rhs.px)
        {
            rhs.px = 0;
        }

        template<class U>
        constexpr intrusive_ptr& operator=(intrusive_ptr<U>&& rhs) noexcept
        {
            this_type(static_cast<intrusive_ptr<U>&&>(rhs)).swap(*this);
            return *this;
        }

        constexpr intrusive_ptr& operator=(intrusive_ptr const& rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }

        constexpr intrusive_ptr& operator=(T* rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }

        constexpr void reset()
        {
            this_type().swap(*this);
        }

        constexpr void reset(T* rhs)
        {
            this_type(rhs).swap(*this);
        }

        constexpr void reset(T* rhs, bool add_ref)
        {
            this_type(rhs, add_ref).swap(*this);
        }

        constexpr T* get() const noexcept
        {
            return px;
        }

        constexpr T* detach() noexcept
        {
            T* ret = px;
            px = 0;
            return ret;
        }

        constexpr T& operator*() const noexcept
        {
            assert(px != 0);
            return *px;
        }

        constexpr T* operator->() const noexcept
        {
            assert(px != 0);
            return px;
        }

        constexpr explicit operator bool() const noexcept
        {
            return px != 0;
        }

        constexpr void swap(intrusive_ptr& rhs) noexcept
        {
            T* tmp = px;
            px = rhs.px;
            rhs.px = tmp;
        }

    private:

        T* px;
    };

    template<class T, class U> constexpr inline bool operator==(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b) noexcept
    {
        return a.get() == b.get();
    }

    template<class T, class U> constexpr inline bool operator!=(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b) noexcept
    {
        return a.get() != b.get();
    }

    template<class T, class U> constexpr inline bool operator==(intrusive_ptr<T> const& a, U* b) noexcept
    {
        return a.get() == b;
    }

    template<class T, class U> constexpr inline bool operator!=(intrusive_ptr<T> const& a, U* b) noexcept
    {
        return a.get() != b;
    }

    template<class T, class U> constexpr inline bool operator==(T* a, intrusive_ptr<U> const& b) noexcept
    {
        return a == b.get();
    }

    template<class T, class U> constexpr inline bool operator!=(T* a, intrusive_ptr<U> const& b) noexcept
    {
        return a != b.get();
    }

    template<class T> constexpr inline bool operator==(intrusive_ptr<T> const& p, std::nullptr_t) noexcept
    {
        return p.get() == 0;
    }

    template<class T> constexpr inline bool operator==(std::nullptr_t, intrusive_ptr<T> const& p) noexcept
    {
        return p.get() == 0;
    }

    template<class T> constexpr inline bool operator!=(intrusive_ptr<T> const& p, std::nullptr_t) noexcept
    {
        return p.get() != 0;
    }

    template<class T> constexpr inline bool operator!=(std::nullptr_t, intrusive_ptr<T> const& p) noexcept
    {
        return p.get() != 0;
    }

    template<class T> constexpr inline bool operator<(intrusive_ptr<T> const& a, intrusive_ptr<T> const& b) noexcept
    {
        return std::less<T*>()(a.get(), b.get());
    }

    template<class T> constexpr inline void swap(intrusive_ptr<T>& lhs, intrusive_ptr<T>& rhs) noexcept
    {
        lhs.swap(rhs);
    }

    // mem_fn support

    template<class T> constexpr inline T* get_pointer(intrusive_ptr<T> const& p) noexcept
    {
        return p.get();
    }

    // pointer casts

    template<class T, class U> constexpr inline intrusive_ptr<T> static_pointer_cast(intrusive_ptr<U> const& p)
    {
        return static_cast<T*>(p.get());
    }

    template<class T, class U> constexpr inline intrusive_ptr<T> const_pointer_cast(intrusive_ptr<U> const& p)
    {
        return const_cast<T*>(p.get());
    }

    template<class T, class U> constexpr inline intrusive_ptr<T> dynamic_pointer_cast(intrusive_ptr<U> const& p)
    {
        return dynamic_cast<T*>(p.get());
    }

    template<class T, class U> constexpr inline intrusive_ptr<T> static_pointer_cast(intrusive_ptr<U>&& p) noexcept
    {
        return intrusive_ptr<T>(static_cast<T*>(p.detach()), false);
    }

    template<class T, class U> constexpr inline intrusive_ptr<T> const_pointer_cast(intrusive_ptr<U>&& p) noexcept
    {
        return intrusive_ptr<T>(const_cast<T*>(p.detach()), false);
    }

    template<class T, class U> constexpr inline intrusive_ptr<T> dynamic_pointer_cast(intrusive_ptr<U>&& p) noexcept
    {
        T* p2 = dynamic_cast<T*>(p.get());

        intrusive_ptr<T> r(p2, false);

        if (p2) p.detach();

        return r;
    }

    // operator<<

    template<class E, class T, class Y> std::basic_ostream<E, T>& operator<< (std::basic_ostream<E, T>& os, intrusive_ptr<Y> const& p)
    {
        os << p.get();
        return os;
    }

    // hash_value

    template< class T > struct hash;

    template< class T > std::size_t hash_value(boost::intrusive_ptr<T> const& p) noexcept
    {
        return boost::hash< T* >()(p.get());
    }

} // namespace boost

// std::hash

namespace std
{

    template<class T> struct hash< ::boost::intrusive_ptr<T> >
    {
        std::size_t operator()(::boost::intrusive_ptr<T> const& p) const noexcept
        {
            return std::hash< T* >()(p.get());
        }
    };

} // namespace std

#endif  // #ifndef BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED
