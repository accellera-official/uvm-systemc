//----------------------------------------------------------------------
//   Copyright 2019 COSEDA Technologies GmbH
//   Copyright 2014 Université Pierre et Marie Curie, Paris
//   Copyright 2013-2014 Fraunhofer-Gesellschaft zur Foerderung
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

#include <cstdlib>

#include <systemc>
#include <map>

#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/phasing/uvm_objection.h"
#include "uvmsc/phasing/uvm_process_phase.h"
#include "uvmsc/phasing/uvm_phase.h"
#include "uvmsc/phasing/uvm_domain.h"
#include "uvmsc/phasing/uvm_common_phases.h"
#include "uvmsc/macros/uvm_message_defines.h"
#include "uvmsc/conf/uvm_config_db.h"
#include "uvmsc/misc/uvm_misc.h"
#include "uvmsc/report/uvm_report_server.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"


#define UVM_PH_TRACE(ID,MSG,PH,VERB) \
{ \
  std::ostringstream str;\
  str << "Phase '" << PH->get_full_name() << "' (id = " \
      << PH->get_inst_id() << ") " << MSG; \
  UVM_INFO(ID, str.str(), VERB); \
}

using namespace sc_core;

namespace uvm {

//----------------------------------------------------------------------
// Static data member initialized here
//----------------------------------------------------------------------

// Refactored from the former uvm_phase::m_phase_trace and
// uvm_phase::m_phase_hopper globals to uvm_coreservice_t.

bool& uvm_phase::m_phase_trace()
{
  return uvm_coreservice_t::get()->get_uvm_phase_m_phase_trace();
}

uvm_phase_queue<uvm_phase*>* uvm_phase::m_phase_hopper()
{
  return uvm_coreservice_t::get()->get_uvm_phase_m_phase_hopper();
}

//----------------------------------------------------------------------
// Group: Construction
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

uvm_phase::uvm_phase( const std::string& name,
                      uvm_phase_type phase_type,
                      uvm_phase* parent) : uvm_object( name )
{
  m_phase_type = phase_type;
  (void)m_phase_hopper();

  /* TODO uvm_test_done_objection is deprecated???
  if (name == "run")
    phase_done = uvm_test_done_objection::get();
  else
  */
    phase_done = new uvm_objection(name+"_objection");

  m_parent = parent;
  m_imp = nullptr;
  m_nodes.clear();

  max_ready_to_end_iter = 20;

  m_end_node = nullptr;
  m_predecessors.clear();
  m_successors.clear();
  m_sync.clear();

  m_num_procs_not_yet_returned = 0;
  m_state = UVM_PHASE_DORMANT;
  m_run_count = 0;

  /* TODO command line interface
    uvm_cmdline_processor clp = uvm_cmdline_processor::get_inst();
    std::string val;
    if (clp.get_arg_value("+UVM_PHASE_TRACE", val))
      m_phase_trace() = 1;
    else
      m_phase_trace() = 0;
    if (clp.get_arg_value("+UVM_USE_OVM_RUN_SEMANTIC", val))
      m_use_ovm_run_semantic = 1;
    else
      m_use_ovm_run_semantic = 0;
  */

  // TODO remove when command line interface is implemented
  m_use_ovm_run_semantic = false;

  if (parent == nullptr && (phase_type == UVM_PHASE_SCHEDULE ||
                         phase_type == UVM_PHASE_DOMAIN ))
  {
    //m_parent = this; // was already commented in UVM-SV
    m_end_node = new uvm_phase((name+"_end"), UVM_PHASE_TERMINAL, this);
    this->m_successors[m_end_node] = true;
    m_end_node->m_predecessors[this] = true;
  }

  m_jump_fwd = false;
  m_jump_bkwd = false;
  m_jump_phase = nullptr;
}

//----------------------------------------------------------------------
// member function: get_phase_type
//
//! Returns the phase type as defined by #uvm_phase_type
//----------------------------------------------------------------------

uvm_phase_type uvm_phase::get_phase_type() const
{
  return m_phase_type;
}

//----------------------------------------------------------------------
// Group: State
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: get_state
//
//! Accessor to return current state of this phase
//----------------------------------------------------------------------

uvm_phase_state uvm_phase::get_state() const
{
  return m_state;
}

//----------------------------------------------------------------------
// member function: get_run_count
//
//! Accessor to return the integer number of times this phase has executed
//----------------------------------------------------------------------

int uvm_phase::get_run_count() const
{
  return m_run_count;
}

//----------------------------------------------------------------------
// member function: find_by_name
//
//! Locate a phase node with the specified \p name and return its handle.
//! With \p stay_in_scope set, searches only within this phase's schedule or
//! domain.
//----------------------------------------------------------------------

uvm_phase* uvm_phase::find_by_name( const std::string& name,
                                    bool stay_in_scope ) const
{
  // TBD full search - comment from UVM-SV

  if (m_phase_trace())
  {
    std::cout <<"\nFIND node named '" << name << "' within "
         << get_name() << " (scope " << uvm_phase_type_name[m_phase_type] << ")";
    if (stay_in_scope)
      std::cout << " staying within scope";
    std::cout << std::endl;
  }

  if (get_name() == name)
    return const_cast<uvm_phase*>(this); // TODO avoid const-cast!

  const uvm_phase* ph_name;
  ph_name = m_find_predecessor_by_name(name, stay_in_scope, this);

  if (ph_name == nullptr)
    ph_name = m_find_successor_by_name(name, stay_in_scope, this);

  return const_cast<uvm_phase*>(ph_name); // TODO avoid const-cast!
}

//----------------------------------------------------------------------
// member function: find
//
//! Locate the phase node with the specified \p phase IMP and return its handle.
//! With \p stay_in_scope set, searches only within this phase's schedule or
//! domain.
//----------------------------------------------------------------------

uvm_phase* uvm_phase::find( const uvm_phase* phase,
                            bool stay_in_scope) const
{
  // TBD full search - comment from UVM-SV

  if (m_phase_trace())
  {
    std::cout << "FIND node '" << phase->get_name()
         << "' within " << get_name()
         << " (scope " << uvm_phase_type_name[m_phase_type]
         << ")";

    if (stay_in_scope)
      std::cout << " staying within scope";
    if (phase == this)
      std::cout << ", node is equal to current phase (this)";
    if (phase == m_imp)
      std::cout << ", node is the top-level entry.";
    std::cout << std::endl;
  }

  if (phase == m_imp || phase == this)
    return const_cast<uvm_phase*>(phase); // TODO avoid const-cast!

  const uvm_phase* find_phase = m_find_predecessor( phase, stay_in_scope, this );

  if (find_phase == nullptr)
    find_phase = m_find_successor( phase, stay_in_scope, this );

  return const_cast<uvm_phase*>(find_phase); // TODO avoid const-cast!
}


//----------------------------------------------------------------------
// member function: is
//
//! Returns true if the containing \p uvm_phase refers to the same phase
//! as the phase argument, false otherwise
//----------------------------------------------------------------------

bool uvm_phase::is( const uvm_phase* phase ) const
{
  return (m_imp == phase || this == phase);
}

//----------------------------------------------------------------------
// member function: is_before
//
//! Returns true if the containing \p uvm_phase refers to a phase that is earlier
//! than the phase argument, false otherwise
//----------------------------------------------------------------------

bool uvm_phase::is_before( const uvm_phase* phase ) const
{
  if (m_phase_trace())
  {
    std::cout << "this = " << get_name() << " is before phase = "
         << phase->get_name() << "?" << std::endl;
  }

  // TBD: add support for 'stay_in_scope=1' functionality - comment from UVM-SV

  return (!is(phase) && m_find_successor(phase, 0, this) != nullptr);
}

//----------------------------------------------------------------------
// member function: is_after
//
//! Returns true if the containing \p uvm_phase refers to a phase that is later
//! than the phase argument, false otherwise
//----------------------------------------------------------------------

bool uvm_phase::is_after( const uvm_phase* phase ) const
{
  //std::cout << "this=" << get_name() << " is after phase=" << phase.get_name() << "?" << std::endl;
  // TODO UVM_SV: add support for 'stay_in_scope=1' functionality

  return (!is(phase) && m_find_predecessor(phase, 0, this) != nullptr);
}


//----------------------------------------------------------------------
// Group: Callbacks
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: exec_func (virtual)
//
//! Implements the functor/delegate functionality for a function phase type
//! \p comp  - the component to execute the functionality upon
//! \p phase - the phase schedule that originated this phase call
//----------------------------------------------------------------------

void uvm_phase::exec_func( uvm_component* comp, uvm_phase* phase )
{
}

//----------------------------------------------------------------------
// member function: exec_process (virtual)
//
//! Implements the functor/delegate functionality for a process phase type
//! \p comp  - the component to execute the functionality upon
//! \p phase - the phase schedule that originated this phase call
//!
//! NOTE - this was the exec_task in UVM-SV
//----------------------------------------------------------------------

void uvm_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
}

