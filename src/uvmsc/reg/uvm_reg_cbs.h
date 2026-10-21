//----------------------------------------------------------------------
//   Copyright 2013-2016 NXP B.V.
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

#ifndef UVM_REG_CBS_H_
#define UVM_REG_CBS_H_

#include "uvmsc/base/uvm_handle.h"
#include "uvmsc/cb/uvm_callback.h"
#include "uvmsc/reg/uvm_reg_model.h"

namespace uvm {


// forward class declarations
class uvm_reg_item;
class uvm_reg_field;
class uvm_reg_map;

//------------------------------------------------------------------------------
// Class: uvm_reg_cbs
//
//! Facade class for field, register, memory and backdoor
//! access callback methods.
//------------------------------------------------------------------------------

class uvm_reg_cbs : public uvm_callback
{
 public:

  uvm_reg_cbs( const std::string& name = "uvm_reg_cbs" ) : uvm_callback(name) {}

  virtual void pre_write( uvm_reg_item&  rw) {}

  virtual void post_write( uvm_reg_item&  rw ) {}

  virtual void pre_read( uvm_reg_item&  rw ) {}

  virtual void post_read( uvm_reg_item&  rw ) {}

  virtual void post_predict( uvm_reg_field* fld,
                             uvm_reg_data_t previous,
                             uvm_reg_data_t value,
                             uvm_predict_e  kind,
                             uvm_path_e     path,
                             uvm_reg_map*   map ) {}

  virtual void encode( std::vector<uvm_reg_data_t>& data ) {}

  virtual void decode( std::vector<uvm_reg_data_t>& data ) {}

}; // class uvm_reg_cbs

} // namespace uvm

#endif // UVM_REG_CBS_H_
