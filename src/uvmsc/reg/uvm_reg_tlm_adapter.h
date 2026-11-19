//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010 Mentor Graphics Corporation
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

#ifndef UVM_REG_TLM_ADAPTER_H_
#define UVM_REG_TLM_ADAPTER_H_

#include <systemc>

#include "uvmsc/reg/uvm_reg_adapter.h"
#include "uvmsc/macros/uvm_object_defines.h"
#include "uvmsc/factory/uvm_object_registry.h"

namespace uvm {


//----------------------------------------------------------------------
// Class: uvm_reg_tlm_adapter
//
//! For converting between #uvm_reg_bus_op and #uvm_tlm_gp items.
//----------------------------------------------------------------------

class uvm_reg_tlm_adapter : public uvm_reg_adapter
{
 public:
  UVM_OBJECT_UTILS(uvm_reg_tlm_adapter)

  uvm_reg_tlm_adapter( const std::string& name = "uvm_reg_tlm_adapter" );

  uvm_sequence_item* reg2bus( const uvm_reg_bus_op& rw ) override;

  void bus2reg( const uvm_sequence_item* bus_item,
                        uvm_reg_bus_op& rw ) override;

}; // class uvm_reg_tlm_adapter

//////////

} // namespace uvm

#endif // UVM_REG_TLM_ADAPTER_H_
