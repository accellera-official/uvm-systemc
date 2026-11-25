//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010 Mentor Graphics Corporation
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
//----------------------------------------------------------------------

#include "uvmsc/reg/uvm_reg_tlm_adapter.h"

namespace uvm {

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

uvm_reg_tlm_adapter::uvm_reg_tlm_adapter( const std::string& name )
  : uvm_reg_adapter(name)
{}

//----------------------------------------------------------------------
// member function: reg2bus
//
//! Converts a #uvm_reg_bus_op struct to a #uvm_tlm_gp item.
//----------------------------------------------------------------------

uvm_handle<uvm_sequence_item> uvm_reg_tlm_adapter::reg2bus( const uvm_reg_bus_op& rw )
{
  /* TODO reg2bus
  uvm_tlm_gp gp = uvm_tlm_gp::type_id::create("tlm_gp",, this.get_full_name());
  int nbytes = (rw.n_bits-1)/8+1;
  uvm_reg_addr_t addr=rw.addr;

  if (rw.access_kind == UVM_WRITE)
    gp.set_command(UVM_TLM_WRITE_COMMAND);
  else
    gp.set_command(UVM_TLM_READ_COMMAND);

  gp.set_address(addr);

  gp.m_byte_enable = new [nbytes];

  gp.set_streaming_width (nbytes);

  gp.m_data = new [gp.get_streaming_width()];

  for (int i = 0; i < nbytes; i++) begin
      gp.m_data[i] = rw.data[i*8+:8];
  gp.m_byte_enable[i] = (i > nbytes) ? 1'b0 : rw.byte_en[i];
  end

  return gp;
  */
  return nullptr;
}

//----------------------------------------------------------------------
// member function: bus2reg
//
//! Converts a #uvm_tlm_gp item to a #uvm_reg_bus_op.
//! into the provided \p rw transaction.
//----------------------------------------------------------------------

void uvm_reg_tlm_adapter::bus2reg( const uvm_sequence_item* bus_item,
                        uvm_reg_bus_op& rw )

{
  /* TODO bus2reg
  uvm_tlm_gp* gp;
  int nbytes;

  if (bus_item == nullptr)
    UVM_FATAL("REG/NULL_ITEM","bus2reg: bus_item argument is nullptr");

  gp = dynamic_cast<uvm_tlm_gp*>(bus_item);
  if (gp == nullptr)
  {
    UVM_ERROR("WRONG_TYPE", "Provided bus_item is not of type uvm_tlm_gp");
    return;
  }

  if (gp->get_command() == UVM_TLM_WRITE_COMMAND)
    rw.access_kind = UVM_WRITE;
  else
    rw.access_kind = UVM_READ;

  rw.addr = gp.get_address();

  rw.byte_en = 0;
  foreach (gp.m_byte_enable[i])
  rw.byte_en[i] = gp.m_byte_enable[i];

  rw.data = 0;
  foreach (gp.m_data[i])
  rw.data[i*8+:8] = gp.m_data[i];

  rw.status = (gp.is_response_ok()) ? UVM_IS_OK : UVM_NOT_OK;
  */
}

} // namespace uvm
