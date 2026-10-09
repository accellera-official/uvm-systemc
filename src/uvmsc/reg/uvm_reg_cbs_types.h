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

#ifndef UVM_REG_CBS_TYPES_H_
#define UVM_REG_CBS_TYPES_H_

#include <systemc>

#include "uvmsc/base/uvm_handle.h"
#include "uvmsc/cb/uvm_callbacks.h"
#include "uvmsc/cb/uvm_callback_iter.h"

namespace uvm {

//forward class declarations
class uvm_reg_field;
class uvm_vreg_field;
class uvm_reg;
class uvm_reg_cbs;
class uvm_vreg_cbs;
class uvm_reg_file;
class uvm_vreg;
class uvm_reg_block;
class uvm_mem;
class uvm_reg_item;
class uvm_reg_map;
class uvm_reg_map_info;
class uvm_reg_backdoor;
class uvm_vreg_field_cbs;

//class uvm_reg_sequence;
//class uvm_reg_adapter;
//class uvm_reg_indirect_data;

//----------------------------------------------------------------------
// Callback typedefs
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Type: uvm_reg_cb
//
//! Convenience callback type declaration for registers
//!
//! Use this declaration to register register callbacks rather than
//! the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callbacks<uvm_reg, uvm_reg_cbs> uvm_reg_cb;


//----------------------------------------------------------------------
// Type: uvm_reg_cb_iter
//
//! Convenience callback iterator type declaration for registers
//!
//! Use this declaration to iterate over registered register callbacks
//! rather than the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callback_iter<uvm_reg, uvm_reg_cbs> uvm_reg_cb_iter;


//----------------------------------------------------------------------
// Type: uvm_reg_bd_cb
//
//! Convenience callback type declaration for backdoor
//!
//! Use this declaration to register register backdoor callbacks rather than
//! the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callbacks<uvm_reg_backdoor, uvm_reg_cbs> uvm_reg_bd_cb;


//----------------------------------------------------------------------
// Type: uvm_reg_bd_cb_iter
//
//! Convenience callback iterator type declaration for backdoor
//!
//! Use this declaration to iterate over registered register backdoor callbacks
//! rather than the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callback_iter<uvm_reg_backdoor, uvm_reg_cbs> uvm_reg_bd_cb_iter;


//----------------------------------------------------------------------
// Type: uvm_mem_cb
//
//! Convenience callback type declaration for memories
//!
//! Use this declaration to register memory callbacks rather than
//! the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callbacks<uvm_mem, uvm_reg_cbs> uvm_mem_cb;


//----------------------------------------------------------------------
// Type: uvm_mem_cb_iter
//
//! Convenience callback iterator type declaration for memories
//!
//! Use this declaration to iterate over registered memory callbacks
//! rather than the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callback_iter<uvm_mem, uvm_reg_cbs> uvm_mem_cb_iter;


//----------------------------------------------------------------------
// Type: uvm_reg_field_cb
//
//! Convenience callback type declaration for fields
//!
//! Use this declaration to register field callbacks rather than
//! the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callbacks<uvm_reg_field, uvm_reg_cbs> uvm_reg_field_cb;


//----------------------------------------------------------------------
// Type: uvm_reg_field_cb_iter
//
//! Convenience callback iterator type declaration for fields
//!
//! Use this declaration to iterate over registered field callbacks
//! rather than the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callback_iter<uvm_reg_field, uvm_reg_cbs> uvm_reg_field_cb_iter;

//----------------------------------------------------------------------
// Type: uvm_vreg_cb
//
//! Convenience callback type declaration
//!
//! Use this declaration to register virtual register callbacks rather than
//! the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callbacks<uvm_vreg, uvm_vreg_cbs> uvm_vreg_cb;

//----------------------------------------------------------------------
// Type: uvm_vreg_cb_iter
//
//! Convenience callback iterator type declaration
//!
//! Use this declaration to iterate over registered virtual register callbacks
//! rather than the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callback_iter<uvm_vreg, uvm_vreg_cbs> uvm_vreg_cb_iter;

//----------------------------------------------------------------------
// Type: uvm_vreg_field_cb
//
//! Convenience callback type declaration
//!
//! Use this declaration to register virtual field callbacks rather than
//! the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callbacks<uvm_vreg_field, uvm_vreg_field_cbs> uvm_vreg_field_cb;

//----------------------------------------------------------------------
// Type: uvm_vreg_field_cb_iter
//
//! Convenience callback iterator type declaration
//!
//! Use this declaration to iterate over registered virtual field callbacks
//! rather than the more verbose parameterized class
//----------------------------------------------------------------------

typedef uvm_callback_iter<uvm_vreg_field, uvm_vreg_field_cbs> uvm_vreg_field_cb_iter;



} // namespace uvm

#endif // UVM_REG_CBS_TYPES_H_
