//---------------------------------------------------------------------- 
//   Copyright 2016 NXP B.V.
//   Copyright 2011 Mentor Graphics Corporation
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

// This test creates a simple hierarchy and makes sure that phases
// start at the correct time. The env has no delays in it so all
// phase processed in the env end immediately.

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;
using namespace sc_core;
using namespace std;

// global variable to track fails
bool failed;

// is/is_before/is_after should work with the argument being either
// a phase instance or a phase imp; we'll use an array of instances and
// another array of imps
map<uvm_phase*, int>  imp_index;
map<uvm_phase*, int> inst_index;
int phases;

class test : public uvm_component
{
public:
  UVM_COMPONENT_UTILS(test);

  uvm_phase* build_ph;
  uvm_phase* connect_ph;
  uvm_phase* end_of_elaboration_ph;
  uvm_phase* start_of_simulation_ph;
  uvm_phase* run_ph;
  uvm_phase* extract_ph;
  uvm_phase* check_ph;
  uvm_phase* report_ph;
  uvm_phase* final_ph;
  uvm_phase* pre_reset_ph;
  uvm_phase* reset_ph;
  uvm_phase* post_reset_ph;
  uvm_phase* pre_configure_ph;
  uvm_phase* configure_ph;
  uvm_phase* post_configure_ph;
  uvm_phase* pre_main_ph;
  uvm_phase* main_ph;
  uvm_phase* post_main_ph;
  uvm_phase* pre_shutdown_ph;
  uvm_phase* shutdown_ph;
  uvm_phase* post_shutdown_ph;

  test( uvm_component_name name) : uvm_component(name)
  {}

  void set_up_imp_index()
  {
    build_ph               = uvm_build_phase::get();
    connect_ph             = uvm_connect_phase::get();
    end_of_elaboration_ph  = uvm_end_of_elaboration_phase::get();
    start_of_simulation_ph = uvm_start_of_simulation_phase::get();
    run_ph                 = uvm_run_phase::get();
    extract_ph             = uvm_extract_phase::get();
    check_ph               = uvm_check_phase::get();
    report_ph              = uvm_report_phase::get();
    final_ph               = uvm_final_phase::get();
    pre_reset_ph           = uvm_pre_reset_phase::get();
    reset_ph               = uvm_reset_phase::get();
    post_reset_ph          = uvm_post_reset_phase::get();
    pre_configure_ph       = uvm_pre_configure_phase::get();
    configure_ph           = uvm_configure_phase::get();
    post_configure_ph      = uvm_post_configure_phase::get();
    pre_main_ph            = uvm_pre_main_phase::get();
    main_ph                = uvm_main_phase::get();
    post_main_ph           = uvm_post_main_phase::get();
    pre_shutdown_ph        = uvm_pre_shutdown_phase::get();
    shutdown_ph            = uvm_shutdown_phase::get();
    post_shutdown_ph       = uvm_post_shutdown_phase::get();

    //cout << "uvm_build_ph id is " << build_ph->get_inst_id() << " type=" << build_ph->get_phase_type() << endl;

    imp_index[build_ph]               = 1;
    imp_index[connect_ph]             = 2;
    imp_index[end_of_elaboration_ph]  = 3;
    imp_index[start_of_simulation_ph] = 4;
    imp_index[run_ph]                 = 5;
    imp_index[pre_reset_ph]           = 6;
    imp_index[reset_ph]               = 7;
    imp_index[post_reset_ph]          = 8;
    imp_index[pre_configure_ph]       = 9;
    imp_index[configure_ph]           = 10;
    imp_index[post_configure_ph]      = 11;
    imp_index[pre_main_ph]            = 12;
    imp_index[main_ph]                = 13;
    imp_index[post_main_ph]           = 14;
    imp_index[pre_shutdown_ph]        = 15;
    imp_index[shutdown_ph]            = 16;
    imp_index[post_shutdown_ph]       = 17;
    imp_index[extract_ph]             = 18;
    imp_index[check_ph]               = 19;
    imp_index[report_ph]              = 20;
    imp_index[final_ph]               = 21;
  }

