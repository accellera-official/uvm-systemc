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

#ifndef UVM_COMMON_PHASES_H_
#define UVM_COMMON_PHASES_H_

#include "uvmsc/phasing/uvm_phase.h"
#include "uvmsc/phasing/uvm_topdown_phase.h"
#include "uvmsc/phasing/uvm_bottomup_phase.h"
#include "uvmsc/phasing/uvm_process_phase.h"

//////////////

namespace uvm {


//----------------------------------------------------------------------
// Title: UVM Common Phases
//
// The common phases are the set of function and task phases that all
// objects of type #uvm_component execute together.
// All objects of type #uvm_component are always synchronized
// with respect to the common phases.
//
// The common phases are executed in the sequence they are specified below.
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Class: uvm_build_phase
//
//! Create and configure of testbench structure
//!
//! #uvm_topdown_phase that calls the
//! uvm_component::build_phase method.
//!
//! Upon entry:
//!  - The top-level components have been instantiated under <uvm_root>.
//!  - Current simulation time is still equal to 0 but some "delta cycles" may have occurred
//!
//! Typical Uses:
//!  - Instantiate sub-components.
//!  - Instantiate register model.
//!  - Get configuration values for the component being built.
//!  - Set configuration values for sub-components.
//!
//! Exit Criteria:
//!  - All objects of type #uvm_component have been instantiated.
//----------------------------------------------------------------------

class uvm_build_phase : public uvm_topdown_phase
{
 public:

  virtual void exec_func( uvm_component* comp, uvm_phase* phase );

  static uvm_build_phase* get();

  virtual const std::string get_type_name() const;

  static uvm_build_phase* m_inst;
  static const std::string type_name;

 protected:
  explicit uvm_build_phase( const std::string& name = "build" );
};


//----------------------------------------------------------------------
// Class: uvm_connect_phase
//
//! Establish cross-component connections.
//!
//! #uvm_bottomup_phase that calls the
//!  uvm_component::connect_phase method.
//!
//! Upon Entry:
//! - All components have been instantiated.
//! - Current simulation time is still equal to 0
//!   but some "delta cycles" may have occurred.
//!
//! Typical Uses:
//! - Connect TLM ports and exports.
//! - Connect TLM initiator sockets and target sockets.
//! - Connect register model to adapter components.
//! - Setup explicit phase domains.
//!
//! Exit Criteria:
//! - All cross-component connections have been established.
//! - All independent phase domains are set.
//----------------------------------------------------------------------

class uvm_connect_phase : public uvm_bottomup_phase
{
 public:

  virtual void exec_func(uvm_component* comp, uvm_phase* phase);

  static uvm_connect_phase* get();

  virtual const std::string get_type_name() const;

  static uvm_connect_phase* m_inst;
  static const std::string type_name;

 protected:
  uvm_connect_phase( const std::string& name = "connect" );
};


//----------------------------------------------------------------------
// Class: uvm_end_of_elaboration_phase
//
//! Fine-tune the testbench.
//!
//! #uvm_bottomup_phase that calls the
//! uvm_component::end_of_elaboration_phase method.
//!
//! Upon Entry:
//! - The verification environment has been completely assembled.
//! - Current simulation time is still equal to 0
//!   but some "delta cycles" may have occurred.
//!
//! Typical Uses:
//! - Display environment topology.
//! - Open files.
//! - Define additional configuration settings for components.
//!
//! Exit Criteria:
//! - None.
//----------------------------------------------------------------------

class uvm_end_of_elaboration_phase : public uvm_bottomup_phase
{
 public:

  virtual void exec_func(uvm_component* comp, uvm_phase* phase);

  static uvm_end_of_elaboration_phase* get();

  virtual const std::string get_type_name() const;

  static uvm_end_of_elaboration_phase* m_inst;
  static const std::string type_name;

 protected:
  uvm_end_of_elaboration_phase( const std::string& name = "end_of_elaboration" );
};

//----------------------------------------------------------------------
// Class: uvm_start_of_simulation_phase
//
//! Get ready for DUT to be simulated.
//!
//! #uvm_bottomup_phase that calls the
//! uvm_component::start_of_simulation_phase method.
//!
//! Upon Entry:
//! - Other simulation engines, debuggers, hardware assisted platforms and
//!   all other run-time tools have been started and synchronized.
//! - The verification environment has been completely configured
//!   and is ready to start.
//! - Current simulation time is still equal to 0
//!   but some "delta cycles" may have occurred.
//!
//! Typical Uses:
//! - Display environment topology
//! - Set debugger breakpoint
//! - Set initial run-time configuration values.
//!
//! Exit Criteria:
//! - None.
//----------------------------------------------------------------------

class uvm_start_of_simulation_phase : public uvm_bottomup_phase
{
 public:

  virtual void exec_func(uvm_component* comp, uvm_phase* phase);

  static uvm_start_of_simulation_phase* get();

  virtual const std::string get_type_name() const;

  static uvm_start_of_simulation_phase* m_inst;
  static const std::string type_name;

