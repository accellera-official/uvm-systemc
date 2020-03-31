//----------------------------------------------------------------------
//   Copyright 2013-2016 NXP B.V.
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

#ifndef UVM_EVENT_H_
#define UVM_EVENT_H_

#include <systemc>
#include <vector>
#include <memory>

#include "uvmsc/base/uvm_object.h"

// forward declaration of uvm_packer used in uvm_object
class uvm_object;
class uvm_event_callback;
class uvm_printer;

namespace uvm {

//----------------------------------------------------------------------
// CLASS: uvm_event
//
//! The class uvm_event is a wrapper class around the SystemC event
//! construct.  It provides some additional standardized UVM services
//! such as setting callbacks and maintaining the number of waiters.
//----------------------------------------------------------------------

class uvm_event : public uvm_object
{
 public:

  // Constructor creates a new event object.
  uvm_event( const std::string& name="" );

  //---------
  // waiting
  //---------

  virtual void wait_on ( bool delta = false );

  virtual void wait_off ( bool delta = false );

  virtual void wait_trigger();

  virtual void wait_ptrigger();

  virtual void wait_trigger_data( uvm_object*& data );

  virtual void wait_ptrigger_data( uvm_object*& data );

  //------------
  // triggering
  //------------

  virtual void trigger( uvm_object* data = NULL );

  virtual uvm_object* get_trigger_data();

  virtual sc_core::sc_time get_trigger_time();

  //-------
  // state
  //-------

  virtual bool is_on();

  virtual bool is_off();

  virtual void reset( bool wakeup = false );

  //-----------
  // callbacks
  //-----------

  virtual void add_callback( uvm_event_callback* cb, bool append = true );

  virtual void delete_callback( uvm_event_callback* cb );

  //--------------
  // waiters list
  //--------------

  virtual void cancel();

  virtual int get_num_waiters() const;

  virtual uvm_object* create( const std::string& name = "" );

  virtual const std::string get_type_name() const;

  virtual void do_print( const uvm_printer& printer ) const;

  virtual void do_copy( const uvm_object& rhs );

  /////////////////////////////////////////////////////
  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////
  /////////////////////////////////////////////////////

  virtual ~uvm_event();

  // copy constructor
  uvm_event( const uvm_event& ev );

  uvm_event& operator=( const uvm_event& ev );

private:

  void m_clean();
  void m_init_event( const uvm_event& ev );

  // data members

  const static std::string type_name;

  bool               m_event_val;
  sc_core::sc_event* m_event;

  std::vector<uvm_event*> m_uvm_event_list;

  int                m_num_waiters;
  bool               on;

  sc_core::sc_event* m_on_ev;

  sc_core::sc_time   trigger_time;
  uvm_object*        trigger_data;

  std::vector<uvm_event_callback*> callbacks;
  typedef std::vector<uvm_event_callback*>::iterator callbacks_itt;

  // TODO: unused
  /* int m_cnt; */
  static int g_cnt;
}; // class uvm_event

} // namespace uvm

#endif // UVM_EVENT_H_