//----------------------------------------------------------------------
// Group: Schedule
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: add
//
//! Build up a schedule structure inserting phase by phase, specifying linkage
//! Phases can be added anywhere, in series or parallel with existing nodes
//!  \p phase        - handle of singleton derived imp containing actual functor.
//!                  by default the new phase is appended to the schedule
//!  \p with_phase   - specify to add the new phase in parallel with this one
//!  \p after_phase  - specify to add the new phase as successor to this one
//!  \p before_phase - specify to add the new phase as predecessor to this one
//----------------------------------------------------------------------

void uvm_phase::add( uvm_phase* phase,
                     uvm_phase* with_phase,
                     uvm_phase* after_phase,
                     uvm_phase* before_phase )
{
  uvm_phase* new_node = nullptr; // moved to data member to guarantee lifetime
  uvm_phase* begin_node = nullptr;
  uvm_phase* end_node = nullptr;

  if (phase == nullptr)
    UVM_FATAL("PH/NULL", "add: phase argument is nullptr");

  if (with_phase != nullptr && with_phase->get_phase_type() == UVM_PHASE_IMP)
  {
    std::string nm = with_phase->get_name();
    uvm_phase* ph = find(with_phase);
    if (ph == nullptr)
      UVM_FATAL("PH_BAD_ADD",
         "cannot find with_phase '" + nm + "' within node '" + get_name() + "'");
  }

  if (before_phase != nullptr && before_phase->get_phase_type() == UVM_PHASE_IMP)
  {
    std::string nm = before_phase->get_name();
    uvm_phase* ph = find(before_phase);
    if (ph == nullptr)
      UVM_FATAL("PH_BAD_ADD",
         "cannot find before_phase '" + nm + "' within node '" + get_name() + "'");
  }

  if (after_phase != nullptr && after_phase->get_phase_type() == UVM_PHASE_IMP)
  {
    std::string nm = after_phase->get_name();
    uvm_phase* ph = find(after_phase);
    if (ph == nullptr)
      UVM_FATAL("PH_BAD_ADD",
         "cannot find after_phase '" + nm + "' within node '" + get_name() + "'");
  }

  if (with_phase != nullptr && (after_phase != nullptr || before_phase != nullptr))
    UVM_FATAL("PH_BAD_ADD",
       "cannot specify both 'with' and 'before/after' phase relationships");

  if (before_phase == this || after_phase == m_end_node || with_phase == m_end_node)
    UVM_FATAL("PH_BAD_ADD",
       "cannot add before begin node, after end node, or with end nodes");

  // If we are inserting a new "leaf node"
  if (phase->get_phase_type() == UVM_PHASE_IMP)
  {
    new_node = new uvm_phase(phase->get_name(), UVM_PHASE_NODE, this);
    new_node->m_imp = phase;
    begin_node = new_node;
    end_node = new_node;
    m_nodes[new_node] = this;
  }
  // We are inserting an existing schedule
  else
  {
    begin_node = phase;
    end_node   = phase->m_end_node;
    phase->m_parent = this;
  }

  // If 'with_phase' is us, then insert node in parallel
  if (with_phase == this)
  {
    after_phase = this;
    before_phase = m_end_node;
  }

  // If no before/after/with specified, insert at end of this schedule
  if (with_phase == nullptr && after_phase == nullptr && before_phase == nullptr)
    before_phase = m_end_node;

  if (m_phase_trace())
  {
    uvm_phase_type typ = phase->get_phase_type();

    std::ostringstream str1, str2;
    if (new_node != nullptr)
      str1 << new_node->get_name() << " (inst_id = " << new_node->get_inst_id() << ")";
    else str1 << "";

    str2 << get_name() << " (" << uvm_phase_type_name[m_phase_type]
         << ") ADD_PHASE: " << std::endl
         << " phase full name = " << phase->get_full_name() << std::endl
         << " phase name = " << phase->get_name() << " ("
         << uvm_phase_type_name[typ] << ", inst_id = " << phase->get_inst_id() << ")" << std::endl
         << " with_phase = " << ((with_phase == nullptr) ? "nullptr" : with_phase->get_name()) << std::endl
         << " after_phase = " <<  ((after_phase == nullptr)  ? "nullptr" : after_phase->get_name()) << std::endl
         << " before_phase = " << ((before_phase == nullptr) ? "nullptr" : before_phase->get_name()) << std::endl
         << " new_node = " << ((new_node == nullptr) ? "nullptr" : str1.str() ) << std::endl
         << " begin_node = " << ((begin_node == nullptr) ? "nullptr" : begin_node->get_name()) << std::endl
         << " end_node = " << ((end_node == nullptr) ? "nullptr" : end_node->get_name());
    UVM_INFO("PH/TRC/ADD_PH", str2.str(), UVM_DEBUG);
  }

  // INSERT IN PARALLEL WITH 'WITH' PHASE
  if( with_phase != nullptr )
  {
    begin_node->m_predecessors = with_phase->m_predecessors;
    end_node->m_successors = with_phase->m_successors;
    for( m_schedulemapItT it = with_phase->m_predecessors.begin();
         it != with_phase->m_predecessors.end();
         it++)
      it->first->m_successors[begin_node] = true;

    for( m_schedulemapItT it = with_phase->m_successors.begin();
             it != with_phase->m_successors.end();
             it++)
          it->first->m_predecessors[end_node] = true;
  }

  // INSERT BEFORE PHASE
  else
    if( before_phase != nullptr && after_phase == nullptr )
    {
      begin_node->m_predecessors = before_phase->m_predecessors;
      end_node->m_successors[before_phase] = true;

      for( m_schedulemapItT it = before_phase->m_predecessors.begin();
               it != before_phase->m_predecessors.end();
               it++)
      {
        it->first->m_successors.erase(before_phase);
        it->first->m_successors[begin_node] = true;
      }
      before_phase->m_predecessors.clear();
      before_phase->m_predecessors[end_node] = true;
    }

  // INSERT AFTER PHASE
  else
    if( before_phase == nullptr && after_phase != nullptr )
    {
      end_node->m_successors = after_phase->m_successors;
      begin_node->m_predecessors[after_phase] = true;

      for( m_schedulemapItT it = after_phase->m_successors.begin();
               it != after_phase->m_successors.end();
               it++)
      {
        it->first->m_predecessors.erase(after_phase);
        it->first->m_predecessors[end_node] = true;
      }

      after_phase->m_successors.clear();
      after_phase->m_successors[begin_node] = true;
    }

  // IN BETWEEN 'BEFORE' and 'AFTER' PHASES
  else
    if (before_phase != nullptr && after_phase != nullptr )
    {
      if (!after_phase->is_before(before_phase))
      {
        UVM_FATAL("PH_ADD_PHASE","Phase '"+before_phase->get_name()+
                 "' is not before phase '"+after_phase->get_name()+"'");
      }

      // before and after? add 1 pred and 1 succ
      begin_node->m_predecessors[after_phase] = true;
      end_node->m_successors[before_phase] = true;
      after_phase->m_successors[begin_node] = true;
      before_phase->m_predecessors[end_node] = true;

      if (after_phase->m_successors.find(before_phase) !=
          after_phase->m_successors.end()) // if exists
      {
        after_phase->m_successors.erase(before_phase);
        before_phase->m_successors.erase(after_phase);
      }
    }
}

//----------------------------------------------------------------------
// member function: get_parent
//
//! Returns the parent schedule node, if any, for hierarchical graph traversal
//----------------------------------------------------------------------

uvm_phase* uvm_phase::get_parent() const
{
  return m_parent;
}

//----------------------------------------------------------------------
// member function: get_full_name
//
//! Returns the full path from the enclosing domain down to this node.
//! The singleton IMP phases have no hierarchy.
//----------------------------------------------------------------------

