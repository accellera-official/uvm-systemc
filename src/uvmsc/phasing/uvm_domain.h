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

#ifndef UVM_DOMAIN_H_
#define UVM_DOMAIN_H_

#include <map>

#include "uvmsc/phasing/uvm_phase.h"

//////////////

namespace uvm {

// forward declaration of necessary classes.
class uvm_build_phase;
class uvm_connect_phase;
class uvm_end_of_elaboration_phase;
class uvm_start_of_simulation_phase;
class uvm_run_phase;
class uvm_extract_phase;
class uvm_check_phase;
class uvm_report_phase;
class uvm_final_phase;
class uvm_pre_reset_phase;
class uvm_reset_phase;
class uvm_post_reset_phase;
class uvm_pre_configure_phase;
class uvm_configure_phase;
class uvm_post_configure_phase;
class uvm_pre_main_phase;
class uvm_main_phase;
class uvm_post_main_phase;
class uvm_pre_shutdown_phase;
class uvm_shutdown_phase;
class uvm_post_shutdown_phase;

//------------------------------------------------------------------------------
// Class: uvm_domain
//
//! Phasing schedule node representing an independent branch of the schedule.
//! Handle used to assign domains to components or hierarchies in the testbench
//------------------------------------------------------------------------------

class uvm_domain : public uvm_phase
{
 public:
  friend class uvm_component;

  typedef std::map<std::string, uvm_domain*> domains_mapT;
  typedef domains_mapT::iterator domains_mapItT;

//  static uvm_phase* m_uvm_schedule;
//  static uvm_domain* m_uvm_domain; // run-time phases
//  static uvm_domain* m_common_domain;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  static void get_domains( domains_mapT& domains );

  static void add_uvm_phases( uvm_phase* schedule );

  void jump( const uvm_phase* phase );

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////
  
  static uvm_domain* get_uvm_domain();

  uvm_domain( const std::string& name );

  static uvm_domain* get_common_domain();
  
  static uvm_phase* get_uvm_schedule();

 private:

  static domains_mapT& get_m_domains();

  uvm_phase* m_schedule;
};

} // namespace uvm

#endif /* UVM_DOMAIN_H_ */
