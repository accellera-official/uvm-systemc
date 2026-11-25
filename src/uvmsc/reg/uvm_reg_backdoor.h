//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
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

#ifndef UVM_REG_BACKDOOR_H_
#define UVM_REG_BACKDOOR_H_

#include <systemc>
#include <map>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/macros/uvm_object_defines.h"
#include "uvmsc/factory/uvm_object_registry.h"
#include "uvmsc/reg/uvm_reg_cbs_types.h"

namespace uvm {


// forward class declarations
class uvm_reg_cbs;
class uvm_reg_item;
class uvm_reg_field;

//----------------------------------------------------------------------
// Class: uvm_reg_backdoor
//
//! Base class for user-defined back-door register and memory access.
//!
//! This class can be extended by users to provide user-specific back-door access
//! to registers and memories that are not implemented in pure SystemVerilog
//! or that are not accessible using the default DPI backdoor mechanism.
//----------------------------------------------------------------------

class uvm_reg_backdoor : public uvm_object
{
 public:
  friend class uvm_reg_block;
  friend class uvm_reg;

  UVM_OBJECT_UTILS(uvm_reg_backdoor);

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  explicit uvm_reg_backdoor( const std::string& name = "" );

 protected:
  void do_pre_read( uvm_handle<uvm_reg_item>  rw );

  void do_post_read( uvm_handle<uvm_reg_item>  rw );

  void do_pre_write( uvm_handle<uvm_reg_item>  rw );

  void do_post_write( uvm_handle<uvm_reg_item>  rw );

 public:
  virtual void write( uvm_handle<uvm_reg_item>  rw );

  virtual void read( uvm_handle<uvm_reg_item>  rw );

  virtual void read_func( uvm_handle<uvm_reg_item> rw );

  virtual bool is_auto_updated( uvm_reg_field* field );

  virtual void wait_for_change( uvm_object* element );

  virtual void pre_read( uvm_reg_item&  rw );

  virtual void post_read( uvm_reg_item&  rw );

  virtual void pre_write( uvm_reg_item&  rw );

  virtual void post_write( uvm_reg_item&  rw );

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

 private:
  UVM_REGISTER_CB(uvm_reg_backdoor, uvm_reg_cbs)

  void start_update_thread( uvm_object* element );

  void start_update_thread_core( uvm_object* element, uvm_reg* rg );

  void kill_update_thread( uvm_object* element );

  bool has_update_threads();

  std::string m_fname;
  int m_lineno;

  std::map<uvm_object*, sc_core::sc_process_handle> m_update_thread;

}; // class uvm_reg_backdoor

} // namespace uvm

#endif // UVM_REG_BACKDOOR_H_