const std::string uvm_phase::get_full_name() const
{
  std::string dom, sch;
  std::string full_name;

  if (m_phase_type == UVM_PHASE_IMP)
    return get_name();

  full_name = get_domain_name();

  sch = get_schedule_name();
  if (!sch.empty())
    full_name = full_name + "." + sch;

  if (m_phase_type != UVM_PHASE_DOMAIN && m_phase_type != UVM_PHASE_SCHEDULE)
    full_name = full_name + "." + get_name();

  return full_name;
}

//----------------------------------------------------------------------
// member function: get_schedule
//
//! Returns the topmost parent schedule node, if any, for hierarchical graph traversal
//----------------------------------------------------------------------

uvm_phase* uvm_phase::get_schedule( bool hier ) const
{
  uvm_phase* sched = const_cast<uvm_phase*>(this);
  if (hier)
    while (sched->m_parent != nullptr &&
          (sched->m_parent->get_phase_type() == UVM_PHASE_SCHEDULE))
      sched = sched->m_parent;

  if (sched->m_phase_type == UVM_PHASE_SCHEDULE)
    return sched;

  if (sched->m_phase_type == UVM_PHASE_NODE)
    if (m_parent != nullptr && m_parent->m_phase_type != UVM_PHASE_DOMAIN)
      return m_parent;

  return nullptr;
}

//----------------------------------------------------------------------
// member function: get_domain
//
//! Returns the enclosing domain
//----------------------------------------------------------------------

uvm_domain* uvm_phase::get_domain() const
{
  uvm_phase* phase = const_cast<uvm_phase*>(this);

  while (phase != nullptr && phase->m_phase_type != UVM_PHASE_DOMAIN)
    phase = phase->m_parent;
  if (phase == nullptr) // no parent domain
    return nullptr;

  uvm_domain* domain = dynamic_cast<uvm_domain*>(phase);

  if(domain == nullptr)
      UVM_FATAL("PH/INTERNAL", "get_domain: m_phase_type is DOMAIN but dynamic_cast to uvm_domain fails");

  return domain;
}

//----------------------------------------------------------------------
// member function: get_imp
//
//! Returns the phase implementation for this this node.
//! Returns nullptr if this phase type is not a UVM_PHASE_LEAF_NODE.
//----------------------------------------------------------------------

uvm_phase* uvm_phase::get_imp() const
{
  return m_imp;
}

//----------------------------------------------------------------------
// member function: get_schedule_name
//
//! Returns the schedule name associated with this phase node
//----------------------------------------------------------------------

std::string uvm_phase::get_schedule_name( bool hier ) const
{
  uvm_phase* sched;
  std::string s;

  sched = get_schedule(hier);

  if (sched == nullptr)
    return "";

  s = sched->get_name();

  while (sched->m_parent != nullptr && sched->m_parent != sched &&
          (sched->m_parent->get_phase_type() == UVM_PHASE_SCHEDULE))
  {
    sched = sched->m_parent;
    s = sched->get_name()+( s.length()>0 ? "." : "" )+s;
  }

  return s;
}

//----------------------------------------------------------------------
// member function: get_domain_name
//
//! Returns the domain name associated with this phase node
//----------------------------------------------------------------------

std::string uvm_phase::get_domain_name() const
{
  uvm_domain* domain;
  domain = get_domain();
  if (domain == nullptr)
    return "unknown";

  return domain->get_name();
}

//----------------------------------------------------------------------
// Group: Synchronization
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: get_objection
//
//! Return the #uvm_objection that gates the termination of the phase.
//----------------------------------------------------------------------

uvm_objection* uvm_phase::get_objection() const
{
  return phase_done;
}

//----------------------------------------------------------------------
// member function: raise_objection (virtual)
//
//! The member function #raise_objection shall raise an objection to
//! ending this phase.
//----------------------------------------------------------------------

void uvm_phase::raise_objection( uvm_object* obj,
                                 const std::string& description,
                                 int count )
{
  phase_done->raise_objection(obj, description, count);
}

//----------------------------------------------------------------------
// member function: drop_objection (virtual)
//
//! The member function #drop_objection shall drop an objection to
//! ending this phase.
//----------------------------------------------------------------------

void uvm_phase::drop_objection( uvm_object* obj,
                                const std::string& description,
                                int count )
{
  phase_done->drop_objection(obj, description, count);
}

//----------------------------------------------------------------------
// member function: sync
//
//! Synchronize two domains, fully or partially
//!
//!  \p target       - handle of target domain to synchronize this one to
//!  \p phase        - optional single phase in this domain to synchronize,
//!                  otherwise sync all
//!  \p with_phase   - optional different target-domain phase to synchronize with,
//!                  otherwise use ~phase~ in the target domain
//----------------------------------------------------------------------

void uvm_phase::sync( uvm_domain& target,
                      uvm_phase* phase,
                      uvm_phase* with_phase )
{
  if (!this->m_is_domain())
  {
    UVM_FATAL("PH_BADSYNC","sync() called from a non-domain phase schedule node");
  }
  else if (!target.m_is_domain())
  {
    UVM_FATAL("PH_BADSYNC","sync() called with a non-domain phase schedule node as target");
  }
  else if (phase == nullptr && with_phase != nullptr)
  {
    UVM_FATAL("PH_BADSYNC","sync() called with nullptr phase and non-nullptr with phase");
  }
  else if (phase == nullptr)
  {
    // whole domain sync - traverse this domain schedule from begin to end node and sync each node
    std::map<uvm_phase*, int> visited;
    std::vector<uvm_phase*> queue;
    queue.push_back(this);
    visited[this] = 1;
    while (queue.size())
    {
      uvm_phase* node;
      node = queue.front();       // pop_front
      queue.erase(queue.begin()); // pop_front
      if (node->m_imp != nullptr)
        sync(target, node->m_imp);

      for( m_schedulemapItT it = node->m_successors.begin();
           it != node->m_successors.end();
           it++)
      {
        if (visited.find(it->first) == visited.end()) // not exists
        {
          queue.push_back(it->first);
          visited[it->first] = 1;
        }
      }
    }
  }
  else
  {
    // single phase sync
    // this is a 2-way ('with') sync and we check first in case it is already there
    uvm_phase* from_node;
    uvm_phase* to_node;
    std::vector<int> found_to;
    std::vector<int> found_from;

    if(with_phase == nullptr)
      with_phase = phase;

    from_node = find(phase);
    to_node = target.find(with_phase);

    if(from_node == nullptr || to_node == nullptr)
      return;

    for ( unsigned int i = 0; i < from_node->m_sync.size(); i++)
      if (from_node->m_sync[i] == to_node)
        found_to.push_back(i);

    for ( unsigned int i = 0; i < to_node->m_sync.size(); i++)
      if (to_node->m_sync[i] == from_node)
        found_from.push_back(i);

    if (found_to.size() == 0) from_node->m_sync.push_back(to_node);
    if (found_from.size() == 0) to_node->m_sync.push_back(from_node);
  }
}

//----------------------------------------------------------------------
// member function: unsync
//
//! Remove synchronization between two domains, fully or partially
//!
//!  \p target       - handle of target domain to remove synchronization from
//!  \p phase        - optional single phase in this domain to un-synchronize,
//!                  otherwise unsync all
//!  \p with_phase   - optional different target-domain phase to un-synchronize with,
//!                  otherwise use ~phase~ in the target domain
//----------------------------------------------------------------------

void uvm_phase::unsync( uvm_domain& target,
                        uvm_phase* phase,
                        uvm_phase* with_phase )
{
  if (!this->m_is_domain())
  {
    UVM_FATAL("PH_BADSYNC","unsync() called from a non-domain phase schedule node");
  }
  else if (!target.m_is_domain())
  {
    UVM_FATAL("PH_BADSYNC","unsync() called with a non-domain phase schedule node as target");
  }
  else if (phase == nullptr && with_phase != nullptr)
  {
    UVM_FATAL("PH_BADSYNC","unsync() called with nullptr phase and non-nullptr with phase");
  }
  else if (phase == nullptr)
  {
    // whole domain unsync - traverse this domain schedule from begin to end node and unsync each node
    std::map<uvm_phase*, int> visited;
    std::vector<uvm_phase*> queue;
    queue.push_back(this);
    visited[this] = 1;
    while (queue.size())
    {
      uvm_phase* node;
      node = queue.front();
      queue.erase(queue.begin());

      if (node->m_imp != nullptr)
        unsync(target,node->m_imp);

      for( m_schedulemapItT it = node->m_successors.begin();
           it != node->m_successors.end();
           it++)
      {
        if (visited.find(it->first) == visited.end()) // not exists
        {
          queue.push_back(it->first);
          visited[it->first] = 1;
        }
      }
    }
  }
  else
  {
    // single phase unsync
    // this is a 2-way ('with') sync and we check first in case it is already there
    uvm_phase* from_node;
    uvm_phase* to_node;
    std::vector<phase_listItT> found_to;
    std::vector<phase_listItT> found_from;

    if (with_phase == nullptr)
      with_phase = phase;

    from_node = find(phase);
    to_node = target.find(with_phase);

    if (from_node == nullptr || to_node == nullptr)
      return;

    for ( phase_listItT it = from_node->m_sync.begin();
          it != from_node->m_sync.end();
          it++ )
      if ((*it) == to_node)
        found_to.push_back(it);

    for ( phase_listItT it = to_node->m_sync.begin();
          it != to_node->m_sync.end();
          it++ )
      if ( (*it) == from_node)
        found_from.push_back(it);

    if (found_to.size())
      from_node->m_sync.erase(found_to[0]);

    if (found_from.size())
      to_node->m_sync.erase(found_from[0]);
  }
}

