//-----------------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2010 Cadence Design Systems, Inc.
//   Copyright 2011 Mentor Graphics Corporation
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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
//-----------------------------------------------------------------------------

#include "uvmsc/reg/uvm_reg_indirect_ftdr_seq.h"
#include "uvmsc/reg/uvm_reg.h"

namespace uvm {

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

uvm_reg_indirect_ftdr_seq::uvm_reg_indirect_ftdr_seq( uvm_reg* addr_reg,
                                                      int idx,
                                                      uvm_reg* data_reg)
  : uvm_reg_frontdoor("uvm_reg_indirect_ftdr_seq")
{
  m_addr_reg = addr_reg;
  m_idx      = idx;
  m_data_reg = data_reg;
}

void uvm_reg_indirect_ftdr_seq::body()
{
   uvm_handle<uvm_reg_item>  rw;

   rw = adopt_handle(dynamic_cast<uvm_reg_item*>(rw_info->clone()));

   rw->element = m_addr_reg;
   rw->access_kind = UVM_WRITE;
   rw->value[0]= m_idx;

   m_addr_reg->m_atomic_check_lock(true); // TODO need this?
   m_data_reg->m_atomic_check_lock(true);

   m_addr_reg->do_write(rw);

   if (rw->status == UVM_NOT_OK)
     return;

   rw = adopt_handle(dynamic_cast<uvm_reg_item*>(rw_info->clone()));
   rw->element = m_data_reg;

   if (rw_info->access_kind == UVM_WRITE)
     m_data_reg->do_write(rw);
   else
   {
     m_data_reg->do_read(rw);
     rw_info->value[0] = rw->value[0];
   }

   m_addr_reg->m_atomic_check_lock(false); // TODO need this?
   m_data_reg->m_atomic_check_lock(false);

   rw_info->status = rw->status;
}

} // namespace uvm
