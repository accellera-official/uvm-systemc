//----------------------------------------------------------------------
//   Copyright 2012-2022 NXP B.V.
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

#ifndef UVM_VERSION_H_
#define UVM_VERSION_H_

#include <string>

namespace uvm {

// TODO file used for MSVC. Find a way to pass version and revision info dynamically

/* UVM SystemC Release Date */
#ifndef UVM_RELEASE_DATE
#define UVM_RELEASE_DATE "2023-03-15"
#endif

/* UVM SystemC Version */
#ifndef UVM_VERSION
#define UVM_VERSION "1.0-beta5"
#endif

// compatible with UVM 1.1.d
#ifndef UVM_VERSION_STRING
#define UVM_VERSION_STRING "1.0-beta5"
#endif

extern const std::string uvm_revision;

std::string uvm_revision_string();

} // namespace uvm

#endif /* UVM_VERSION_H_ */