//----------------------------------------------------------------------
// member function: wait_for_state
//
//! Wait until this phase compares with the given state \p state and operand \p op
//! For #UVM_EQ and #UVM_NE operands, several #uvm_phase_states can be
//! supplied by ORing their enum constants, in which case the caller will
//! wait until the phase state is any of #UVM_EQ or none of #UVM_NE the
//! provided states.
//----------------------------------------------------------------------

void uvm_phase::wait_for_state( uvm_phase_state state, uvm_wait_op op)
{
  switch (op)
  {
    case UVM_EQ:  while(!((state&m_state) != 0)) sc_core::wait(m_state_ev); break;
    case UVM_NE:  while(!((state&m_state) == 0)) sc_core::wait(m_state_ev); break;
    case UVM_LT:  while(!(m_state <  state)) sc_core::wait(m_state_ev); break;
    case UVM_LTE: while(!(m_state <= state)) sc_core::wait(m_state_ev); break;
    case UVM_GT:  while(!(m_state >  state)) sc_core::wait(m_state_ev); break;
    case UVM_GTE: while(!(m_state >= state)) sc_core::wait(m_state_ev); break;
  }
}

//----------------------------------------------------------------------
// Group: Jumping
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: jump
//
//! Jump to a specified \p phase. If the destination \P phase is within the current
//! phase schedule, a simple local jump takes place. If the jump-to \p phase is
//! outside of the current schedule then the jump affects other schedules which
//! share the phase.
//----------------------------------------------------------------------

void uvm_phase::jump( const uvm_phase* phase )
{
  const uvm_phase* d;

  if ((m_state < UVM_PHASE_STARTED) ||
      (m_state > UVM_PHASE_READY_TO_END) )
  {
    std::ostringstream str;
    str << "Attempting to jump from phase '" << get_name()
        << "' which is not currently active (current state is "
        << uvm_phase_state_name[m_state]
        << "). The jump will not happen until the phase becomes active.";
    UVM_ERROR("JMPPHIDL", str.str());
  }

  // A jump can be either forward or backwards in the phase graph.
  // If the specified phase (name) is found in the set of predecessors
  // then we are jumping backwards.  If, on the other hand, the phase is in the set
  // of successors then we are jumping forwards.  If neither, then we
  // have an error.
  //
  // If the phase is non-existant and thus we don't know where to jump
  // we have a situation where the only thing to do is to uvm_report_fatal
  // and terminate_phase.  By calling this function the intent was to
  // jump to some other phase. So, continuing in the current phase doesn't
  // make any sense.  And we don't have a valid phase to jump to.  So we're done.

  d = m_find_predecessor(phase, 0);
  if (d == nullptr)
  {
    d = m_find_successor(phase, 0);
    if (d == nullptr)
    {
      std::ostringstream str;
      str << "phase '" << phase->get_name()
          << "' is neither a predecessor or successor of phase '"
          << get_name() << "' or is non-existant, so we cannot jump to it. "
          << "Phase control flow is now undefined so the simulation "
          << "must terminate";
      UVM_FATAL("PH_BADJUMP", str.str());
    }
    else
    {
      UVM_INFO("PH_JUMPF","Jumping forward to phase " + phase->get_name()
        + "...", UVM_DEBUG);
      m_jump_fwd = true;
      m_jump_fwd_ev.notify();
    }
  }
  else
  {
    UVM_INFO("PH_JUMPB","Jumping backward to phase " + phase->get_name()
      + "...", UVM_DEBUG);
    m_jump_bkwd = true;
    m_jump_bkwd_ev.notify();
  }

  m_jump_phase = const_cast<uvm_phase*>(d); // TODO avoid const-cast!
}


//----------------------------------------------------------------------
// member function: get_jump_target
//
//! Return handle to the target phase of the current jump, or null if no jump
//! is in progress. Valid for use during the phase_ended() callback
//----------------------------------------------------------------------

