//----------------------------------------------------------------------
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
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

#include <string>
#include <map>

#include "uvmsc/cb/uvm_typeid.h"

namespace uvm {

int uvm_typeid_base::set_cb(uvm_typeid_base* ti, uvm_callbacks_base* cb)
{
  m_access_typeid_map()[ti] = cb;
  return 1;
}

uvm_callbacks_base* uvm_typeid_base::get_cb(uvm_typeid_base* ti)
{
  return m_access_typeid_map()[ti];
}

int uvm_typeid_base::set_typeid(uvm_callbacks_base* cb, uvm_typeid_base* ti)
{
  m_access_type_map()[cb] = ti;
  return 1;
}

uvm_typeid_base* uvm_typeid_base::get_typeid(uvm_callbacks_base* cb)
{
  return m_access_type_map()[cb];
}

} // namespace uvm
