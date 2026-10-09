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

#include <systemc>

#include "uvmsc/reg/uvm_reg_adapter.h"
#include "uvmsc/reg/uvm_reg_item.h"

namespace uvm {

//----------------------------------------------------------------------
// Constructor
//
//! Create a new instance of this type, giving it the optional \p name.
//----------------------------------------------------------------------

uvm_reg_adapter::uvm_reg_adapter( const std::string& name ) : uvm_object(name)
{
  provides_responses = false;
  supports_byte_enable = false;
  parent_sequence = nullptr;
  m_item = nullptr;
}

//----------------------------------------------------------------------
// member function: get_item
//
//! Returns the bus-independent read/write information that corresponds to
//! the generic bus transaction currently translated to a bus-specific
//! transaction.
//! This function returns a value reference only when called in the
//! uvm_reg_adapter::reg2bus() method.
//! It returns nullptr at all other times.
//! The content of the return #uvm_reg_item instance must not be modified
//! and used strictly to obtain additional information about the operation.
//----------------------------------------------------------------------

uvm_handle<uvm_reg_item>  uvm_reg_adapter::get_item() const
{
  return m_item;
}

//----------------------------------------------------------------------
// member function: m_set_item
//
//! Implementation defined
//----------------------------------------------------------------------

void uvm_reg_adapter::m_set_item( uvm_handle<uvm_reg_item>  item )
{
  m_item = item;
}

///////////


} // namespace uvm