uvm_phase* uvm_phase::get_jump_target() const
{
  return m_jump_phase;
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------
// member function: execute (virtual)
//
//! Implementation defined
//! Provide the required per-component execution flow. Called by traverse
//----------------------------------------------------------------------

void uvm_phase::execute( uvm_component* comp,
                         uvm_phase* phase)
{
}

//----------------------------------------------------------------------
// member function: kill (virtual)
//
//! Implementation defined
//! Provide the required per-component killing procedure. Called by traverse
//----------------------------------------------------------------------

void uvm_phase::kill( uvm_component* comp,
                      uvm_phase* phase)
{
}

//----------------------------------------------------------------------
// member function: traverse (virtual)
//
//! Implementation defined
//! Provide the required component traversal behavior. Called by execute
//----------------------------------------------------------------------

void uvm_phase::traverse( uvm_component* comp,
                          uvm_phase* phase,
                          uvm_phase_state state )
{
}

//----------------------------------------------------------------------
// member function: execute_phase
//
//! Implementation defined
//----------------------------------------------------------------------

void uvm_phase::execute_phase( bool proc )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_root* top = cs->get_root();

  sc_process_handle m_phase_proc;

  // If we got here by jumping forward, we must wait for
  // all its predecessor nodes to be marked DONE.
  // (the next conditional speeds this up)
  // Also, this helps us fast-forward through terminal (end) nodes
  if (proc)
  {
    for( m_schedulemapItT it = m_predecessors.begin();
         it != m_predecessors.end();
         it++)
    {
      while(it->first->m_state != UVM_PHASE_DONE) // TODO check
      {
        sc_core::wait(it->first->m_state_ev);
      }
    }
  }

  // If DONE (by, say, a forward jump), return immed
  if (m_state == UVM_PHASE_DONE)
    return;

  //---------
  // SYNCING:
  //---------
  // Wait for phases with which we have a sync()
  // relationship to be ready. Sync can be 2-way -
  // this additional state avoids deadlock.
  // NOTE: this only works in a process thread
  if (m_sync.size() && proc)
  {
    m_state = UVM_PHASE_SYNCING;
    m_state_ev.notify();

    for( phase_listItT it = m_sync.begin();
         it != m_sync.end();
         it++ )
    {
      while(!( (*it)->m_state >= UVM_PHASE_SYNCING))  // TODO check
      {
        sc_core::wait( (*it)->m_state_ev);
      }
    }
  } // if sync

  m_run_count++;

  if (m_phase_trace())
    UVM_PH_TRACE("PH/TRC/STRT", "Starting phase (type " +
      std::string(uvm_phase_type_name[m_phase_type])+")", this, UVM_LOW);

  // If we're a schedule or domain, then "fake" execution
  if (m_phase_type != UVM_PHASE_NODE)
  {
    m_state = UVM_PHASE_STARTED;

    if (proc)
    {
      m_state_ev.notify();
      sc_core::wait(SC_ZERO_TIME);
    }
    else
      m_state_ev.notify(SC_ZERO_TIME);

    m_state = UVM_PHASE_EXECUTING;
    if (proc)
    {
      m_state_ev.notify();
      sc_core::wait(SC_ZERO_TIME);
    }
    else
      m_state_ev.notify(SC_ZERO_TIME);

  }
  else // start phase node
  {
    //---------
    // STARTED:
    //---------
    m_state = UVM_PHASE_STARTED;

    if (proc)
      m_state_ev.notify();
    else
      m_state_ev.notify(SC_ZERO_TIME);

    m_imp->traverse(top, this, UVM_PHASE_STARTED); // start at the top
    m_ready_to_end_count = 0 ; // reset the ready_to_end count when phase starts

    if (proc)
      sc_core::wait(SC_ZERO_TIME); // LET ANY WAITERS WAKE UP

    if (!proc)
    {
      //-----------
      // EXECUTING: (function phases)
      //-----------
      m_state = UVM_PHASE_EXECUTING;
      m_state_ev.notify(SC_ZERO_TIME);
      // cannot add wait to untimed phase
      // sc_core::wait(SC_ZERO_TIME); // LET ANY WAITERS WAKE UP
      m_imp->traverse(top, this, UVM_PHASE_EXECUTING);
    }
    else // execute process
    {
      m_executing_phases()[this] = true;

      if (m_phase_proc.valid())
      {
        std::ostringstream str;
        str << "Process handle already in use for spawned process '" << m_phase_proc.name() << "'.";
        UVM_FATAL("PHEXEC", str.str());
      }
      else
      {
        uvm_process_phase* process_phase =
          dynamic_cast<uvm_process_phase*>(m_imp);

        if (process_phase == nullptr)
          UVM_FATAL("PHEXEC","Invalid process handle for runtime phase.");

        UVM_PH_TRACE("PH_SPAWN", "", this, UVM_DEBUG);

        m_phase_proc =
          sc_core::sc_spawn(sc_bind(&uvm_phase::m_master_phase_process,
            this, process_phase));
      }

      uvm_wait_for_nba_region(); //Give sequences, etc. a chance to object

      // JUMP
      sc_core::sc_process_handle jump_handle =
        sc_core::sc_spawn(sc_bind(&uvm_phase::m_wait_for_jump, this));

      // WAIT FOR ALL DROPPED
      sc_core::sc_process_handle all_dropped_handle =
        sc_core::sc_spawn(sc_bind(&uvm_phase::m_wait_for_all_dropped, this));

      // TIMEOUT
      sc_core::sc_process_handle timeout_handle =
        sc_core::sc_spawn(sc_bind(&uvm_phase::m_wait_for_timeout, this));

      // wait till any of these processes is terminated
      sc_core::wait( jump_handle.terminated_event() |
            all_dropped_handle.terminated_event() |
            timeout_handle.terminated_event() );

      if (jump_handle.valid())
        jump_handle.kill(SC_INCLUDE_DESCENDANTS);
      if (all_dropped_handle.valid())
        all_dropped_handle.kill(SC_INCLUDE_DESCENDANTS);
      if (timeout_handle.valid())
        timeout_handle.kill(SC_INCLUDE_DESCENDANTS);


      uvm_wait_for_nba_region(); // Give sequences, etc. a chance to object

    } // else execute process

  } // else start phase node

  m_executing_phases().erase(this);

  //---------
  // JUMPING:
  //---------

  // If jump_to() was called then we need to kill all the successor
  // phases which may still be running and then initiate the new
  // phase.  The return is necessary so we don't start new successor
  // phases.  If we are doing a forward jump then we want to set the
  // state of this phase's successors to UVM_PHASE_DONE.  This
  // will let us pretend that all the phases between here and there
  // were executed and completed.  Thus any dependencies will be
  // satisfied preventing deadlocks.
  // GSA TBD insert new jump support

  if (m_phase_type == UVM_PHASE_NODE)
  {
    if( m_jump_fwd || m_jump_bkwd )
    {
      UVM_INFO("PH_JUMP",
        "phase "+get_name()+" (schedule "+get_schedule_name()+", domain "
        + get_domain_name()+ ") is jumping to phase "+ m_jump_phase->get_name(),
        UVM_MEDIUM);

      if (proc)
        sc_core::wait(SC_ZERO_TIME); // LET ANY WAITERS WAKE UP

      // execute 'phase_ended' callbacks
      if (m_phase_trace())
        UVM_PH_TRACE("PH_END","Jumping out of phase", this, UVM_HIGH);

      m_state = UVM_PHASE_ENDED;
      if (proc)
        m_state_ev.notify();
      else
        m_state_ev.notify(SC_ZERO_TIME);

      if (m_imp != nullptr)
         m_imp->traverse(top, this, UVM_PHASE_ENDED);

      if (proc)
        sc_core::wait(SC_ZERO_TIME); // LET ANY WAITERS WAKE UP

      m_state = UVM_PHASE_JUMPING;
      if (proc)
        m_state_ev.notify();
      else
        m_state_ev.notify(SC_ZERO_TIME);

      if (proc && m_phase_proc.valid())
      {
        std::ostringstream str;
        str << "kill master processes '" << m_phase_proc.name()
            << "' for " << get_name() << " phase ";
        UVM_PH_TRACE("PH_KILL", str.str(), this, UVM_DEBUG);
        m_phase_proc.kill(SC_INCLUDE_DESCENDANTS);
        sc_assert(m_phase_proc.terminated());
      }

      if (proc)
        sc_core::wait(SC_ZERO_TIME); // LET ANY WAITERS WAKE UP

      phase_done->clear();

      if(m_jump_fwd)
        clear_successors(UVM_PHASE_DONE, m_jump_phase);

      m_jump_phase->clear_successors();
      m_jump_fwd = false;
      m_jump_bkwd = false;
      m_phase_hopper()->try_put(m_jump_phase);
      m_jump_phase = nullptr;
      return;

    } // jump

    // Wait for predecessors, for function phases only
    if (!proc)
      m_wait_for_pred();

    //-------
    // ENDED:
    //-------
    // execute 'phase_ended' callbacks
    if (m_phase_trace())
      UVM_PH_TRACE("PH_END","ENDING PHASE", this, UVM_HIGH);

    m_state = UVM_PHASE_ENDED;

    if(proc)
      m_state_ev.notify();
    else
      m_state_ev.notify(SC_ZERO_TIME);

    if (m_imp != nullptr)
    m_imp->traverse(top,this,UVM_PHASE_ENDED);

    if (proc)
      sc_core::wait(SC_ZERO_TIME); // LET ANY WAITERS WAKE UP

    //---------
    // CLEANUP:
    //---------
    // kill this phase's threads
    m_state = UVM_PHASE_CLEANUP;

    if (proc)
      m_state_ev.notify();
    else
      m_state_ev.notify(SC_ZERO_TIME);

    if (proc && m_phase_proc.valid())
    {
      std::ostringstream str;
      str << "kill master processes '" << m_phase_proc.name()
          << "' for " << get_name() << " phase ";
      UVM_PH_TRACE("PH_KILL", str.str(), this, UVM_DEBUG);
      m_phase_proc.kill(SC_INCLUDE_DESCENDANTS);
      sc_assert(m_phase_proc.terminated());
    }

    if (proc)
      sc_core::wait(SC_ZERO_TIME); // LET ANY WAITERS WAKE UP

    phase_done->clear();
  } // if m_phase_type == UVM_PHASE_NODE

  //------
  // DONE:
  //------
  if (m_phase_trace())
    UVM_PH_TRACE("PH/TRC/DONE","Completed phase", this, UVM_LOW);
  m_state = UVM_PHASE_DONE;


  if (proc)
  {
    m_state_ev.notify();
    sc_core::wait(SC_ZERO_TIME); // LET ANY WAITERS WAKE UP
  }
  else
    m_state_ev.notify(SC_ZERO_TIME);

  //-----------
  // SCHEDULED:
  //-----------
  // If more successors, schedule them to run now
  if (m_successors.size() == 0)
  {
    top->m_phase_all_done = true;
    top->m_phase_all_done_ev.notify();
  }
  else
  {
    // execute all the successors
    for( m_schedulemapItT it = m_successors.begin();
         it != m_successors.end();
         it++)
    {
      if( it->first->m_state < UVM_PHASE_SCHEDULED)
      {
        it->first->m_state = UVM_PHASE_SCHEDULED;

        if (proc)
        {
          it->first->m_state_ev.notify();
          sc_core::wait(SC_ZERO_TIME); // LET ANY WAITERS WAKE UP
        }
        else
          it->first->m_state_ev.notify(SC_ZERO_TIME);

        m_phase_hopper()->try_put(it->first);
        if (m_phase_trace())
          UVM_PH_TRACE("PH/TRC/SCHEDULED",
            "Scheduled from phase "+get_full_name(), it->first, UVM_LOW);
      }
    }
  }
}


//----------------------------------------------------------------------
// member function: master_phase_process
//
//! Implementation defined
//! This process will be spawned when executing the phases
//----------------------------------------------------------------------

