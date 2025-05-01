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

#ifndef UVM_PROCESS_PHASE_H_
#define UVM_PROCESS_PHASE_H_

#include "uvmsc/phasing/uvm_phase.h"
#include "uvmsc/phasing/uvm_domain.h"

//////////////

namespace uvm {


//------------------------------------------------------------------------------
// Class: uvm_process_phase
//
//! Base class for all process-based phases.
//! It forks a call to uvm_phase::exec_process()
//! for each component in the hierarchy.
//!
//! The completion of the process does not imply, nor is it required for,
//! the end of phase. Once the phase completes, any remaining forked
//! uvm_phase::exec_process() threads are forcibly and immediately killed.
//!
//! By default, the way for a process phase to extend over time is if there is
//! at least one component that raises an objection.
//!
//! There is however one scenario wherein time advances within a process-based phase
//! without any objections to the phase being raised. If two (or more) phases
//! share a common successor, such as the #uvm_run_phase and the
//! #uvm_post_shutdown_phase sharing the #uvm_extract_phase as a successor,
//! then phase advancement is delayed until all predecessors of the common
//! successor are ready to proceed.  Because of this, it is possible for time to
//! advance between <uvm_component::phase_started> and <uvm_component::phase_ended>
//! of a process phase without any participants in the phase raising an objection.
//------------------------------------------------------------------------------

class uvm_process_phase : public uvm_phase
{
 public:
  explicit uvm_process_phase( const std::string& name );

  void traverse( uvm_component* comp,
                         uvm_phase* phase,
                         uvm_phase_state state ) override;

  void m_traverse( uvm_component* comp,
                   uvm_phase* phase,
                   uvm_phase_state state );

  void execute( uvm_component* comp,
                        uvm_phase* phase ) override;

  void kill( uvm_component* comp,
             uvm_phase* phase ) override;

  void exec_proc( uvm_component* comp,
                  uvm_phase* phase );

 private:
  //std::string m_proc_name;
  std::map<uvm_component*, sc_core::sc_process_handle> m_proc_handle;
};


} // namespace uvm

#endif /* UVM_PROCESS_PHASE_H_ */
