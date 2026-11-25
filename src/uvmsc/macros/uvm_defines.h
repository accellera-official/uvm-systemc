//----------------------------------------------------------------------
//   Copyright 2013-2019 NXP B.V.
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

#ifndef UVM_DEFINES_H_
#define UVM_DEFINES_H_

#include <typeinfo>
#include <type_traits>

#define UVM_TYPENAME(T) typeid(T).name()

#define uvm_typeof(x) std::remove_reference<decltype((x))>::type

#define UVM_DEPRECATED_MSG(msg) [[deprecated(msg)]]

#if defined(ALLOW_UVM_SC_DEPRECATED_SILENT)
#define UVM_DEPRECATED_DECL(MSG, ...) __VA_ARGS__
#define UVM_DEPRECATED_API_ENABLED 1
#elif defined(ALLOW_UVM_SC_DEPRECATED)
#define UVM_DEPRECATED_DECL(MSG, ...) UVM_DEPRECATED_MSG(MSG) __VA_ARGS__
#define UVM_DEPRECATED_API_ENABLED 1
#else
#define UVM_DEPRECATED_DECL(MSG, ...)
#define UVM_DEPRECATED_API_ENABLED 0
#endif

// Version tags are for maintainers to track deprecation/removal waves.
#define UVM_DEPRECATED_1_0(MSG, ...) UVM_DEPRECATED_DECL(MSG, __VA_ARGS__)
#define UVM_DEPRECATED_1_0_ENABLED UVM_DEPRECATED_API_ENABLED

#endif // UVM_DEFINES_H_
