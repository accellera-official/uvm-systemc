//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
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

#ifndef UVM_BOTTOMUP_PHASE_H_
#define UVM_BOTTOMUP_PHASE_H_

#include "uvmsc/phasing/uvm_phase.h"
#include "uvmsc/phasing/uvm_domain.h"

//////////////

namespace uvm {

//----------------------------------------------------------------------------
// Class: uvm_bottomup_phase
//
//! Base class for function phases that operate bottom-up.
//! The pure virtual function execute() is called for each component.
//! This is the default traversal so is included only for naming.
//!
//! A bottom-up function phase completes when the <execute()> method
//! has been called and returned on all applicable components
//! in the hierarchy.
//----------------------------------------------------------------------------

class uvm_bottomup_phase : public uvm_phase
{
 public:

  explicit uvm_bottomup_phase( const std::string& name );

  void traverse( uvm_component* comp,
                         uvm_phase* phase,
                         uvm_phase_state state ) override;

  void execute( uvm_component* comp,
                        uvm_phase* phase ) override;

};

} // namespace uvm

#endif /* UVM_BOTTOMUP_PHASE_H_ */