void uvm_phase::m_master_phase_process( uvm_phase* process_phase )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_root* top = cs->get_root();

  //-----------
  // EXECUTING: (process phases)
  //-----------
  m_state = UVM_PHASE_EXECUTING;
  m_state_ev.notify();
  process_phase->traverse(top, this, UVM_PHASE_EXECUTING);

  UVM_INFO("PH_EXEC", "Traversal done. Wait forever...", UVM_DEBUG);
  sc_core::wait(_forever); // stay alive for later kill
}

//----------------------------------------------------------------------
// member function: m_wait_for_jump
//
//! Implementation defined
//! This process will wait till a jump is detected
//----------------------------------------------------------------------

void uvm_phase::m_wait_for_jump()
{
  while (!(m_jump_fwd || m_jump_bkwd))
    sc_core::wait(m_jump_fwd_ev | m_jump_bkwd_ev);

  UVM_PH_TRACE("PH/TRC/EXE/JUMP", "Phase exit on jump request", this, UVM_DEBUG);
}


//----------------------------------------------------------------------
// member function: m_wait_for_all_dropped
//
//! Implementation defined
//! This process will wait till all objections are dropped
//----------------------------------------------------------------------

void uvm_phase::m_wait_for_all_dropped()
{

  bool do_ready_to_end = false; // used for ready_to_end iterations

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_root* top = cs->get_root();

  // OVM semantic: don't end until objection raised or stop request
  if (phase_done->get_objection_total(top) ||
       (m_use_ovm_run_semantic && m_imp->get_name() == "run" ))
  {
    if (!phase_done->m_top_all_dropped)
      phase_done->wait_for(UVM_ALL_DROPPED, top);

    UVM_PH_TRACE("PH/TRC/EXE/ALLDROP","Phase exit all dropped", this, UVM_DEBUG);
  }
  else
     if (m_phase_trace())
       UVM_PH_TRACE("PH/TRC/SKIP","No objections raised, skipping phase", this, UVM_LOW);

  wait_for_self_and_siblings_to_drop() ;
  do_ready_to_end = true;

  //--------------
  // READY_TO_END:
  //--------------

  while (do_ready_to_end)
  {
    uvm_wait_for_nba_region(); // Let all siblings see no objections before traverse might raise another

    UVM_PH_TRACE("PH_READY_TO_END","Phase ready to end", this, UVM_DEBUG);
    m_ready_to_end_count++;
    if (m_phase_trace())
      UVM_PH_TRACE("PH_READY_TO_END_CB","Calling callback ready_to_end", this, UVM_HIGH);

    m_state = UVM_PHASE_READY_TO_END;
    m_state_ev.notify();

    if (m_imp != nullptr)
      m_imp->traverse(top, this, UVM_PHASE_READY_TO_END);

    uvm_wait_for_nba_region(); // Give traverse targets a chance to object
    wait_for_self_and_siblings_to_drop();

    do_ready_to_end = (m_state == UVM_PHASE_EXECUTING)
      && (m_ready_to_end_count < max_ready_to_end_iter) ; //when we don't wait in task above, we drop out of while loop
  }
}

//----------------------------------------------------------------------
// member function: m_wait_for_timeout
//
//! Implementation defined
//! This process will wait for a timeout
//----------------------------------------------------------------------

void uvm_phase::m_wait_for_timeout()
{

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_root* top = cs->get_root();

  if (this->get_name() == "run") // only for run phase
  {

    if (top->phase_timeout == SC_ZERO_TIME)
      //SV was: sc_core::wait(top->phase_timeout != 0);
      while(!(top->phase_timeout != SC_ZERO_TIME))
        sc_core::wait(top->phase_timeout_changed); // TODO - do we really need this?

    if (m_phase_trace())
    {
      std::ostringstream str;
      str << "Starting phase timeout watchdog (timeout = "
        //  << top->phase_timeout
          << " )";
      UVM_PH_TRACE("PH/TRC/TO_WAIT", str.str(), this, UVM_HIGH);
    }

    sc_core::wait(top->phase_timeout); // Actual timeout...

    if (sc_core::sc_time_stamp() == top->phase_timeout) // if timeout reached
    {
      if (m_phase_trace())
        UVM_PH_TRACE("PH/TRC/TIMEOUT", "Phase timeout watchdog expired", this, UVM_LOW);

      for( m_schedulemapItT it = m_executing_phases().begin();
           it != m_executing_phases().end();
           it++)
      {
        if (it->first->phase_done->get_objection_total() > 0)
        {
          if (m_phase_trace())
          {
            std::ostringstream str;
            str << "Phase '" << it->first->get_full_name()
                << "' has outstanding objections:\n"
                << it->first->phase_done->convert2string();
            UVM_PH_TRACE("PH/TRC/TIMEOUT/OBJCTN", str.str(), this, UVM_LOW);
          }
        }
      }
      std::ostringstream str;
      str << "Default timeout of " << top->phase_timeout
          << " hit, indicating a probable testbench issue";
      UVM_FATAL("PH_TIMEOUT", str.str());
    }
    else
    {
      if (m_phase_trace())
        UVM_PH_TRACE("PH/TRC/TIMEOUT", "Phase timeout watchdog expired", this, UVM_LOW);

      for( m_schedulemapItT it = m_executing_phases().begin();
           it != m_executing_phases().end();
           it++)
      {
         if (it->first->phase_done->get_objection_total() > 0)
         {
            if (m_phase_trace())
            {
              std::ostringstream str;
              str << "Phase '" << it->first->get_full_name()
                  << "' has outstanding objections:\n"
                  << it->first->phase_done->convert2string();
              UVM_PH_TRACE("PH/TRC/TIMEOUT/OBJCTN", str.str(), this, UVM_LOW);
            }
         }
      }
      std::ostringstream str;
      str << "Explicit timeout of "
          << top->phase_timeout
          << " hit, indicating a probable testbench issue.";
      UVM_FATAL("PH_TIMEOUT", str.str());
    }

    if (m_phase_trace())
      UVM_PH_TRACE("PH/TRC/EXE/3","Phase exit timeout", this, UVM_DEBUG);
  } // if run phase
  else
  {
    UVM_INFO("PH_EXEC", "Wait forever for phase " + this->get_name(), UVM_DEBUG);
    sc_core::wait(_forever); // never unblock for non-run phase
  }
}


//----------------------------------------------------------------------
// member function: m_find_predecessor
//
//! Implementation defined
//----------------------------------------------------------------------

