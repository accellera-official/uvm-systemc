//----------------------------------------------------------------------
//   Copyright 2012-2020 NXP B.V.
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

#ifndef UVM_ROOT_H_
#define UVM_ROOT_H_

#include <systemc>

#include "uvmsc/base/uvm_component.h"


namespace uvm {

// forward class declarations
class uvm_default_coreservice_t;
class uvm_component;
class uvm_component_name;

//----------------------------------------------------------------------
// Class: uvm_root
//
//! The class uvm_root serves as the implicit top-level and phase controller for
//! all UVM components. An application shall not directly instantiate uvm_root.
//! The UVM manager automatically creates a single instance of uvm_root.
//----------------------------------------------------------------------

class uvm_root : public uvm_component
{
 public:
  friend class uvm_phase;
  friend class uvm_default_coreservice_t;
  friend class uvm_component_name;

  //--------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------

  static uvm_root* get();

  static uvm_root* m_uvm_get_root();

  //--------------------------------------------------------------------
  // Group: Simulation control
  //--------------------------------------------------------------------

  virtual void run_test( const std::string& test_name = "" );

  virtual void die();

  void set_timeout( const sc_core::sc_time& timeout, bool overridable = true );

  void set_finish_on_completion( bool enable );

  bool get_finish_on_completion();

  //--------------------------------------------------------------------
  // Group: Topology
  //--------------------------------------------------------------------

  uvm_component* find( const std::string& comp_match );

  void find_all( const std::string& comp_match, std::vector<uvm_component*>& comps,
                 uvm_component* comp = nullptr );

  void print_topology( uvm_printer* printer = nullptr );

  void enable_print_topology( bool enable = true );

  //--------------------------------------------------------------------
  // Variables
  //--------------------------------------------------------------------

  const uvm_root* uvm_top; // returns uvm_root singleton

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////
  
  bool get_phase_all_done();

  ~uvm_root() override; // destructor

  sc_core::sc_object* m_hdl_obj; // TODO enable HDL access more elegant

 private:

  // other data members

  bool m_enable_print_topology; // default set to false

  bool m_finish_on_completion; // default set to true

  void m_uvm_header();

  void before_end_of_elaboration() override;
  void end_of_elaboration() override;
  void start_of_simulation() override;
  void end_of_simulation() override;

  const char* kind() const override
  {
    return "uvm::uvm_root";
  }

  explicit uvm_root( uvm_component_name nm );

  void m_find_all_recurse( const std::string& comp_match, std::vector<uvm_component*>& comps,
                           uvm_component* comp = nullptr );

  void m_register_test( const std::string& test_name );

  void m_unregister_test( const std::string& test_name );


  // data members

  bool m_phase_all_done;
  bool phases_registered;

  sc_core::sc_time phase_timeout;
  sc_core::sc_event phase_timeout_changed;
  sc_core::sc_event m_phase_all_done_ev;

  static bool m_uvm_timeout_overridable;

};

} // namespace uvm


#endif /* UVM_ROOT_H_ */
