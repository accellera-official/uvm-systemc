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

#ifndef UVM_REG_ADAPTER_H_
#define UVM_REG_ADAPTER_H_

#include <systemc>

#include "uvmsc/base/uvm_handle.h"
#include "uvmsc/base/uvm_object.h"
#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/reg/uvm_reg_bus_op.h"

namespace uvm {


// forward class declarations
class uvm_reg_item;
class uvm_sequence_item;
class uvm_sequence_base;

//----------------------------------------------------------------------
// Class: uvm_reg_adapter
//
// This class defines an interface for converting between <uvm_reg_bus_op>
// and a specific bus transaction. 
//----------------------------------------------------------------------

class uvm_reg_adapter : public uvm_object
{
 public:
  friend class uvm_reg_map;

  //--------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------

  explicit uvm_reg_adapter( const std::string& name = "" );

  //--------------------------------------------------------------------
  // member function: reg2bus (pure virtual)
  //
  // Extensions of this class ~must~ implement this method to convert the specified
  // <uvm_reg_bus_op> to a corresponding <uvm_sequence_item> subtype that defines the bus
  // transaction.
  //
  // The method must allocate a new bus-specific <uvm_sequence_item>,
  // assign its members from
  // the corresponding members from the given generic ~rw~ bus operation, then
  // return it.
  //--------------------------------------------------------------------

  virtual uvm_handle<uvm_sequence_item> reg2bus( const uvm_reg_bus_op& rw ) = 0;

  //--------------------------------------------------------------------
  // member function: bus2reg (pure virtual)
  //
  // Extensions of this class ~must~ implement this method to copy members
  // of the given bus-specific ~bus_item~ to corresponding members of the provided
  // ~bus_rw~ instance. Unlike <reg2bus>, the resulting transaction
  // is not allocated from scratch. This is to accommodate applications
  // where the bus response must be returned in the original request.
  //--------------------------------------------------------------------

  virtual void bus2reg( const uvm_sequence_item* bus_item,
                        uvm_reg_bus_op& rw ) = 0;

  virtual uvm_handle<uvm_reg_item>  get_item() const;

  //--------------------------------------------------------------------
  // UVM LRM data members
  //--------------------------------------------------------------------

  bool supports_byte_enable;

  bool provides_responses;

  uvm_sequence_base* parent_sequence;

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

 private:

  virtual void m_set_item( uvm_handle<uvm_reg_item>  item );

  // other private data members

  uvm_handle<uvm_reg_item>  m_item;

};

///////////


} // namespace uvm

#endif // UVM_REG_ADAPTER_H_
