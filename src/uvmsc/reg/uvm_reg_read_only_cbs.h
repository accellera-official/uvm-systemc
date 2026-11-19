//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2010 Cadence Design Systems, Inc.
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

#ifndef UVM_REG_READ_ONLY_CBS_H_
#define UVM_REG_READ_ONLY_CBS_H_

#include "uvmsc/reg/uvm_reg_cbs.h"
#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/macros/uvm_object_defines.h"
#include "uvmsc/factory/uvm_object_registry.h"

namespace uvm {

// forward class declarations
class uvm_reg;

//------------------------------------------------------------------------------
// Class: uvm_reg_read_only_cbs
//
//! Pre-defined register callback method for read-only registers
//! that will issue an error if a write() operation is attempted.
//------------------------------------------------------------------------------

class uvm_reg_read_only_cbs : public uvm_reg_cbs
{
 public:

  uvm_reg_read_only_cbs( const std::string& name = "uvm_reg_read_only_cbs" );

  UVM_OBJECT_UTILS(uvm_reg_read_only_cbs)
   
  void pre_write( uvm_reg_item* rw ) override;

  static void add( uvm_reg* rg );

  static void remove( uvm_reg* rg );

 private:

  static uvm_reg_read_only_cbs* get();

  // local data members

  static uvm_reg_read_only_cbs* m_me;

}; // class uvm_reg_read_only_cbs

} // namespace uvm

#endif // UVM_REG_READ_ONLY_CBS_H_
