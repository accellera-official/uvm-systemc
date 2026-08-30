//----------------------------------------------------------------------
//   Copyright 2012-2015 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
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

#ifndef UVM_AGENT_H_
#define UVM_AGENT_H_

#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_object_globals.h"

namespace uvm {

//----------------------------------------------------------------------
// CLASS: uvm_agent
//
//! Base class for user-defined agents.
//! Derived from #uvm_component.
//----------------------------------------------------------------------

class uvm_agent : public uvm_component
{
 public:
  uvm_active_passive_enum is_active;

  explicit uvm_agent( uvm_component_name name_ );
  	
  const std::string get_type_name() const override;

  void build_phase(uvm_phase& phase) override;

  virtual uvm_active_passive_enum get_is_active() const;

  const char* kind() const override;  // SystemC API

};

/////////////////////////////////////////////

} /* namespace uvm */

#endif /* UVM_AGENT_H_ */
