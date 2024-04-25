//----------------------------------------------------------------------
//   Copyright 2013-2016 NXP B.V.
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

#ifndef UVM_REG_ITEM_H_
#define UVM_REG_ITEM_H_

#include <systemc>
#include <list>

#include "uvmsc/macros/uvm_object_defines.h"
#include "uvmsc/seq/uvm_sequence_item.h"
#include "uvmsc/factory/uvm_object_registry.h"
#include "uvmsc/reg/uvm_reg_model.h"

namespace uvm {

// forward class declaration
class uvm_reg_map;

//------------------------------------------------------------------------------
// CLASS: uvm_reg_item
//
//! Defines an abstract register transaction item. No bus-specific information
//! is present, although a handle to a #uvm_reg_map is provided in case a user
//! wishes to implement a custom address translation algorithm.
//------------------------------------------------------------------------------

class uvm_reg_item : public uvm_sequence_item
{
  using uvm_sequence_item::uvm_report;
  using uvm_sequence_item::uvm_report_info;
  using uvm_sequence_item::uvm_report_warning;
  using uvm_sequence_item::uvm_report_error;
  using uvm_sequence_item::uvm_report_fatal;

 public:

  explicit uvm_reg_item( const std::string& name = "" );

  virtual std::string convert2string() const;

  virtual void do_copy( const uvm_object& rhs );

  UVM_OBJECT_UTILS(uvm_reg_item)

  // data members:

  uvm_elem_kind_e element_kind;

  uvm_object* element;

  // note: we cannot use the kind variable name, as this is used in SystemC to report the type name of the object
  /*rand*/ uvm_access_e access_kind;      // TODO support random

  /*rand*/ std::vector<uvm_reg_data_t> value; // TODO support random

  /*rand*/ uvm_reg_addr_t offset;

  uvm_status_e status;

  // TODO: constraints
  //constraint max_values { value.size() > 0 && value.size() < 1000; }

  uvm_reg_map* local_map;

  uvm_reg_map* map;

  uvm_path_e path;

  /*rand*/ uvm_sequence_base* parent;  // TODO support random

  int prior; // default -1, done in constructor

  /*rand*/ uvm_object* extension;  // TODO support random

  std::string bd_kind;

  std::string fname;

  int lineno;

}; // class uvm_reg_item

} // namespace uvm

#endif // UVM_REG_ITEM_H_
