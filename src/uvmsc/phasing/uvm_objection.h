//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
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

#ifndef UVM_OBJECTION_H_
#define UVM_OBJECTION_H_

#include <systemc>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/base/uvm_root.h"

#include <map>
#include <list>

//////////////

namespace uvm {

class uvm_objection_events
{
 public:
  int waiters;
  sc_core::sc_event raised;
  sc_core::sc_event dropped;
  sc_core::sc_event all_dropped;

  uvm_objection_events() : waiters(0) {}
};

// Have a pool of context objects to use
class uvm_objection_context_object
{
 public:
  uvm_object* obj;
  uvm_object* source_obj;
  std::string description;
  int count;
  uvm_objection* objection;

  // Clears the values stored within the object,
  // preventing memory leaks from reused objects
  void clear()
  {
    obj = nullptr;
    source_obj = nullptr;
    description = "";
    count = 0;
    objection = nullptr;
  }
};

//------------------------------------------------------------------------------
// CLASS: uvm_objection
//
//! This base class defines the objection mechanism and end-of-test
//! functionality.
//------------------------------------------------------------------------------

class uvm_objection : public uvm_object
{
 public:

  typedef std::vector<uvm_objection*> m_objections_list;
  typedef sc_core::sc_event m_event;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  // Constructors
  uvm_objection();
  uvm_objection( const std::string& name );

  virtual void clear( uvm_object* obj = nullptr );

  bool trace_mode( int mode = -1 );

  //--------------------------------------------------------------------------
  // Group: Objection Control
  //--------------------------------------------------------------------------

  void m_set_hier_mode( uvm_object* obj );

  virtual void raise_objection( uvm_object* obj,
                                const std::string& description = "",
                                int count = 1 );

  virtual void drop_objection( uvm_object* obj,
                               const std::string& description = "",
                               int count = 1 );

  void set_drain_time( uvm_object* obj = nullptr,
                       const sc_core::sc_time& drain = sc_core::SC_ZERO_TIME );

  //--------------------------------------------------------------------------
  // Group: Callback Hooks
  //--------------------------------------------------------------------------

  virtual void raised( uvm_object* obj,
                       uvm_object* source_obj,
                       const std::string& description,
                       int count );

  virtual void dropped( uvm_object* obj,
                        uvm_object* source_obj,
                        const std::string& description,
                        int count );

  virtual void all_dropped( uvm_object* obj,
                            uvm_object* source_obj,
                            const std::string& description,
                            int count );

  //--------------------------------------------------------------------------
  // Group: Objection Status
  //--------------------------------------------------------------------------

  void get_objectors( std::vector<uvm_object*>& objlist ) const;

  void wait_for( uvm_objection_event objt_event,
                 uvm_object* obj = nullptr );

  int get_objection_count( uvm_object* obj = nullptr ) const;

  int get_objection_total( uvm_object* obj = nullptr ) const;

  const sc_core::sc_time get_drain_time( uvm_object* obj = nullptr ) const;

  void display_objections( uvm_object* obj = nullptr,
                           bool show_header = true ) const;

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  void m_objection_init();

  static m_objections_list& m_objections();

  void m_raise( uvm_object* obj,
                uvm_object* source_obj,
                const std::string& description = "",
                int count = 1 );

  void m_drop( uvm_object* obj,
               uvm_object* source_obj,
               const std::string& description = "",
               int count = 1,
               int in_top_thread = 0 );

  void m_forked_drain( uvm_object* obj,
                       uvm_object* source_obj,
                       const std::string& description = "",
                       int count = 1,
                       int in_top_thread = 0 );

  void m_propagate( uvm_object* obj,
                    uvm_object* source_obj,
                    const std::string& description,
                    int count,
                    bool raise,
                    int in_top_thread );

  uvm_object* m_get_parent( uvm_object* obj );

  void m_report( uvm_object* obj,
                 uvm_object* source_obj,
                 const std::string& description,
                 int count,
                 const std::string& action ) const;

  std::string convert2string() const override;

  static void m_execute_scheduled_forks();

  void m_forked_drain_guard( uvm_objection_context_object* context );

  static void m_init_objections();

  std::string m_display_objections( uvm_object* obj = nullptr,
                                    bool show_header = true ) const;
  // member variables
 protected:
  bool m_trace_mode;
  typedef std::map< uvm_object*, int > m_count_mapT;
  typedef m_count_mapT::iterator m_count_mapitT;
  typedef m_count_mapT::const_iterator m_count_mapcitT;
  m_count_mapT m_source_count;
  m_count_mapT m_total_count;
  std::map<uvm_object*, sc_core::sc_time> m_drain_time;
  std::map<uvm_object*, uvm_objection_events*> m_events;

  uvm_root* m_top;

  sc_core::sc_event m_total_count_ev;

  bool m_cleared;
  sc_core::sc_process_handle m_background_proc;

  bool m_hier_mode;

  uvm_objection_context_object* m_ctxt;

  typedef std::vector<uvm_objection_context_object*> m_objection_list_t;
  m_objection_list_t m_context_pool;

  typedef m_objection_list_t::iterator m_scheduled_list_itt;
  m_objection_list_t m_forked_list;

  typedef std::map<uvm_object*, uvm_objection_context_object* > m_objection_map_t;
  typedef m_objection_map_t::iterator m_objection_map_itt;
  m_objection_map_t m_scheduled_contexts;
  m_objection_map_t m_forked_contexts;

  static m_objection_list_t& m_scheduled_list();        // was static var, now function
  static sc_core::sc_event& m_scheduled_list_changed(); // was static var, now function

  std::map<uvm_object*, sc_core::sc_process_handle> m_drain_proc;

 public:
  bool m_top_all_dropped;

  typedef std::vector<uvm_objection*>::iterator m_obj_listItT;

}; // class uvm_objection

//////////////

} // namespace uvm


#endif /* UVM_OBJECTION_H_ */