const uvm_phase* uvm_phase::m_find_predecessor( const uvm_phase* phase,
                                          bool stay_in_scope,
                                          const uvm_phase* orig_phase ) const
{
  if (m_phase_trace())
  {
    std::cout << "  FIND predecessor node '" << phase->get_name()
         << "' (id = " << phase->get_inst_id()
         << ") - checking against " << get_name()
         << " (" << uvm_phase_type_name[m_phase_type]
         << " id = " << get_inst_id();
    if (m_imp != nullptr)
       std::cout << m_imp->get_inst_id();
    std::cout << ")" << std::endl;
  }

  if (phase == nullptr)
    return nullptr;

  if (phase == m_imp || phase == this)
    return this;

  for ( m_schedulemapcItT it = m_predecessors.begin();
        it != m_predecessors.end();
        it++ )
  {
    const uvm_phase* orig;
    orig = (orig_phase == nullptr) ? this : orig_phase;

    if (!stay_in_scope ||
        (it->first->get_schedule() == orig->get_schedule()) ||
        (it->first->get_domain() == orig->get_domain()))
    {
      const uvm_phase* found;
      found = it->first->m_find_predecessor( phase, stay_in_scope, orig );

      if (found != nullptr)
        return found;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------
// member function: m_find_predecessor_by_name
//
//! Implementation defined
//----------------------------------------------------------------------

const uvm_phase* uvm_phase::m_find_predecessor_by_name( const std::string& name,
                                                  bool stay_in_scope,
                                                  const uvm_phase* orig_phase ) const
{
  if (m_phase_trace())
  {
    std::cout << "FIND PRED node '" << name <<"' - checking against "
          << get_name() << " (" << uvm_phase_type_name[m_phase_type]
          << " id = " << get_inst_id();

    if (m_imp != nullptr)
      std::cout << "/" << m_imp->get_inst_id();
    std::cout << ")" << std::endl;
  }

  if (get_name() == name)
    return this;

  for ( m_schedulemapcItT it = m_predecessors.begin();
        it != m_predecessors.end();
        it++ )
  {
    const uvm_phase* orig;
    orig = (orig_phase==nullptr) ? this : orig_phase;

    if (!stay_in_scope ||
        ( it->first->get_schedule() == orig->get_schedule()) ||
        ( it->first->get_domain() == orig->get_domain()))
    {
      const uvm_phase* found;
      found = it->first->m_find_predecessor_by_name( name, stay_in_scope, orig );

      if (found != nullptr)
        return found;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------
// member function: m_find_successor
//
//! Implementation defined
//----------------------------------------------------------------------

const uvm_phase* uvm_phase::m_find_successor( const uvm_phase* phase,
                                        bool stay_in_scope,
                                        const uvm_phase* orig_phase ) const
{
  if (m_phase_trace())
  {
    std::cout << "  FIND SUCC node '" << phase->get_name()
         << "' (id = " << phase->get_inst_id()
         << ") - checking against " << get_name()
         << " (" << uvm_phase_type_name[m_phase_type]
         << " id = " << get_inst_id();
    if (m_imp != nullptr)
      std::cout << "/" << m_imp->get_inst_id();
    std::cout << ")" << std::endl;
  }

  if (phase == nullptr)
    return nullptr;

  if (phase == m_imp || phase == this)
    return this;

  for( m_schedulemapcItT it = m_successors.begin();
       it != m_successors.end();
       it++)
  {
    const uvm_phase* orig;
    orig = (orig_phase == nullptr) ? this : orig_phase;

    if (!stay_in_scope ||
        (it->first->get_schedule() == orig->get_schedule()) ||
        (it->first->get_domain() == orig->get_domain()))
    {
      const uvm_phase* found;
      found = it->first->m_find_successor( phase, stay_in_scope, orig );

      if (found != nullptr)
        return found;
    }
  }
  return nullptr;
}


//----------------------------------------------------------------------
// member function: m_find_successor_by_name
//
//! Implementation defined
//----------------------------------------------------------------------

const uvm_phase* uvm_phase::m_find_successor_by_name( const std::string& name,
                                                bool stay_in_scope,
                                                const uvm_phase* orig_phase ) const
{
  if (m_phase_trace())
  {
    std::cout << "  FIND SUCC node '" << name << "' - checking against "
         << get_name() << " (" << uvm_phase_type_name[m_phase_type]
         << " id=" << get_inst_id();
    if (m_imp != nullptr)
      std::cout << "/" << m_imp->get_inst_id();
    std::cout << ")" << std::endl;
  }

  if (get_name() == name)
    return this;

  for( m_schedulemapcItT it = m_successors.begin();
       it != m_successors.end();
       it++)
  {
    const uvm_phase* orig;
    orig = (orig_phase == nullptr) ? this : orig_phase;

    if (!stay_in_scope ||
        (it->first->get_schedule() == orig->get_schedule()) ||
        (it->first->get_domain() == orig->get_domain()))
    {
      const uvm_phase* found;
      found = it->first->m_find_successor_by_name( name, stay_in_scope, orig );

      if (found != nullptr)
        return found;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------
// member function: m_register_phases
//
//! Implementation defined
//! This member function registers all phases in a lookup table, so
//! we can simply access each individual phase by using its string name.
//----------------------------------------------------------------------

void uvm_phase::m_register_phases()
{
  uvm_phase* ph = uvm_domain::get_common_domain();
  uvm_phase* ph_next = nullptr;

  // register all phase nodes
  do
  {
    for( m_schedulemapItT it = ph->m_successors.begin();
         it != ph->m_successors.end();
         it++)
    {
      ph_next = it->first;
      std::string s = it->first->get_name();
      m_phase_nodes(s, ph_next);
      //std::cout << "phase registered : " << ph_next->get_name() << std::endl;
    }
    ph = ph_next;
  }
  while (ph != nullptr && ph->m_successors.size() != 0);
}


//----------------------------------------------------------------------
// member function: m_prerun_phases (static)
//
//! Implementation defined
//! This member function calls the prerun phases build and connect
//----------------------------------------------------------------------

void uvm_phase::m_prerun_phases()
{
  m_run_single_phase("build");
  m_run_single_phase("connect");
}

//----------------------------------------------------------------------
// member function: m_run_single_phase
//
//! Implementation defined
//! This member function runs a single phase. Use to call the build,
//! elaborate and end_of_elaboration call.
//----------------------------------------------------------------------

void uvm_phase::m_run_single_phase( const std::string& phase_name )
{
  uvm_phase* ph = m_phase_nodes(phase_name);

  if (ph == nullptr)
  {
    UVM_WARNING("PH/RUN", "Phase with name '" + phase_name + "' not found. Skipped.");
    return;
  }

  ph->execute_phase(false); // execute phase as function call
}

//----------------------------------------------------------------------
// member function: m_run_phases_process
//
//! Implementation defined.
//! This member function defines the spawned thread to execute
//! the run phases. Is called from the uvm_root::start_of_simulation
//! callback
//----------------------------------------------------------------------

void uvm_phase::m_run_phases_process()
{
  sc_core::sc_process_handle exec_run_proc =
    sc_core::sc_spawn(sc_bind(&uvm_phase::m_run_phases),
      "uvm_phase_run_process");
}

//----------------------------------------------------------------------
// member function: m_run_phases
//
//! Implementation defined
//! This member function contains the top-level process that controls
//! the run phases
//----------------------------------------------------------------------

void uvm_phase::m_run_phases()
{
  uvm_phase* phase;

  // only put start_of_simulation phase in the queue to get started
  m_phase_hopper()->empty();
  int queue_ok = m_phase_hopper()->try_put(m_phase_nodes("start_of_simulation"));

  if (!queue_ok)
  {
    UVM_FATAL("PH_RUN", "Unable to launch UVM-SystemC phasing. Simulation will stop.");
    return;
  }

  do
  {
    m_phase_hopper()->get(phase);

    uvm_process_phase* process_phase =
      dynamic_cast<uvm_process_phase*>(phase->m_imp);

    bool valid_proc = ((process_phase != nullptr) || (phase->m_phase_type != UVM_PHASE_NODE));

    //std::string s = "uvm_exec_phase_" + std::string(sc_core::sc_gen_unique_name( this->get_full_name_under().c_str() ));

    if (!valid_proc) {
      phase->execute_phase( valid_proc ); // untimed function call
    }
    else
    {
      sc_core::sc_process_handle exec_run_proc =
        sc_core::sc_spawn(sc_bind(&uvm_phase::execute_phase, phase, valid_proc )
//         , s.c_str()
       );
    }
  }
  while (phase != nullptr && phase->m_successors.size() != 0 );

  // Phasing done, so we can wrap-up and stop the simulation

  //  regressions check point
  if( std::getenv( "UVMSC_REGRESSION" ) != 0 )
    std::cerr << "UVMSC_REGRESSION_ENDED" << std::endl;

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  sc_core::wait(cs->get_root()->m_phase_all_done_ev);

  //TODO clean up after ourselves
  //phase_runner_proc.kill();

  auto rs = uvm_report_server::get_server();
  rs->report_summarize();

  // soft stop (pause) simulation; return sc_start
  if ( sc_core::sc_get_status() == sc_core::SC_RUNNING )
    sc_core::sc_pause();
}

//----------------------------------------------------------------------
// member function: wait_for_self_and_siblings_to_drop
//
//! Implementation defined
//! This member function loops until this phase instance and all its siblings, either
//! sync'd or sharing a common successor, have all objections dropped.
//----------------------------------------------------------------------

void uvm_phase::wait_for_self_and_siblings_to_drop()
{
  bool need_to_check_all = true;
  std::map<uvm_phase*, bool> siblings;

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_root* top = cs->get_root();

  get_predecessors_for_successors(siblings);

  for( phase_listItT it = m_sync.begin();
        it != m_sync.end();
        it++ )
    siblings[*it] = true;


  while (need_to_check_all)
  {
    need_to_check_all = 0 ; //if all are dropped, we won't need to do this again

    // wait for own objections to drop
    if (phase_done->get_objection_total(top) != 0)
    {
      m_state = UVM_PHASE_EXECUTING ;
      m_state_ev.notify();
      phase_done->wait_for(UVM_ALL_DROPPED, top);
      need_to_check_all = true;
    }

    // now wait for siblings to drop
    for( m_schedulemapItT it = siblings.begin();
          it != siblings.end();
          it++ )
    {
      it->first->wait_for_state(UVM_PHASE_EXECUTING, UVM_GTE); // sibling must be at least executing
      if (it->first->phase_done->get_objection_total(top) != 0)
      {
        m_state = UVM_PHASE_EXECUTING ;
        m_state_ev.notify();
        it->first->phase_done->wait_for(UVM_ALL_DROPPED, top); // sibling must drop any objection
        need_to_check_all = true;
      }
    }
  }
}

//----------------------------------------------------------------------
// member function: clear
//
//! Implementation defined
//! for internal graph maintenance after a forward jump
//----------------------------------------------------------------------

void uvm_phase::clear_phase( uvm_phase_state state )
{
  m_state = state;
  m_state_ev.notify();
  //m_phase_proc = nullptr;
  phase_done->clear(this);
}

//----------------------------------------------------------------------
// member function: clear_successors
//
//! Implementation defined
//! for internal graph maintenance after a forward jump
//! - called only by execute_phase()
//! - depth-first traversal of the DAG, calling clear() on each node
//! - do not clear the end phase or beyond
//----------------------------------------------------------------------

void uvm_phase::clear_successors( uvm_phase_state state,
                                  uvm_phase* end_state)
{
  if(this == end_state)
    return;
  clear_phase(state);
  for( m_schedulemapItT it = m_successors.begin();
       it != m_successors.end();
       it++)
    it->first->clear_successors(state, end_state);
}


//----------------------------------------------------------------------
// member function: get_predecessors_for_successors
//
//! Implementation defined
//----------------------------------------------------------------------

void uvm_phase::get_predecessors_for_successors(
  std::map<uvm_phase*, bool>& pred_of_succ )
{
  bool done = false;

  std::map<uvm_phase*, bool> successors;

  // get all successors
  for( m_schedulemapItT it = m_successors.begin();
       it != m_successors.end();
       it++)
    successors[it->first] = true;

  // replace TERMINAL or SCHEDULE nodes with their successors
  do
  {
    done = true;

    for( m_schedulemapItT it = successors.begin();
         it != successors.end();
         it++)
    {
      uvm_phase* succ = it->first;
      if (succ->get_phase_type() != UVM_PHASE_NODE)
      {
        for( m_schedulemapItT lit = succ->m_successors.begin();
             lit != succ->m_successors.end();
             lit++)
        {
            uvm_phase* next_succ = lit->first;
            successors[next_succ] = true;
            //std::cout << "replace " << succ->get_name() << " with : " << next_succ->get_name() << std::endl;
        }
        done = false;
        successors.erase(succ);
        break; // restart for-loop due to possible invalidated iterator
      }
    }
  }
  while(!done);

  // get all predecessors to these successors
  for( m_schedulemapItT it = successors.begin();
       it != successors.end();
       it++)
    for( m_schedulemapItT lit = it->first->m_predecessors.begin();
         lit != it->first->m_predecessors.end();
         lit++)
      pred_of_succ[lit->first] = true;

  // replace any terminal nodes with their predecessors, recursively.
  // we are only interested in "real" phase nodes
  do
  {
    done = true;

    for( m_schedulemapItT it = pred_of_succ.begin();
         it != pred_of_succ.end();
         it++ )
    {
      uvm_phase* pred = it->first;

      if (pred->get_phase_type() != UVM_PHASE_NODE)
      {
        for( m_schedulemapItT lit = pred->m_predecessors.begin();
             lit != pred->m_predecessors.end();
             lit++ )
        {
          uvm_phase* pred_next = lit->first;
          pred_of_succ[pred_next] = true;
          //std::cout << "replace " << pred->get_name() << " with : " << pred_next->get_name() << std::endl;
        }
        done = false;
        pred_of_succ.erase(pred);
        break; // restart for-loop due to possible invalidated iterator
      }
    }
  }
  while (!done);

  // remove ourselves from the list
  pred_of_succ.erase(this);
}


//----------------------------------------------------------------------
// member function: m_wait_for_pred
//
//! Implementation defined
//----------------------------------------------------------------------

void uvm_phase::m_wait_for_pred()
{
  if(!(m_jump_fwd || m_jump_bkwd))
  {
    std::map<uvm_phase*, bool> pred_of_succ;

    get_predecessors_for_successors(pred_of_succ);

    // wait for predecessors to successors (real phase nodes, not terminals)
    // mostly debug msgs
    for( m_schedulemapItT it = pred_of_succ.begin();
         it != pred_of_succ.end();
         it++ )
    {
      if (m_phase_trace())
      {
        std::ostringstream str;
        str << "Waiting for phase '"
            << it->first->get_name()
            << "' (" << it->first->get_inst_id()
            << ") to be READY_TO_END. Current state is "
            << it->first->m_state;
        UVM_PH_TRACE("PH/TRC/WAIT_PRED_OF_SUCC",str.str(), this, UVM_HIGH);
      }

      it->first->wait_for_state(UVM_PHASE_READY_TO_END, UVM_GTE);

      if (m_phase_trace())
      {
        std::ostringstream str;
        str << "Phase '" << it->first->get_name()
            << "' (" << it->first->get_inst_id()
            << ") is now READY_TO_END. Releasing phase";
        UVM_PH_TRACE("PH/TRC/WAIT_PRED_OF_SUCC",str.str(), this, UVM_HIGH);
      }
    }

    if (m_phase_trace())
    {
      if (pred_of_succ.size() != 0 )
      {
        std::string s = "( ";

        for( m_schedulemapItT it = pred_of_succ.begin();
             it != pred_of_succ.end();
             it++ )
          s = s + it->first->get_full_name() + " ";
        s = s+")";
        UVM_PH_TRACE("PH/TRC/WAIT_PRED_OF_SUCC",
          "*** All predecessors to successors " + s +
          " in READY_TO_END state, so ending phase ***",
          this, UVM_HIGH);
      }
      else
        UVM_PH_TRACE("PH/TRC/WAIT_PRED_OF_SUCC",
          "*** No predecessors to successors other than myself, so ending phase ***",
          this, UVM_HIGH);
    }

  } // if jump
}

//----------------------------------------------------------------------
// member function: m_get_transitive_children
//
//! Implementation defined
//----------------------------------------------------------------------

void uvm_phase::m_get_transitive_children(std::vector<uvm_phase*>& phases)
{
  for(m_schedulemapItT it = m_successors.begin();
      it != m_successors.end();
      it++)
  {
      phases.push_back(it->first);
      it->first->m_get_transitive_children(phases);
  }
}

//----------------------------------------------------------------------
// member function: m_is_domain
//
//! Implementation defined
//----------------------------------------------------------------------

bool uvm_phase::m_is_domain()
{
  return (m_phase_type == UVM_PHASE_DOMAIN);
}

//----------------------------------------------------------------------
// member function: m_executing_phases
//
//! Implementation defined
//----------------------------------------------------------------------

std::map<uvm_phase*, bool>& uvm_phase::m_executing_phases()
{
  // Refactored from the former function-local executing_phases global to uvm_coreservice_t.
  return uvm_coreservice_t::get()->get_uvm_phase_executing_phases();
}

//----------------------------------------------------------------------
// member function: m_executing_phases
//
//! Implementation defined
//----------------------------------------------------------------------

uvm_phase* uvm_phase::m_phase_nodes(const std::string& name, uvm_phase* phase)
{
  // Refactored from the former function-local phase_nodes global to uvm_coreservice_t.
  std::map<std::string, uvm_phase*>& phase_nodes = uvm_coreservice_t::get()->get_uvm_phase_phase_nodes();

  if (phase) // add to map if phase is specified
    phase_nodes[name] = phase;

  return phase_nodes[name];
}

//----------------------------------------------------------------------
// member function: get_full_name_under (private)
//
//! Implementation defined
//! Returns the full path from the enclosing domain down to this node.
//! Instead of the '.' it uses the underscore
//----------------------------------------------------------------------

const std::string uvm_phase::get_full_name_under() const
{
  std::string dom, sch;
  std::string full_name;

  if (m_phase_type == UVM_PHASE_IMP)
    return get_name();

  full_name = get_domain_name();

  sch = get_schedule_name();
  if (!sch.empty())
    full_name = full_name + "_" + sch;

  if (m_phase_type != UVM_PHASE_DOMAIN && m_phase_type != UVM_PHASE_SCHEDULE)
    full_name = full_name + "_" + get_name();

  return full_name;
}

//////////////

} /* namespace uvm */
