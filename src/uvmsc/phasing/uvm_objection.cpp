//----------------------------------------------------------------------
//   Copyright 2014 Université Pierre et Marie Curie, Paris
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
//   Copyright 2017 Intel Corp.
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

#include <iostream>
#include <iomanip>
#include <sstream>

#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/base/uvm_root.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/seq/uvm_sequencer_base.h"
#include "uvmsc/seq/uvm_sequence_base.h"
#include "uvmsc/seq/uvm_sequence_item.h"
#include "uvmsc/report/uvm_report_object.h"
#include "uvmsc/macros/uvm_message_defines.h"
#include "uvmsc/phasing/uvm_objection.h"

using namespace sc_core;

namespace uvm {

//----------------------------------------------------------------------
// Initialize static members
//----------------------------------------------------------------------

//none

//----------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------

uvm_objection::uvm_objection() : uvm_object( sc_core::sc_gen_unique_name("objection") )
{
  m_objection_init();
}

uvm_objection::uvm_objection( const std::string& name ) : uvm_object(name)
{
  m_objection_init();
}

//----------------------------------------------------------------------
// member function: clear
//
//! Immediately clears the objection state. All counts are cleared and the
//! any processes waiting on a call to wait_for(UVM_ALL_DROPPED, uvm_top)
//! are released.
//! The caller, if a uvm_object-based object, should pass its 'this' handle
//! to the \p obj argument to document who cleared the objection.
//! Any drain_times set by the user are not effected.
//----------------------------------------------------------------------

void uvm_objection::clear( uvm_object* obj )
{
  std::string name;
  m_scheduled_list_itt idx_it;
  unsigned int idx;

  if (obj == nullptr)
    obj = m_top;

  name = obj->get_full_name();

  if (name.empty())
    name = "uvm_top";
  else
    name = obj->get_full_name();

  if (!m_top_all_dropped && get_objection_total(m_top))
    UVM_WARNING("OBJTN_CLEAR","Object '" + name +
          "' cleared objection counts for "+get_name());

  // TODO - Should there be a warning if there are outstanding objections?
  m_source_count.clear();
  m_total_count.clear();

  // Remove any scheduled drains from the static queue
  idx_it = m_scheduled_list().begin();
  idx = 0;
  while (idx < m_scheduled_list().size())
  {
    if (m_scheduled_list()[idx]->objection == this)
    {
      m_scheduled_list()[idx]->clear();
      m_context_pool.push_back(m_scheduled_list()[idx]);
      idx_it = m_scheduled_list().erase(idx_it);
      m_scheduled_list_changed().notify();
    }
    else
    {
      idx++;
      idx_it++;
    }
  }

  // Scheduled contexts and m_forked_lists have duplicate
  // entries... clear out one, free the other.
  m_scheduled_contexts.clear();
  while (m_forked_list.size())
  {
    m_forked_list[0]->clear();
    m_context_pool.push_back(m_forked_list[0]);
    m_forked_list.erase(m_forked_list.begin()); // pop_front
  }

  m_objection_map_itt it = m_forked_contexts.begin();

  // running drains have a context and a process
  for (unsigned int i = 0 ; i < m_forked_contexts.size();
      i++ )
  {
    m_drain_proc[it->first].kill();
    m_drain_proc.erase(it->first); // delete process from list

    it->second->clear();
    m_context_pool.push_back(it->second);
    m_forked_contexts.erase(it->first);
  }

  m_top_all_dropped = false;
  m_cleared = true;

  if (m_events.find(m_top) != m_events.end()) // if exists
    m_events[m_top]->all_dropped.notify();
}

//----------------------------------------------------------------------
// member function: trace_mode
//
//! Set or get the trace mode for the objection object. If no
//! argument is specified (or an argument other than 0 or 1)
//! the current trace mode is unaffected. A trace_mode of
//! 0 turns tracing off. A trace mode of 1 turns tracing on.
//! The return value is the mode prior to being reset.
//----------------------------------------------------------------------

bool uvm_objection::trace_mode( int mode )
{
  bool old_trace_mode = m_trace_mode;
  if(mode == 0)
    m_trace_mode = 0;
  else
    if(mode == 1)
      m_trace_mode = true;

  return old_trace_mode;
}

//----------------------------------------------------------------------
// Group: Objection Control
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: m_set_hier_mode
//
//! Hierarchical mode only needs to be set for intermediate components, not
//! for uvm_root or a leaf component.
//----------------------------------------------------------------------

void uvm_objection::m_set_hier_mode( uvm_object* obj )
{
  uvm_component* comp;
  if((m_hier_mode == true) || (obj == m_top))
  {
    // Don't set if already set or the object is uvm_top.
    return;
  }

  comp = dynamic_cast<uvm_component*>(obj);
  if(comp != nullptr)
  {
    // Don't set if object is a leaf.
    if(comp->get_num_children() == 0)
      return;
  }
  else
    return; // Don't set if object is a non-component.

  // restore counts on non-source nodes
  m_total_count.clear();

  for( m_count_mapitT it = m_source_count.begin();
       it != m_source_count.end();
       it++ )
  {
    uvm_object* theobj = obj;
    int count = it->second;
    do
    {
      if (m_total_count.find(theobj) != m_total_count.end()) // exists
        m_total_count[theobj] += count;
      else
        m_total_count[theobj] = count;

      theobj = m_get_parent(theobj);
      m_total_count_ev.notify(); // let other processes take action
    }
    while (theobj != nullptr);
  }

  m_hier_mode = true;
}

//----------------------------------------------------------------------
// member function: raise_objection
//
//! Raises the number of objections for the source object by count, which
//! defaults to 1.  The object is usually the this handle of the caller.
//! If object is not specified or null, the implicit top-level component,
//! uvm_root, is chosen.
//----------------------------------------------------------------------

void uvm_objection::raise_objection( uvm_object* obj,
                                     const std::string& description,
                                     int count )
{
  if(obj == nullptr)
    obj = m_top;

  m_cleared = false;
  m_top_all_dropped = false;

  m_raise(obj, obj, description, count);
}

//----------------------------------------------------------------------
// member function: drop_objection
//
//! Drops the number of objections for the source object by count, which
//! defaults to 1.  The object is usually the this handle of the caller.
//! If object is not specified or null, the implicit top-level component,
//! #uvm_root, is chosen.
//----------------------------------------------------------------------

void uvm_objection::drop_objection( uvm_object* obj,
                                    const std::string& description,
                                    int count )
{
  if(obj == nullptr)
    obj = m_top;

  m_drop(obj, obj, description, count, 0);
}


//----------------------------------------------------------------------
// member function: set_drain_time
//
//! Sets the drain time on the given \p object to \p drain.
//!
//! The drain time is the amount of time to wait once all objections have
//! been dropped before calling the all_dropped callback and propagating
//! the objection to the parent.
//!
//! If a new objection for this ~object~ or any of its descendents is raised
//! during the drain time or during execution of the all_dropped callbacks,
//! the drain_time/all_dropped execution is terminated.
//----------------------------------------------------------------------

void uvm_objection::set_drain_time( uvm_object* obj, const sc_time& drain )
{
  if (obj == nullptr)
    obj = m_top;

  m_drain_time[obj] = drain;

  m_set_hier_mode(obj);
}

//----------------------------------------------------------------------
// Group: Callback Hooks
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: raised
//
//! Objection callback that is called when a #raise_objection has reached \p obj.
//! The default implementation calls uvm_component::raised.
//----------------------------------------------------------------------

void uvm_objection::raised( uvm_object* obj,
                            uvm_object* source_obj,
                            const std::string& description,
                            int count)
{
  uvm_component* comp = dynamic_cast<uvm_component*>(obj);
  if (comp != nullptr)
    comp->raised(this, source_obj, description, count);

  if (m_events.find(obj) != m_events.end()) //exists
    m_events[obj]->raised.notify();
}


//----------------------------------------------------------------------
// member function: dropped
//
//! Objection callback that is called when a #drop_objection has reached \p obj.
//! The default implementation calls uvm_component::dropped.
//----------------------------------------------------------------------

void uvm_objection::dropped( uvm_object* obj,
                             uvm_object* source_obj,
                             const std::string& description,
                             int count)
{
  uvm_component* comp = dynamic_cast<uvm_component*>(obj);
  if(comp != nullptr)
    comp->dropped(this, source_obj, description, count);

  if (m_events.find(obj) != m_events.end()) // if exists
    m_events[obj]->dropped.notify();
}

//----------------------------------------------------------------------
// member function: all_dropped
//
//! Objection callback that is called when a #drop_objection has reached \p obj,
//! and the total count for \p obj goes to zero. This callback is executed
//! after the drain time associated with \p obj. The default implementation
//! calls uvm_component::all_dropped.
//----------------------------------------------------------------------

void uvm_objection::all_dropped( uvm_object* obj,
                                 uvm_object* source_obj,
                                 const std::string& description,
                                 int count )
{
  uvm_component* comp;
  comp = dynamic_cast<uvm_component*>(obj);
  if(comp != nullptr)
    comp->all_dropped(this, source_obj, description, count);

  if (m_events.find(obj) != m_events.end()) // if exists
    m_events[obj]->all_dropped.notify();

  if (obj == m_top)
    m_top_all_dropped = true;
}

//----------------------------------------------------------------------
// Group: Objection Status
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: get_objectors
//
//! Returns the current list of objecting objects (objects that
//! raised an objection but have not dropped it).
//----------------------------------------------------------------------

void uvm_objection::get_objectors( std::vector<uvm_object*>& objlist ) const
{
  objlist.clear();
  for( m_count_mapcitT it = m_source_count.begin();
       it != m_source_count.end();
       it++)
    objlist.push_back(it->first);
}

//----------------------------------------------------------------------
// member function: wait_for
//
//! Waits for the raised, dropped, or all_dropped \p event to occur in
//! the given \p obj. The task returns after all corresponding callbacks
//! for that event have been executed.
//----------------------------------------------------------------------

void uvm_objection::wait_for( uvm_objection_event objt_event, uvm_object* obj )
{
  if (obj == nullptr)
    obj = m_top;

  if ( m_events.find(obj) == m_events.end()) // not exists
    m_events[obj] = new uvm_objection_events();

  m_events[obj]->waiters++;

  switch(objt_event)
  {
    case UVM_RAISED:      sc_core::wait(m_events[obj]->raised); m_events[obj]->raised.cancel(); break;
    case UVM_DROPPED:     sc_core::wait(m_events[obj]->dropped); m_events[obj]->dropped.cancel(); break;
    case UVM_ALL_DROPPED: sc_core::wait(m_events[obj]->all_dropped); m_events[obj]->all_dropped.cancel(); break;
  }

  m_events[obj]->waiters--;

  if (m_events[obj]->waiters == 0)
    m_events.erase(obj);
}

//----------------------------------------------------------------------
// member function: get_objection_count
//
//! Returns the current number of objections raised by the given object.
//----------------------------------------------------------------------

int uvm_objection::get_objection_count( uvm_object* obj ) const
{
  if (obj == nullptr)
    obj = m_top;

  if (m_source_count.find(obj) == m_source_count.end() )
    return 0;

  return m_source_count.find(obj)->second;
}


//----------------------------------------------------------------------
// member function: get_objection_total
//
//! Returns the current number of objections raised by the given \p object
//! and all descendants.
//----------------------------------------------------------------------

int uvm_objection::get_objection_total( uvm_object* obj ) const
{
  uvm_component* comp;
  std::string child;
  int objection_total;

  if (obj == nullptr)
    obj = m_top;

  if (m_total_count.find(obj) == m_total_count.end() ) // if not found
    return 0;

  if (m_hier_mode)
    return m_total_count.find(obj)->second;
  else
  {
    comp = dynamic_cast<uvm_component*>(obj);
    if (comp != nullptr)
    {
      if (m_source_count.find(obj) == m_source_count.end()) // if not exists
        objection_total = 0;
      else
        objection_total = m_source_count.find(obj)->second;

      if (comp->get_first_child(child))
      do
        objection_total += get_objection_total(comp->get_child(child));
      while (comp->get_next_child(child));
    }
    else
      return m_total_count.find(obj)->second;
  }
  return objection_total;
}

//----------------------------------------------------------------------
// member function: get_drain_time
//
//! Returns the current drain time set for the given \p object (default: 0 ns).
//----------------------------------------------------------------------

const sc_time uvm_objection::get_drain_time( uvm_object* obj ) const
{
  if (obj == nullptr)
    obj = m_top;

  if (m_drain_time.find(obj) == m_drain_time.end()) // not exists
    return SC_ZERO_TIME;

  return m_drain_time.find(obj)->second;
}

//----------------------------------------------------------------------
// member function: display_objections
//
//! Displays objection information about the given \p object. If \p object is
//! not specified or nullptr, the implicit top-level component, #uvm_root, is
//! chosen. The argument \p show_header allows control of whether a header is
//! output.
//----------------------------------------------------------------------

void uvm_objection::display_objections( uvm_object* obj,
                                        bool show_header ) const
{
  std::string s = m_display_objections(obj,show_header);
  UVM_INFO("UVM/OBJ/DISPLAY", s, UVM_NONE);
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// member function: m_objection_init
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_objection::m_objection_init()
{
  m_source_count.clear();
  m_total_count.clear();
  m_drain_time.clear();

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  m_top = cs->get_root();

  m_events.clear();
  m_cleared = false;
  m_trace_mode = false;
  m_hier_mode = true;

  m_ctxt = nullptr;
  m_context_pool.clear();
  m_scheduled_list().clear();

  m_top_all_dropped = false;

  m_objections().push_back(this);
}

//----------------------------------------------------------------------
// member function: m_objections
//
// Implementation-defined member function
//----------------------------------------------------------------------

uvm_objection::m_objections_list& uvm_objection::m_objections()
{
  static m_objections_list list;
  return list;
}

//----------------------------------------------------------------------
// member function: m_scheduled_list
//
// Implementation-defined member function
//----------------------------------------------------------------------

std::vector<uvm_objection_context_object*>& uvm_objection::m_scheduled_list()
{
  static std::vector<uvm_objection_context_object*> list;
  return list;
}

//----------------------------------------------------------------------
// member function: m_scheduled_list_changed
//
// Implementation-defined member function
//----------------------------------------------------------------------

sc_event& uvm_objection::m_scheduled_list_changed()
{
  static sc_event list_changed;
  return list_changed;
}

//----------------------------------------------------------------------
// member function: m_raise
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_objection::m_raise( uvm_object* obj,
                             uvm_object* source_obj,
                             const std::string& description,
                             int count )
{
  if (m_total_count.find(obj) != m_total_count.end()) // if exists
    m_total_count[obj] += count;
  else
    m_total_count[obj] = count;

  m_total_count_ev.notify();

  if (source_obj == obj)
  {
    if (m_source_count.find(obj) != m_source_count.end() ) // if exists
      m_source_count[obj] += count;
    else
      m_source_count[obj] = count;
  }

  if (m_trace_mode)
    m_report(obj, source_obj, description, count, "raised");

  raised(obj, source_obj, description, count);

  // Handle any outstanding drains...

  // First go through the scheduled list
  uvm_objection_context_object* ctxt = nullptr;
  unsigned int idx = 0;
  m_scheduled_list_itt idx_it = m_scheduled_list().begin();

  while (idx < m_scheduled_list().size())
  {
    if ((m_scheduled_list()[idx]->obj == obj) &&
        (m_scheduled_list()[idx]->objection == this))
    {
      // Caught it before the drain was forked
      ctxt = m_scheduled_list()[idx];
      m_scheduled_list().erase(idx_it);
      m_scheduled_list_changed().notify();
      break;
    }
    idx++; idx_it++;
  }

  // If it's not there, go through the forked list
  if (ctxt == nullptr)
  {
    idx = 0;
    idx_it = m_forked_list.begin();

    while (idx < m_forked_list.size())
    {
      if (m_forked_list[idx]->obj == obj)
      {
        // Caught it after the drain was forked,
        // but before the fork started
        ctxt = m_forked_list[idx];
        m_forked_list.erase(idx_it);
        m_scheduled_contexts.erase(ctxt->obj);
        break;
      }
      idx++; idx_it++;
    }
  }

  // If it's not there, go through the forked contexts
  if (ctxt == nullptr)
  {
    if (m_forked_contexts.find(obj) != m_forked_contexts.end()) // exists
    {
      // Caught it with the forked drain running
      ctxt = m_forked_contexts[obj];
      m_forked_contexts.erase(obj);

      // Kill the drain
      m_drain_proc[obj].kill();
      m_drain_proc.erase(obj);
    }
  }

  if (ctxt == nullptr)
  {
    // If there were no drains, just propagate as usual

    if (!m_hier_mode && obj != m_top)
      m_raise(m_top, source_obj, description, count);
    else
      if (obj != m_top)
        m_propagate(obj, source_obj, description, count, true, 0);
  }
  else
  {
    // Otherwise we need to determine what exactly happened
    int diff_count;

    // Determine the diff count, if it's positive, then we're
    // looking at a 'raise' total, if it's negative, then
    // we're looking at a 'drop', but not down to 0.  If it's
    // a '0', that means that there is no change in the total.
    diff_count = count - ctxt->count;

    if (diff_count != 0)
    {
      // Something changed
      if (diff_count > 0)
      {
        // we're looking at an increase in the total
        if (!m_hier_mode && obj != m_top)
          m_raise(m_top, source_obj, description, diff_count);
        else
          if (obj != m_top)
            m_propagate(obj, source_obj, description, diff_count, true, 0);
      }
      else
      {
        // we're looking at a decrease in the total
        // The count field is always positive...
        diff_count = -diff_count;
        if (!m_hier_mode && obj != m_top)
          m_drop(m_top, source_obj, description, diff_count);
        else
          if (obj != m_top)
            m_propagate(obj, source_obj, description, diff_count, false, 0);
      }
    }

    // Cleanup
    ctxt->clear();
    m_context_pool.push_back(ctxt);
  }
}

//----------------------------------------------------------------------
// member function: m_drop
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_objection::m_drop( uvm_object* obj,
                            uvm_object* source_obj,
                            const std::string& description,
                            int count,
                            int in_top_thread )
{
  if ( (m_total_count.find(obj) == m_total_count.end() ) ||
       (count > m_total_count[obj]) )
  {
    if(m_cleared)
      return;

    std::ostringstream str;
    str << "Object " << obj->get_full_name()
        << " attempted to drop objection: '"
        << this->get_name()
        << "' count below zero!";
    UVM_FATAL("OBJTN_ZERO", str.str());
    return;
  }

  if (obj == source_obj)
  {
    if ( m_source_count.find(obj) == m_source_count.end() ||
          (count > m_source_count[obj]))
    {
      if(m_cleared)
        return;

      std::ostringstream str;
      str << "Object "
          << obj->get_full_name()
          << " attempted to drop objection: '"
          << this->get_name()
          << "' count below zero!";
      UVM_FATAL("OBJTN_ZERO", str.str());
      return;
    }
    m_source_count[obj] -= count;
  }

  m_total_count[obj] -= count;
  m_total_count_ev.notify();

  if (m_trace_mode)
    m_report(obj, source_obj, description, count, "dropped");

  dropped(obj, source_obj, description, count);

  // if count != 0, no reason to fork
  if (m_total_count[obj] != 0)
  {
    if ( !m_hier_mode && obj != m_top )
      m_drop(m_top, source_obj, description, count, in_top_thread);
    else
      if (obj != m_top)
        this->m_propagate(obj, source_obj, description, count, false, in_top_thread);
  }
  else
  {
    uvm_objection_context_object* ctxt = nullptr;
    if (m_context_pool.size())
    {
      ctxt = m_context_pool.front();                // pop_
      m_context_pool.erase(m_context_pool.begin()); // front
    }
    else
      ctxt = new uvm_objection_context_object();

    ctxt->obj = obj;
    ctxt->source_obj = source_obj;
    ctxt->description = description;
    ctxt->count = count;
    ctxt->objection = this;

    // Need to be thread-safe, let the background
    // process handle it.

    // Why don't we look at in_top_thread here?  Because
    // a re-raise will kill the drain at object that it's
    // currently occuring at, and we need the leaf-level kills
    // to not cause accidental kills at branch-levels in
    // the propagation.

    // Using the background process just allows us to
    // separate the links of the chain.
    m_scheduled_list().push_back(ctxt);
    m_scheduled_list_changed().notify();

  } // else: !if(m_total_count[obj] != 0)
}

//----------------------------------------------------------------------
// member function: m_forked_drain
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_objection::m_forked_drain( uvm_object* obj,
                                    uvm_object* source_obj,
                                    const std::string& description,
                                    int count,
                                    int in_top_thread )
{
  if (m_drain_time.find(obj) != m_drain_time.end()) // exists
    sc_core::wait(m_drain_time[obj]); // TODO introduce macro UVM_DELAY(m_drain_time[obj]);

  if (m_trace_mode)
    m_report(obj, source_obj, description, count, "all_dropped");

  all_dropped(obj,source_obj,description, count);

  // wait for all_dropped cbs to complete
  // TODO are there sub processess here????
  // wait fork;

  /* NOT NEEDED - Any raise would have killed us!
    if(!m_total_count.exists(obj))
      diff_count = -count;
    else
      diff_count = m_total_count[obj] - count;
   */

  // we are ready to delete the 0-count entries for the current
  // object before propagating up the hierarchy.
  if ( m_source_count.find(obj) != m_source_count.end() // exists
      && m_source_count[obj] == 0)
    m_source_count.erase(obj);

  if (m_total_count.find(obj) != m_total_count.end() // exists
      && m_total_count[obj] == 0)
    m_total_count.erase(obj);

  if (!m_hier_mode && obj != m_top)
    m_drop(m_top, source_obj, description, count, 1);
  else
    if (obj != m_top)
      m_propagate(obj, source_obj, description, count, false, 1);

}

//----------------------------------------------------------------------
// member function: m_propagate
//
//! Implementation-defined member function
//!
//! Propagate the objection to the objects parent. If the object is a
//! component, the parent is just the hierarchical parent. If the object is
//! a sequence, the parent is the parent sequence if one exists, or
//! it is the attached sequencer if there is no parent sequence.
//!
//! obj : the uvm_object on which the objection is being raised or lowered
//! source_obj : the root object on which the end user raised/lowered the
//!   objection (as opposed to an anscestor of the end user object)a
//! count : the number of objections associated with the action.
//! raise : indicator of whether the objection is being raised or lowered. A
//!   1 indicates the objection is being raised.
//----------------------------------------------------------------------

void uvm_objection::m_propagate( uvm_object* obj,
                                 uvm_object* source_obj,
                                 const std::string& description,
                                 int count,
                                 bool raise,
                                 int in_top_thread)
{
  if (obj != nullptr && obj != m_top)
  {
    obj = m_get_parent(obj);
    if(raise)
      m_raise(obj, source_obj, description, count);
    else
      m_drop(obj, source_obj, description, count, in_top_thread);
  }
}

//----------------------------------------------------------------------
// member function: m_get_parent
//
//! Internal member function for getting the parent of the given \p object.
//! The ultimate parent is uvm_top, UVM's implicit top-level component.
//----------------------------------------------------------------------

uvm_object* uvm_objection::m_get_parent( uvm_object* obj )
{
  uvm_component* comp;
  uvm_sequence_base* seq;

  comp = dynamic_cast<uvm_component*>(obj);
  if (comp != nullptr)
  {
    obj = comp->get_parent();
  }
  else
  {
    seq = dynamic_cast<uvm_sequence_base*>(obj);
    if (seq != nullptr)
      obj = dynamic_cast<uvm_object*>(seq->get_sequencer());
    else
      obj = m_top;
  }

  if (obj == nullptr)
    obj = m_top;

  return obj;
}

//----------------------------------------------------------------------
// member function: m_report
//
//! Internal member function for reporting count updates
//----------------------------------------------------------------------

void uvm_objection::m_report( uvm_object* obj,
                              uvm_object* source_obj,
                              const std::string& description,
                              int count,
                              const std::string& action ) const
{
  std::string desc;
  int _count = (m_source_count.find(obj) != m_source_count.end()) ? m_source_count.find(obj)->second : 0;
  int _total = (m_total_count.find(obj) != m_total_count.end()) ? m_total_count.find(obj)->second : 0;

  if (!uvm_report_enabled(UVM_NONE, UVM_INFO, "OBJTN_TRC") || !m_trace_mode) return;

  //desc = description.empty() ? "" : {" ", description, "" }; // commented in UVM_SV

  if (source_obj == obj)
  {
    std::ostringstream str1, str2;
    if (!description.empty()) str1 << " (" << description << ")";
    str2 << "Object " << ((obj->get_full_name().empty()) ? "uvm_top" : obj->get_full_name() )
         << " " << action << " " << count << " objection(s) " << str1.str()
         << " count = " << _count << " total = " << _total;
    UVM_INFO("OBJTN_TRC", str2.str(), UVM_NONE);
  }
  else
  {
    int cpath = 0;
    int last_dot = 0;
    std::string sname = source_obj->get_full_name();
    std::string nm = obj->get_full_name();

    int max = sname.length() > nm.length() ? (int)nm.length() : (int)sname.length();

    // For readability, only print the part of the source obj hierarchy underneath
    // the current object.
    while((sname[cpath] == nm[cpath]) && (cpath < max))
    {
      if(sname[cpath] == '.')
        last_dot = cpath;
      cpath++;
    }

    if(last_dot)
      sname = sname.substr(last_dot+1, sname.length());

    std::ostringstream str;
    str << "Object " << ((obj->get_full_name().empty()) ? "uvm_top" : obj->get_full_name())
        << " " << ((action == "raised") ? "added" : "subtracted")
        << " " << count << " objection(s) " << ((action == "raised") ? "to" : "from")
        << " its total (" << action << " from source object " << sname << ((!description.empty()) ? ", "+description : "")
        << "): count = " << _count << " total = " << _total;
    UVM_INFO("OBJTN_TRC", str.str(), UVM_NONE);
  }
}


//----------------------------------------------------------------------
// member function: convert2string
//
// Implementation-defined member function
//----------------------------------------------------------------------

std::string uvm_objection::convert2string() const
{
  return m_display_objections(m_top, true);
}

//----------------------------------------------------------------------
// member function: m_execute_scheduled_forks
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_objection::m_execute_scheduled_forks()
{
  while(1)
  {
    while (m_scheduled_list().size() == 0)
      sc_core::wait(m_scheduled_list_changed());

    if(m_scheduled_list().size() != 0)
    {
      uvm_objection_context_object* c = nullptr;

      // Save off the context before the fork
      c = m_scheduled_list().front();
      m_scheduled_list().erase(m_scheduled_list().begin()); // pop_front
      m_scheduled_list_changed().notify();

      // A re-raise can use this to figure out props (if any)
      c->objection->m_scheduled_contexts[c->obj] = c;

      // The fork below pulls out from the forked list
      c->objection->m_forked_list.push_back(c);

      // The fork will guard the m_forked_drain call, but
      // a re-raise can kill m_forked_list contexts in the delta
      // before the fork executes.
      // TODO check delta behavior in context of SystemC

      sc_core::sc_process_handle proc =
          sc_core::sc_spawn(sc_bind(&uvm_objection::m_forked_drain_guard, c->objection, c));

      // Save off our process handle, so a re-raise can kill it...
      c->objection->m_drain_proc[c->obj] = proc;

    } // if
  } // while
}

void uvm_objection::m_forked_drain_guard(uvm_objection_context_object* context)
{
  uvm_objection* objection = context->objection;

  // Check to make sure re-raise didn't empty the fifo
  if (objection->m_forked_list.size() > 0)
  {
    uvm_objection_context_object* ctxt = nullptr;
    ctxt = objection->m_forked_list.front();
    objection->m_forked_list.erase(objection->m_forked_list.begin());

    // Clear it out of scheduled
    objection->m_scheduled_contexts.erase(ctxt->obj);

    // Move it in to forked (so re-raise can figure out props)
    objection->m_forked_contexts[ctxt->obj] = ctxt;

    // Execute the forked drain
    objection->m_forked_drain(ctxt->obj, ctxt->source_obj, ctxt->description, ctxt->count, 1);

    // Cleanup if we survived (no re-raises)
    objection->m_drain_proc.erase(ctxt->obj);
    objection->m_forked_contexts.erase(ctxt->obj);

    // Clear out the context object (prevent memory leaks)
    ctxt->clear();

    // Save the context in the pool for later reuse
    m_context_pool.push_back(ctxt);
  }
}

//----------------------------------------------------------------------
// member function: m_init_objections (static)
//
//! Implementation-defined member function
//! should be initialized as spawned process
//----------------------------------------------------------------------

void uvm_objection::m_init_objections()
{
   sc_core::sc_spawn(sc_bind(&uvm_objection::m_execute_scheduled_forks));
}

//----------------------------------------------------------------------
// member function: m_display_objections
//
// Implementation-defined member function
//----------------------------------------------------------------------

std::string uvm_objection::m_display_objections( uvm_object* obj,
                                            bool show_header ) const
{
  std::string blank = "                                                                                   ";
  std::string s;
  std::string ret;
  int total;
  typedef std::map<std::string, uvm_object*> lmapT;
  typedef lmapT::iterator lmapitT;
  lmapT lmap;
  uvm_object* curr_obj;
  int depth;
  std::string name;

  for( m_count_mapcitT it = m_total_count.begin();
         it != m_total_count.end();
         it ++)
  {
    uvm_object* theobj = it->first;
    //if ( m_total_count[it->first] > 0) // also show zeros
      lmap[theobj->get_full_name()] = theobj;
  }

  if (obj == nullptr)
    obj = m_top;

  total = get_objection_total(obj);

  std::ostringstream lstr;

  lstr << std::endl << "The total objection count is " << total << std::endl;

  if (total == 0)
    return lstr.str();

  lstr << "---------------------------------------------------------" << std::endl;
  lstr << " Source  Total   " << std::endl;
  lstr << " Count   Count   Object" << std::endl;
  lstr << "---------------------------------------------------------" << std::endl;

  for( lmapitT lit = lmap.begin();
         lit != lmap.end();
         lit++)
  {
    curr_obj = lmap[lit->first];

    // determine depth
    depth = 0;
    std::string s = lit->first;

    for( std::string::iterator it = s.begin();
           it != s.end();
           it ++)
      if ( (*it) == '.')
        depth++;

    // determine leaf name
    name = lit->first;
    for (int i = (int)lit->first.length()-1; i >= 0; i--)
      if (lit->first[i] == '.')
      {
         name = lit->first.substr(i+1, lit->first.length()-1);
         break;
      }

    if (lit->first.empty())
      name = "uvm_top";
    else
      depth++;

    // print it
    lstr << std::setw(7) << ((m_source_count.find(curr_obj) != m_source_count.end()) ? m_source_count.find(curr_obj)->second : 0)
         << std::setw(7) << ((m_total_count.find(curr_obj) != m_total_count.end()) ? m_total_count.find(curr_obj)->second : 0)
         << blank.substr(0,2*depth+1)
         << name
         << std::endl;
  }

  lstr << "---------------------------------------------------------" << std::endl;

  return lstr.str();
}

////////////////////

} /* namespace uvm */