  void set_up_inst_index()
  {
    uvm_domain* l_uvm_domain    = uvm_domain::get_uvm_domain() ;
    uvm_domain* l_common_domain = uvm_domain::get_common_domain() ;

    build_ph               = l_common_domain->find_by_name("build");
    connect_ph             = l_common_domain->find_by_name("connect");
    end_of_elaboration_ph  = l_common_domain->find_by_name("end_of_elaboration");
    start_of_simulation_ph = l_common_domain->find_by_name("start_of_simulation");
    run_ph                 = l_common_domain->find_by_name("run");
    extract_ph             = l_common_domain->find_by_name("extract");
    check_ph               = l_common_domain->find_by_name("check");
    report_ph              = l_common_domain->find_by_name("report");
    final_ph               = l_common_domain->find_by_name("final");
    pre_reset_ph           = l_uvm_domain->find_by_name("pre_reset");
    reset_ph               = l_uvm_domain->find_by_name("reset");
    post_reset_ph          = l_uvm_domain->find_by_name("post_reset");
    pre_configure_ph       = l_uvm_domain->find_by_name("pre_configure");
    configure_ph           = l_uvm_domain->find_by_name("configure");
    post_configure_ph      = l_uvm_domain->find_by_name("post_configure");
    pre_main_ph            = l_uvm_domain->find_by_name("pre_main");
    main_ph                = l_uvm_domain->find_by_name("main");
    post_main_ph           = l_uvm_domain->find_by_name("post_main");
    pre_shutdown_ph        = l_uvm_domain->find_by_name("pre_shutdown");
    shutdown_ph            = l_uvm_domain->find_by_name("shutdown");
    post_shutdown_ph       = l_uvm_domain->find_by_name("post_shutdown");

    //cout << "uvm_build_ph id is " << build_ph->get_inst_id() << " type=" << build_ph->get_phase_type() << endl;

    inst_index[build_ph]               = 1;
    inst_index[connect_ph]             = 2;
    inst_index[end_of_elaboration_ph]  = 3;
    inst_index[start_of_simulation_ph] = 4;
    inst_index[run_ph]                 = 5;
    inst_index[pre_reset_ph]           = 6;
    inst_index[reset_ph]               = 7;
    inst_index[post_reset_ph]          = 8;
    inst_index[pre_configure_ph]       = 9;
    inst_index[configure_ph]           = 10;
    inst_index[post_configure_ph]      = 11;
    inst_index[pre_main_ph]            = 12;
    inst_index[main_ph]                = 13;
    inst_index[post_main_ph]           = 14;
    inst_index[pre_shutdown_ph]        = 15;
    inst_index[shutdown_ph]            = 16;
    inst_index[post_shutdown_ph]       = 17;
    inst_index[extract_ph]             = 18;
    inst_index[check_ph]               = 19;
    inst_index[report_ph]              = 20;
    inst_index[final_ph]               = 21;
  }

  void phase_started(uvm_phase& phase) override
  {
    bool use_imp ;
    static bool done;

    if (!done)
    {
      set_up_inst_index();
      set_up_imp_index();
      done = true;
    }

    if (phase.is_before(nullptr))
      UVM_ERROR("EXP_NOT_BEFORE", "Expected " + phase.get_name() +".is_before(nullptr) to be FALSE");

    if (phase.is(nullptr))
      UVM_ERROR("EXP_NOT_IS", "Expected " + phase.get_name() + ".is(nullptr) to be FALSE");

    if (phase.is_after(nullptr))
      UVM_ERROR("EXP_NOT_AFTER", "Expected " + phase.get_name() + ".is_after(nullptr) to be FALSE");

    use_imp = false;
    test_is_functions(phase, inst_index, use_imp);
    use_imp = true;
    test_is_functions(phase, imp_index, use_imp);

    phases++;
  }

