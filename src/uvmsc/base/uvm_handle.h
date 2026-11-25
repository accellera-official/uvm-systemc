//----------------------------------------------------------------------
//   Copyright 2026 COSEDA Technologies GmbH
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License"); you may not use this file except in
//   compliance with the License.  You may obtain a copy of
//   the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in
//   writing, software distributed under the License is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//   CONDITIONS OF ANY KIND, either express or implied.  See
//   the License for the specific language governing
//   permissions and limitations under the License.
//----------------------------------------------------------------------

#ifndef UVM_HANDLE_H_
#define UVM_HANDLE_H_

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace uvm
{

template <typename T>
class uvm_handle
{
public:
    using element_type = T;

    // Construction and assignment
    uvm_handle() noexcept = default;
    uvm_handle(std::nullptr_t) noexcept {}
    uvm_handle(const uvm_handle&) noexcept = default;
    uvm_handle(uvm_handle&&) noexcept = default;

    template <typename U,
              typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    uvm_handle(const uvm_handle<U>& other) noexcept : ptr_(other.ptr_) {}

    ~uvm_handle() = default;

    uvm_handle& operator=(const uvm_handle&) noexcept = default;
    uvm_handle& operator=(uvm_handle&&) noexcept = default;

    // Object access
    T* get() const noexcept { return ptr_.get(); }
    T& operator*() const noexcept { return *ptr_; }
    T* operator->() const noexcept { return ptr_.get(); }
    explicit operator bool() const noexcept { return static_cast<bool>(ptr_); }

private:
    explicit uvm_handle(std::shared_ptr<T> ptr) noexcept : ptr_(std::move(ptr)) {}

    template <typename U>
    friend class uvm_handle;

    template <typename U, typename... Args>
    friend uvm_handle<U> uvm_make_handle(Args&&... args);

    template <typename U>
    friend uvm_handle<U> m_adopt_handle(U* ptr);

    template <typename To, typename From>
    friend uvm_handle<To> m_dynamic_handle_cast(const uvm_handle<From>& ptr) noexcept;

    template <typename To, typename From>
    friend uvm_handle<To> m_static_handle_cast(const uvm_handle<From>& ptr) noexcept;

    std::shared_ptr<T> ptr_;
};

template <typename T, typename... Args>
uvm_handle<T> uvm_make_handle(Args&&... args)
{
    return uvm_handle<T>(std::make_shared<T>(std::forward<Args>(args)...));
}

// Implementation-defined helpers with access to handle ownership.
template <typename T>
uvm_handle<T> m_adopt_handle(T* ptr)
{
    return ptr ? uvm_handle<T>(std::shared_ptr<T>(ptr)) : uvm_handle<T>();
}

template <typename To, typename From>
uvm_handle<To> m_dynamic_handle_cast(const uvm_handle<From>& ptr) noexcept
{
    return uvm_handle<To>(std::dynamic_pointer_cast<To>(ptr.ptr_));
}

template <typename To, typename From>
uvm_handle<To> m_static_handle_cast(const uvm_handle<From>& ptr) noexcept
{
    return uvm_handle<To>(std::static_pointer_cast<To>(ptr.ptr_));
}

// Public handle helpers.

//----------------------------------------------------------------------
// function: uvm_adopt_handle
//
//! Returns an owning handle to the object pointed to by \p ptr. The object
//! is deleted when its last owning handle is released. If \p ptr is null,
//! an empty handle is returned.
//!
//! The object shall have been allocated with new and shall not already be
//! owned by another handle or smart pointer. After this call, the caller
//! shall not delete the object directly. Use this function to transfer
//! ownership of an existing object to a handle.
//----------------------------------------------------------------------

template <typename T>
uvm_handle<T> uvm_adopt_handle(T* ptr)
{
    return ::uvm::m_adopt_handle(ptr);
}

//----------------------------------------------------------------------
// function: uvm_dynamic_handle_cast
//
//! Returns a handle to the object in \p ptr cast to type To using
//! dynamic_cast. The returned handle shares ownership with \p ptr; the
//! object is not copied and \p ptr is not modified.
//!
//! If \p ptr is empty or the cast fails, an empty handle is returned. Use
//! this function when the object's dynamic type must be checked before
//! accessing members of a derived class.
//----------------------------------------------------------------------

template <typename To, typename From>
uvm_handle<To> uvm_dynamic_handle_cast(const uvm_handle<From>& ptr) noexcept
{
    return ::uvm::m_dynamic_handle_cast<To>(ptr);
}

//----------------------------------------------------------------------
// function: uvm_static_handle_cast
//
//! Returns a handle to the object in \p ptr cast to type To using
//! static_cast. The returned handle shares ownership with \p ptr; the
//! object is not copied and \p ptr is not modified. If \p ptr is empty,
//! an empty handle is returned.
//!
//! No runtime type check is performed. The caller shall ensure that the
//! cast is valid for the object in \p ptr. Use this function for a known
//! type conversion, such as converting a derived handle to a base handle.
//----------------------------------------------------------------------

template <typename To, typename From>
uvm_handle<To> uvm_static_handle_cast(const uvm_handle<From>& ptr) noexcept
{
    return ::uvm::m_static_handle_cast<To>(ptr);
}

// Comparisons use pointer identity, not object contents.
template <typename T, typename U>
bool operator==(const uvm_handle<T>& lhs, const uvm_handle<U>& rhs) noexcept
{
    return lhs.get() == rhs.get();
}

template <typename T, typename U>
bool operator!=(const uvm_handle<T>& lhs, const uvm_handle<U>& rhs) noexcept
{
    return !(lhs == rhs);
}

template <typename T>
bool operator==(const uvm_handle<T>& lhs, std::nullptr_t) noexcept
{
    return !lhs;
}

template <typename T>
bool operator==(std::nullptr_t, const uvm_handle<T>& rhs) noexcept
{
    return !rhs;
}

template <typename T>
bool operator!=(const uvm_handle<T>& lhs, std::nullptr_t) noexcept
{
    return static_cast<bool>(lhs);
}

template <typename T>
bool operator!=(std::nullptr_t, const uvm_handle<T>& rhs) noexcept
{
    return static_cast<bool>(rhs);
}

} // namespace uvm

#endif // UVM_HANDLE_H_