 protected:
  uvm_start_of_simulation_phase( const std::string& name = "start_of_simulation" );
};

//----------------------------------------------------------------------
// Class: uvm_run_phase
//
//! Stimulate the DUT.
//
//! This #uvm_process_phase calls the
//! <uvm_component::run_phase virtual method. This phase runs in
//! parallel to the runtime phases, #uvm_pre_reset_phase through
//! #uvm_post_shutdown_phase. All components in the testbench
//! are synchronized with respect to the run phase regardles of
//! the phase domain they belong to.
//!
//! Upon Entry:
//! - Indicates that power has been applied.
//! - There should not have been any active clock edges before entry
//!   into this phase (e.g. x->1 transitions via initial blocks).
//! - Current simulation time is still equal to 0
//!   but some "delta cycles" may have occurred.
//!
//! Typical Uses:
//! - Components implement behavior that is exhibited for the entire
//!   run-time, across the various run-time phases.
//! - Backward compatibility with OVM.
//!
//! Exit Criteria:
//! - The DUT no longer needs to be simulated, and
//! - The <uvm_post_shutdown_ph> is ready to end
//!
//! The run phase terminates in one of two ways.
//!
//! 1. All run_phase objections are dropped:
//!
//!   When all objections on the run_phase objection have been dropped,
//!   the phase ends and all of its threads are killed.
//!   If no component raises a run_phase objection immediately upon
//!   entering the phase, the phase ends immediately.
//!
//!
//! 2. Timeout:
//!
//!   The phase ends if the timeout expires before all objections are dropped.
//!   By default, the timeout is set to 9200 seconds.
//!   You may override this via <uvm_root::set_timeout>.
//!
//!   If a timeout occurs in your simulation, or if simulation never
//!   ends despite completion of your test stimulus, then it usually indicates
//!   that a component continues to object to the end of a phase.
//----------------------------------------------------------------------

class uvm_run_phase : public uvm_process_phase
{
 public:
  virtual void exec_process(uvm_component* comp, uvm_phase* phase);

  static uvm_run_phase* get();

  virtual const std::string get_type_name() const;

  static uvm_run_phase* m_inst;
  static const std::string type_name;

 protected:
  uvm_run_phase( const std::string& name = "run");
};


//----------------------------------------------------------------------
// Class: uvm_extract_phase
//
//! Extract data from different points of the verficiation environment.
//!
//! <uvm_bottomup_phase> that calls the
//! <uvm_component::extract_phase> method.
//!
//! Upon Entry:
//! - The DUT no longer needs to be simulated.
//! - Simulation time will no longer advance.
//!
//! Typical Uses:
//! - Extract any remaining data and final state information
//!   from scoreboard and testbench components
//! - Probe the DUT (via zero-time hierarchical references
//!   and/or backdoor accesses) for final state information.
//! - Compute statistics and summaries.
//! - Display final state information
//! - Close files.
//
//! Exit Criteria:
//! - All data has been collected and summarized.
//----------------------------------------------------------------------

class uvm_extract_phase : public uvm_bottomup_phase
{
 public:

  virtual void exec_func(uvm_component* comp, uvm_phase* phase);

  static uvm_extract_phase* get();

  virtual const std::string get_type_name() const;

  static uvm_extract_phase* m_inst;
  static const std::string type_name;

 protected:
  uvm_extract_phase( const std::string& name = "extract" );
};


//----------------------------------------------------------------------
// Class: uvm_check_phase
//
//! Check for any unexpected conditions in the verification environment.
//!
//! #uvm_bottomup_phase that calls the
//! uvm_component::check_phase method.
//!
//! Upon Entry:
//! - All data has been collected.
//!
//! Typical Uses:
//! - Check that no unaccounted-for data remain.
//!
//! Exit Criteria:
//! - Test is known to have passed or failed.
//----------------------------------------------------------------------

class uvm_check_phase : public uvm_bottomup_phase
{
 public:

  virtual void exec_func(uvm_component* comp, uvm_phase* phase);

  static uvm_check_phase* get();

  virtual const std::string get_type_name() const;

  static uvm_check_phase* m_inst;
  static const std::string type_name;

 protected:
  uvm_check_phase( const std::string& name = "check" );

};


//----------------------------------------------------------------------
// Class: uvm_report_phase
//
//! Report results of the test.
//!
//! #uvm_bottomup_phase that calls the
//! uvm_component::report_phase method.
//
//! Upon Entry:
//! - Test is known to have passed or failed.
//!
//! Typical Uses:
//! - Report test results.
//! - Write results to file.
//
//! Exit Criteria:
//! - End of test.
//----------------------------------------------------------------------

class uvm_report_phase : public uvm_bottomup_phase
{
 public:

  virtual void exec_func(uvm_component* comp, uvm_phase* phase);

  static uvm_report_phase* get();

  virtual const std::string get_type_name() const;

  static uvm_report_phase* m_inst;
  static const std::string type_name;

 protected:
  uvm_report_phase( const std::string& name = "report" );

};


//----------------------------------------------------------------------
// Class: uvm_final_phase
//
//! Tie up loose ends.
//!
//! #uvm_topdown_phase that calls the
//! uvm_component::final_phase method.
//!
//! Upon Entry:
//! - All test-related activity has completed.
//!
//! Typical Uses:
//! - Close files.
//! - Terminate co-simulation engines.
//!
//! Exit Criteria:
//! - Ready to exit simulator.
//----------------------------------------------------------------------

class uvm_final_phase : public uvm_topdown_phase
{
 public:

  virtual void exec_func(uvm_component* comp, uvm_phase* phase);

  static uvm_final_phase* get();

  virtual const std::string get_type_name() const;

  static uvm_final_phase* m_inst;
  static const std::string type_name;

 protected:
  uvm_final_phase( const std::string& name = "final" );

};


} // namespace uvm

#endif /* UVM_COMMON_PHASES_H_ */