  void test_is_functions(uvm_phase& a_phase, map<uvm_phase*, int> a_index, bool use_imp)
  {
    int spread;
    bool is, isb, isa;
    uvm_phase* imp;

    if (use_imp) imp = a_phase.get_imp();
    else imp = &a_phase;

    cout << "phase_started: "
         << imp->get_phase_type()
         << "="
         << imp->get_name()
         << " (@"
         << imp->get_inst_id()
         << ")"
         << endl;

    if (a_index.find(imp) == a_index.end()) // not exists
    {
      stringstream str;
      str << "Phase '" + imp->get_name()
          << "' (id="
          << imp->get_inst_id()
          << ") not expected. ";
      UVM_ERROR("UNKNOWN_PHASE", str.str());
    }

    for (map<uvm_phase*, int>::iterator it = a_index.begin(); it != a_index.end(); it++)
    {
      uvm_phase* ph = it->first;
      is = a_phase.is(ph);
      isb = a_phase.is_before(ph);
      isa = a_phase.is_after(ph);

      spread = (a_index[imp] - a_index[ph]);

      if ((imp->get_name() == "run" || ph->get_name() == "run") && (((a_index[ph] >= 6 && a_index[ph] <= 17) ||
           (a_index[imp] >=6 && a_index[imp] <= 17))))
      {
          if ( isa) UVM_ERROR("EXP_NOT_AFTER", "Expected " + a_phase.get_name() + ".is_after(" + ph->get_name() + ") to be FALSE");
          if ( isb) UVM_ERROR("EXP_NOT_BEFORE", "Expected " + a_phase.get_name() + ".is_before(" + ph->get_name() + ") to be FALSE");
          if ( is)  UVM_ERROR("EXP_NOT_IS", "Expected " + a_phase.get_name() + ".is(" + ph->get_name() + ") to be FALSE");
          continue;
      }

      if (spread > 0)
      {
        if (!isa) UVM_ERROR("EXP_AFTER", "Expected " + a_phase.get_name() + ".is_after(" + ph->get_name() + ") to be TRUE");
        if ( isb) UVM_ERROR("EXP_NOT_BEFORE", "Expected " + a_phase.get_name() + ".is_before(" + ph->get_name() + ") to be FALSE");
        if ( is)  UVM_ERROR("EXP_NOT_IS", "Expected " + a_phase.get_name() + ".is(" + ph->get_name() + ") to be FALSE");
      }

      if (spread > 0)
      {
        if (!isa) UVM_ERROR("EXP_AFTER", "Expected " + a_phase.get_name() + ".is_after(" + ph->get_name() + ") to be TRUE");
        if ( isb) UVM_ERROR("EXP_NOT_BEFORE", "Expected " + a_phase.get_name() + ".is_before(" + ph->get_name() + ") to be FALSE");
        if ( is)  UVM_ERROR("EXP_NOT_IS", "Expected " + a_phase.get_name() + ".is(" + ph->get_name() + ") to be FALSE");
      }
      else
        if (spread == 0)
        {
          if ( isa) UVM_ERROR("EXP_NOT_AFTER",  "Expected " + a_phase.get_name() + ".is_after(" + ph->get_name() + ") to be FALSE");
          if (!is)  UVM_ERROR("EXP_IS",         "Expected " + a_phase.get_name() + ".is(" + ph->get_name() + ") to be TRUE");
          if ( isb) UVM_ERROR("EXP_NOT_BEFORE", "Expected " + a_phase.get_name() + ".is_before(" + ph->get_name() + ") to be FALSE");
        }
        else
        {
          if ( isa) UVM_ERROR("EXP_NOT_AFTER", "Expected " + a_phase.get_name() + ".is_after(" + ph->get_name() + ") to be FALSE");
          if ( is)  UVM_ERROR("EXP_NOT_IS",    "Expected " + a_phase.get_name() + ".is(" + ph->get_name() + ") to be FALSE");
          if (!isb) UVM_ERROR("EXP_BEFORE",    "Expected " + a_phase.get_name() + ".is_before(" + ph->get_name() + ") to be TRUE");
        }
    }
  }

  void phase_ended(uvm_phase& phase) override
  {}

  void final_phase(uvm_phase& phase) override
  {
    UVM_INFO("FINAL", "Starting Final", UVM_NONE);

    if (phases != 21)
     UVM_ERROR("NOT ENOUGH PHASES", "Expected 21 phases to be started");
  }

  void report_phase(uvm_phase& phase) override
  {}
};


int sc_main(int, char*[])
{
  failed = false;
  phases = 0;

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_report_server* svr = cs->get_report_server();

  run_test("test");

  if (svr->get_severity_count(UVM_FATAL) +
      svr->get_severity_count(UVM_ERROR) == 0)
    cout << "*** UVM TEST PASSED ***" << endl;
  else
    cout << "*** UVM TEST FAILED ***" << endl;

  return 0;
}
