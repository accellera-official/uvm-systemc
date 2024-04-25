//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2014 Université Pierre et Marie Curie, Paris
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

#ifndef UVM_REG_FRONTDOOR_H_
#define UVM_REG_FRONTDOOR_H_

#include "uvmsc/reg/uvm_reg_sequence.h"
#include "uvmsc/seq/uvm_sequence.h"
#include "uvmsc/seq/uvm_sequence_item.h"

namespace uvm {


//----------------------------------------------------------------------------
// Class: uvm_reg_frontdoor
//
//! Facade class for register and memory frontdoor access.
//!
//! User-defined frontdoor access sequence
//!
//! Base class for user-defined access to register and memory reads and writes
//! through a physical interface.
//!
//! By default, different registers and memories are mapped to different
//! addresses in the address space and are accessed via those exclusively
//! through physical addresses.
//!
//! The frontdoor allows access using a non-linear and/or non-mapped mechanism.
//! Users can extend this class to provide the physical access to these registers.
//----------------------------------------------------------------------------

class uvm_reg_frontdoor : public uvm_reg_sequence<uvm_sequence<uvm_sequence_item> >
{
 public:
  friend class uvm_reg;

  //------------------------------------------------------------------------
  // Variable: rw_info
  //
  //! Holds information about the register being read or written
  //------------------------------------------------------------------------
  uvm_reg_item* rw_info;

  //------------------------------------------------------------------------
  // Variable: sequencer
  //
  //! Sequencer executing the operation
  //------------------------------------------------------------------------
  uvm_sequencer_base* sequencer;

  //------------------------------------------------------------------------
  // member function: new
  //
  //! Constructor, new object given optional name \p name.
  //------------------------------------------------------------------------
  explicit uvm_reg_frontdoor( const std::string& name = "" ) : uvm_reg_sequence<uvm_sequence<uvm_sequence_item> >(name)
  {
    rw_info = NULL;
    sequencer = NULL;

    m_fname = "";
    m_lineno = 0;

  }

 private:
  std::string m_fname;
  int m_lineno;

}; // class uvm_reg_frontdoor

} // namespace uvm

#endif // UVM_REG_FRONTDOOR_H_
