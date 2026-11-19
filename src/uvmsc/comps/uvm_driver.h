//----------------------------------------------------------------------
//   Copyright 2012-2015 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
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

#ifndef UVM_DRIVER_H_
#define UVM_DRIVER_H_

#include <string>

#include <systemc>

#include "uvmsc/base/uvm_component_name.h"
#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_port_base.h"
#include "uvmsc/seq/uvm_sequence_item.h"
#include "uvmsc/seq/uvm_sequencer_ifs.h"
#include "uvmsc/tlm1/uvm_analysis_port.h"
#include "uvmsc/tlm1/uvm_sqr_connections.h"

namespace uvm {

//------------------------------------------------------------------------------
// CLASS: uvm_driver
//
//! The base class for drivers that initiate requests for new transactions via
//! a #uvm_seq_item_pull_port. The ports are typically connected to the exports of
//! an appropriate sequencer component.
//!
//! Derived from #uvm_component.
//------------------------------------------------------------------------------

template <typename REQ = uvm_sequence_item, typename RSP = REQ>
class uvm_driver : public uvm_component
{
 public:
  // default available TLM port to sequencer
  uvm_seq_item_pull_port<REQ, RSP> seq_item_port;
  uvm_analysis_port<RSP> rsp_port;

  explicit uvm_driver( uvm_component_name name_ )
  : uvm_component( name_ ), seq_item_port("seq_item_port"), rsp_port("rsp_port") {}
  	
  const std::string get_type_name() const override;

  const char* kind() const override; // SystemC API

};

/////////////////////////////////////////////
/////////////////////////////////////////////

//----------------------------------------------------------------------
// member function: get_type_name()
//
//! Return the type name of the object
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
inline const std::string uvm_driver<REQ, RSP>::get_type_name() const
{
  return std::string(this->kind());
}

//----------------------------------------------------------------------
// member function: kind()
//
//! SystemC compatible API
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
inline const char* uvm_driver<REQ, RSP>::kind() const
{
  return "uvm::uvm_driver";
}


} /* namespace uvm */

#endif /* UVM_DRIVER_H_ */
