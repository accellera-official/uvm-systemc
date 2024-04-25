//----------------------------------------------------------------------
//   Copyright 2014 Université Pierre et Marie Curie, Paris
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2013-2020 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
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

#include "uvmsc/base/uvm_event_callback.h"
#include "uvmsc/base/uvm_event.h"
#include "uvmsc/base/uvm_globals.h"

using namespace sc_core;

namespace uvm {

//----------------------------------------------------------------------------
// initialization of static data members
//----------------------------------------------------------------------------

const std::string uvm_event::type_name = "uvm::uvm_event";
int uvm_event::g_cnt = 0;

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

uvm_event::uvm_event( const std::string& name ): uvm_object(name)
{
  trigger_data = NULL;
  trigger_time = SC_ZERO_TIME;

  m_event = new sc_event();
  m_on_ev = new sc_event();

  m_uvm_event_list.clear();

  on = false;
  m_event_val = false;
  m_num_waiters = 0;
}

//----------------------------------------------------------------------
// Group: waiting
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: wait_on (virtual)
//
//! Waits for the event to be activated for the first time.
//!
//! If the event has already been triggered, this task returns immediately.
//! If /p delta is set, the caller will be forced to wait a single delta 0
//! before returning. This prevents the caller from returning before
//! previously waiting processes have had a chance to resume.
//!
//! Once an event has been triggered, it will be remain "on" until the event
//! is <reset>.
//----------------------------------------------------------------------

void uvm_event::wait_on( bool delta )
{
  if (on) // true = event was triggered already
  {
    if (delta)
    	sc_core::wait(SC_ZERO_TIME);
    return;
  }
  m_num_waiters++;
  sc_core::wait(*m_on_ev); // wait for 'on' event
}

//----------------------------------------------------------------------
// Member function: wait_off (virtual)
//
//! If the event has already triggered and is "on", this task waits for the
//! event to be turned "off" via a call to <reset>.
//!
//! If the event has not already been triggered, this task returns immediately.
//! If ~delta~ is set, the caller will be forced to wait a single delta #0
//! before returning. This prevents the caller from returning before
//! previously waiting processes have had a chance to resume.
//----------------------------------------------------------------------

void uvm_event::wait_off( bool delta )
{
  if (!on) //
  {
    if (delta)
      sc_core::wait(SC_ZERO_TIME);
    return;
  }
  m_num_waiters++;
  sc_core::wait(*m_on_ev); // wait for 'on' event
}

//----------------------------------------------------------------------
// Member function: wait_trigger (virtual)
//
//! Waits for the event to be triggered.
//!
//! If one process calls wait_trigger in the same delta as another process
//! calls <trigger>, a race condition occurs. If the call to wait occurs
//! before the trigger, this method will return in this delta. If the wait
//! occurs after the trigger, this method will not return until the next
//! trigger, which may never occur and thus cause deadlock.
//----------------------------------------------------------------------

void uvm_event::wait_trigger()
{
  m_num_waiters++;
  sc_core::wait(*m_event);
  m_event_val = true;
}

//----------------------------------------------------------------------
// Member function: wait_ptrigger (virtual)
//
//! Waits for a persistent trigger of the event. Unlike <wait_trigger>, this
//! views the trigger as persistent within a given time-slice and thus avoids
//! certain race conditions. If this method is called after the trigger but
//! within the same time-slice, the caller returns immediately.
//----------------------------------------------------------------------

void uvm_event::wait_ptrigger()
{
  if (m_event_val) // m_event.triggered
    return;
  m_num_waiters++;
  sc_core::wait(*m_event);
  m_event_val = true;

}

//----------------------------------------------------------------------
// Member function: wait_trigger_data (virtual)
//
//! This method calls <wait_trigger> followed by <get_trigger_data>.
//----------------------------------------------------------------------

void uvm_event::wait_trigger_data( uvm_object*& data )
{
  wait_trigger();
  data = get_trigger_data();
}


//----------------------------------------------------------------------
// Member function: wait_ptrigger_data (virtual)
//
//! This member function calls #wait_ptrigger followed by #get_trigger_data.
//----------------------------------------------------------------------

void uvm_event::wait_ptrigger_data( uvm_object*& data )
{
  wait_ptrigger();
  data = get_trigger_data();
}

//----------------------------------------------------------------------
// triggering
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: trigger (virtual)
//
//! Triggers the event, resuming all waiting processes.
//!
//! An optional argument \p data can be supplied with the enable to provide
//! trigger-specific information.
//----------------------------------------------------------------------

void uvm_event::trigger( uvm_object* data )
{
  int skip = 0;
  if (callbacks.size())
  {
    for( unsigned int i = 0; i < callbacks.size(); i++ )
    {
      uvm_event_callback* tmp;
      tmp = callbacks[i];
      skip = skip + tmp->pre_trigger(this, data);
    }
  }
  if (skip == 0)
  {
    m_event->notify();
    if (callbacks.size() != 0)
    {
      for( unsigned int i = 0; i < callbacks.size(); i++ )
      {
        uvm_event_callback* tmp;
        tmp = callbacks[i];
        tmp->post_trigger(this, data);
      }
    }
    m_num_waiters = 0;
    on = true;
    m_on_ev->notify();
    trigger_time = sc_time_stamp();
    trigger_data = data;
  }
}

//----------------------------------------------------------------------
// Member function: get_trigger_data (virtual)
//
//! Gets the data, if any, provided by the last call to #trigger.
//----------------------------------------------------------------------

uvm_object* uvm_event::get_trigger_data()
{
  return trigger_data;
}

//----------------------------------------------------------------------
// Member function: get_trigger_time (virtual)
//
//! Gets the time that this event was last triggered. If the event has not been
//! triggered, or the event has been reset, then the trigger time will be 0.
//----------------------------------------------------------------------

sc_time uvm_event::get_trigger_time()
{
  return trigger_time;
}

//----------------------------------------------------------------------
// Group: state
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: is_on (virtual)
//
//! Indicates whether the event has been triggered since it was last reset.
//!
//! A return of true indicates that the event has triggered.
//----------------------------------------------------------------------

bool uvm_event::is_on()
{
  return on;
}

//----------------------------------------------------------------------
// Member function: is_off (virtual)
//
//! Indicates whether the event has been triggered or been reset.
//!
//! A return of true indicates that the event has not been triggered.
//----------------------------------------------------------------------

bool uvm_event::is_off()
{
  return !on;
}

//----------------------------------------------------------------------
// Member function: reset (virtual)
//
//! Resets the event to its off state. If argument \p wakeup is set,
//! then all processes
//! currently waiting for the event are activated before the reset.
//!
//! No callbacks are called during a reset.
//----------------------------------------------------------------------

void uvm_event::reset( bool wakeup )
{
  if (wakeup)
    m_event->notify();

  m_event->cancel(); // delete all pending notifications, basically a new event queue
  m_num_waiters = 0;
  on = false;
  m_on_ev->notify();
  trigger_time = SC_ZERO_TIME;
  trigger_data = NULL;
}

//----------------------------------------------------------------------
// callbacks
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: add_callback (virtual)
//
//! Registers a callback object, \p cb, with this event. The callback object
//! may include pre_trigger and post_trigger functionality. If argument \p append is
//! true, the default, \p cb is added to the back of the callback list. Otherwise,
//! \p cb is placed at the front of the callback list.
//----------------------------------------------------------------------

void uvm_event::add_callback( uvm_event_callback* cb, bool append )
{
  for( unsigned int i = 0; i < callbacks.size(); i++ )
  {
    if (cb == callbacks[i])
    {
      uvm_report_warning("CBRGED","add_callback: Callback already registered. Ignoring.", UVM_NONE);
      return;
    }
  }
  if (append)
    callbacks.push_back(cb);
  else
    callbacks.insert(callbacks.begin(), cb); // push at front
}

//----------------------------------------------------------------------
// Member function: delete_callback (virtual)
//
//! Unregisters the given callback, \p cb, from this event.
//----------------------------------------------------------------------

void uvm_event::delete_callback( uvm_event_callback* cb )
{
  for( callbacks_itt it = callbacks.begin(); it != callbacks.end(); it++)
  {
    if (cb == (*it))
    {
      delete (*it); // delete from memory
      callbacks.erase(it);  // delete from vector list
      return;
    }
  }
  uvm_report_warning("CBNTFD", "delete_callback: Callback not found. Ignoring delete request.", UVM_NONE);
}

//----------------------------------------------------------------------
// waiters list
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: cancel (virtual)
//
//! Decrements the number of waiters on the event.
//!
//! This is used if a process that is waiting on an event is disabled or
//! activated by some other means.
//----------------------------------------------------------------------

void uvm_event::cancel()
{
  if (m_num_waiters > 0)
    m_num_waiters--;
}

//----------------------------------------------------------------------
// Member function: get_m_num_waiters (virtual)
//
//! Returns the number of processes waiting on the event.
//----------------------------------------------------------------------

int uvm_event::get_num_waiters() const
{
  return m_num_waiters;
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// member function: m_clean
//
//! Helper function for destructor
//----------------------------------------------------------------------

void uvm_event::m_clean()
{
  // clean memory
  if (m_event != NULL) { delete m_event; m_event = NULL; }
  if (m_on_ev != NULL) { delete m_on_ev; m_on_ev = NULL; }

  while(!m_uvm_event_list.empty())
  {
    delete m_uvm_event_list.back();
    m_uvm_event_list.pop_back();
  }
}

//----------------------------------------------------------------------
// member function: m_init_event
//
//! Helper function for operator= and copy constructor
//----------------------------------------------------------------------

void uvm_event::m_init_event( const uvm_event& ev )
{
  trigger_data = ev.trigger_data;
  trigger_time = ev.trigger_time;

  // TODO we will only create a new empty events and event list, no copy
  // (copy of sc_event content is not allowed)
  // check if this is sufficient

  m_event = new sc_event();
  m_on_ev = new sc_event();
  m_uvm_event_list.clear();

  on = ev.on;
  m_event_val = ev.m_event_val;
}


//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------

uvm_event::~uvm_event()
{
  m_clean();
}

//----------------------------------------------------------------------
// Copy constructor
//----------------------------------------------------------------------

uvm_event::uvm_event( const uvm_event& ev )
{
  m_init_event(ev);
}

//----------------------------------------------------------------------
// Assignment operator
//----------------------------------------------------------------------

uvm_event& uvm_event::operator=( const uvm_event& ev )
{
  if(this != &ev)
  {
    m_clean();
    m_init_event(ev);
  }
  return *this;
}

//----------------------------------------------------------------------
// Member function: create (virtual)
//
//! Implementation defined
//----------------------------------------------------------------------

uvm_object* uvm_event::create( const std::string& name )
{
  uvm_event* v = new uvm_event(name);

  // put object on stack so we can remove it later
  m_uvm_event_list.push_back(v);

  return v;
}

//----------------------------------------------------------------------
// Member function: get_type_name (virtual)
//
//! Return type name of the object
//----------------------------------------------------------------------

const std::string uvm_event::get_type_name() const
{
  return type_name;
}

//----------------------------------------------------------------------
// Member function: do_print (virtual)
//
//! Print the content of the object using the printer policy
//----------------------------------------------------------------------

void uvm_event::do_print( const uvm_printer& printer ) const
{
  /* TODO print events
  printer.print_int("m_num_waiters", m_num_waiters, $bits(m_num_waiters), UVM_DEC, ".", "int");
  printer.print_int("on", on, $bits(on), UVM_BIN, ".", "bit");
  printer.print_time("trigger_time", trigger_time);
  printer.print_object("trigger_data", trigger_data);
  printer.m_scope.down("callbacks");
  foreach(callbacks[e]) begin
    printer.print_object($sformatf("[%0d]",e), callbacks[e], "[");
  end
  printer.m_scope.up();
  */
}

void uvm_event::do_copy( const uvm_object& rhs )
{
  /* TODO copy
  uvm_event* e;
  uvm_object::do_copy(rhs);
  e = dynamic_cast<uvm_event*>(rhs);

  if(e==NULL) return;

  m_event = e->m_event;
  m_num_waiters = e->m_num_waiters;
  on = e->on;
  trigger_time = e->trigger_time;
  trigger_data = e->trigger_data;
  callbacks.clear();// TODO clear memory
  callbacks = e->callbacks;
  */
}

} // namespace uvm

