//----------------------------------------------------------------------
//   Copyright 2019 COSEDA Technologies GmbH
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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

#include <sstream>

#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/seq/uvm_sequencer_base.h"
#include "uvmsc/macros/uvm_message_defines.h"
#include "uvmsc/phasing/uvm_process_phase.h"
#include "uvmsc/misc/uvm_misc.h"

//////////////

namespace uvm {

//----------------------------------------------------------------------
// Constructor
//
//! Create a new instance of a process-based phase
//----------------------------------------------------------------------

uvm_process_phase::uvm_process_phase( const std::string& name )
  : uvm_phase(name, UVM_PHASE_IMP)
{
  m_proc_handle.clear();
}


//----------------------------------------------------------------------
// member function: traverse
//
//! Traverses the component tree in bottom-up order, calling #execute for
//! each component. The actual order for process-based phases doesn't really
//! matter, as each component task is executed in a separate process whose
//! starting order is not deterministic.
//----------------------------------------------------------------------

void uvm_process_phase::traverse( uvm_component* comp,
                                  uvm_phase* phase,
                                  uvm_phase_state state )
{
  phase->m_num_procs_not_yet_returned = 0;
  m_traverse(comp, phase, state);
}

void uvm_process_phase::m_traverse( uvm_component* comp,
                                    uvm_phase* phase,
                                    uvm_phase_state state )
{
  std::string name;
  uvm_domain* phase_domain = phase->get_domain();
  uvm_domain* comp_domain = comp->get_domain();

  if (comp->get_first_child(name))
    do
      m_traverse(comp->get_child(name), phase, state);
    while(comp->get_next_child(name));

  if (m_phase_trace)
  {
    std::ostringstream str;
    str << "process phase = " << phase->get_name() << std::endl
        << " state = "  << uvm_phase_state_name[state] << std::endl
        << " component = " <<  comp->get_full_name() << std::endl
        << " component domain = " << comp_domain->get_name() << std::endl
        << " phase domain = " << phase_domain->get_name();
    UVM_INFO("PH_TRACE", str.str(), UVM_DEBUG);
  }

  if (phase_domain == uvm_domain::get_common_domain() ||
      phase_domain == comp_domain)
  {
    switch(state)
    {
      case UVM_PHASE_STARTED:
      {
        comp->m_current_phase = phase;
        comp->m_apply_verbosity_settings(phase);
        comp->phase_started(*phase);

        uvm_sequencer_base* seqr = dynamic_cast<uvm_sequencer_base*>(comp);
        if (seqr != NULL)
          seqr->start_phase_sequence(*phase); // TODO make phase a ref?

        break;
      }
      case UVM_PHASE_EXECUTING:
      {
        uvm_phase* ph = this;
        if (comp->m_phase_imps.find(this) != comp->m_phase_imps.end() ) // if exists
          ph = comp->m_phase_imps[this];
        ph->execute(comp, phase);
        break;
      }
      case UVM_PHASE_READY_TO_END:
      {
        comp->phase_ready_to_end(*phase);
        break;
      }
      case UVM_PHASE_ENDED:
      {
        comp->phase_ended(*phase);
        uvm_phase* ph = this;
        ph->kill(comp, phase);
        comp->m_current_phase = NULL;
        break;
      }
      default:
        uvm_report_fatal("PH_BADEXEC","process phase traverse internal error");
        break;
    }
  }
}

//----------------------------------------------------------------------
// member function: execute
//
//! Will spawn the process-based phase \p phase for the component \p comp.
//----------------------------------------------------------------------

void uvm_process_phase::kill( uvm_component* comp,
                              uvm_phase* phase )
{
  if (m_proc_handle.find(comp) != m_proc_handle.end() ) // exists
  {
    if (!m_proc_handle[comp].terminated())
    {
      m_proc_handle[comp].kill();
      m_proc_handle.erase(comp);

      std::ostringstream str;
      str << "Phase '" << phase->get_full_name() << "' (id = "
          << phase->get_inst_id() << ") "
          << "for component " << comp->get_full_name();
      UVM_INFO("PH_KILL", str.str(), UVM_DEBUG);
    }
    else
    {
      // process already finished, remove from list
      std::ostringstream str;
      str << "Phase '" << phase->get_full_name() << "' (id = "
          << phase->get_inst_id() << ") "
          << "for component " << comp->get_full_name()
          << " already terminated.";
      UVM_INFO("PH_TERM", str.str(), UVM_DEBUG);

      m_proc_handle.erase(comp);
    }
  }
  else
  {
    std::ostringstream str;
    str << "No valid process handle found to kill " << phase->get_name()
        << " phase for component; process may have finished for " << comp->get_full_name();
    UVM_INFO("PH_NOKILL", str.str(), UVM_DEBUG);
  }

}


//----------------------------------------------------------------------
// member function: execute
//
//! Will spawn the process-based phase \p phase for the component \p comp.
//----------------------------------------------------------------------

void uvm_process_phase::execute( uvm_component* comp,
                                 uvm_phase* phase )
{
  if (m_proc_handle.find(comp) != m_proc_handle.end() ) // exists
    {
      std::ostringstream str;
      str << "Process handle to already in use for spawned process '" << m_proc_handle[comp].name() << "'.";
      UVM_FATAL("PH_EXEC", str.str());
    }
    // else handle does not exist and can be used
  else
  {
    std::string procname = "exec_proc_"+phase->get_name()+ "_" + uvm_flatten_name(comp->get_full_name());
#if IEEE_1666_SYSTEMC >= 202301L    
    sc_core::sc_hierarchy_scope scope(comp->get_hierarchy_scope());
    m_proc_handle[comp] =
      sc_spawn(sc_bind(&uvm_process_phase::exec_proc, this, comp, phase),
         sc_core::sc_gen_unique_name(procname.c_str()));
#else
	comp->simcontext()->hierarchy_push(comp);
    m_proc_handle[comp] =
      sc_spawn(sc_bind(&uvm_process_phase::exec_proc, this, comp, phase),
         sc_core::sc_gen_unique_name(procname.c_str()));
	comp->simcontext()->hierarchy_pop();
#endif
    comp->m_set_run_handle(m_proc_handle[comp]);

    std::ostringstream str;
    str << "Phase '" << phase->get_full_name() << "' (id = "
        << phase->get_inst_id() << ") "
        << "for component " << comp->get_full_name();
    UVM_INFO("PH_SPAWN", str.str(), UVM_DEBUG);
  }
}
//----------------------------------------------------------------------
// member function: exec_proc
//
//! This is the process which will be spawned to allow multiple run
//! processes running in parallel
//----------------------------------------------------------------------

void uvm_process_phase::exec_proc( uvm_component* comp,
                                   uvm_phase* phase )
{
  phase->m_num_procs_not_yet_returned++;

  // TODO
  // reseed this process for random stability
  //process proc;
  //proc = process::self();
  //proc.srandom(uvm_create_random_seed(phase.get_type_name(), comp.get_full_name()));
  exec_process(comp,phase);
  
  phase->m_num_procs_not_yet_returned--;
}

} // namespace uvm
