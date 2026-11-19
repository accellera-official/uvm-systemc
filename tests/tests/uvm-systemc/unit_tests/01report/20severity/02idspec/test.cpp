//---------------------------------------------------------------------- 
//   Copyright 2017 NXP B.V.
//   Copyright 2010 Cadence Design Systems, Inc. 
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

// This test checks that a id specific severity overrides work.
// For this test, all of the override ids have _OVR as part of
// the name, and all non-overrides do not.

#include <systemc>
#include <uvm>

using namespace uvm;
using namespace std;
using namespace sc_core;

// global variable to track status
bool pass;

typedef pair<uvm_severity, string> sev_id_pair;

class my_catcher : public uvm_report_catcher
{
 public:

  map<sev_id_pair, int> sev_map;
  sev_id_pair p;

  action_e do_catch() override
  {
    string s_str;
    string exp_sev;

    uvm_coreservice_t* cs_;
    cs_ = uvm_coreservice_t::get();

    // Ignore messages from root
    if(get_client() == cs_->get_root())
      return THROW;

    p = make_pair(get_severity(), get_id());

    sev_map[p]++;

    cout << "Got message id " << p.second << " with severity " << uvm_severity_name[p.first] << " and expected severity " << get_message() << endl;

    exp_sev = get_message();

    if(uvm_severity_name[p.first] != exp_sev)
    {
      cout << "**** UVM_TEST failed expected severity " << exp_sev << " got " << uvm_severity_name[p.first] << " ****" << endl;
      pass = false;
    }
    return CAUGHT;
  }
};


class test : public uvm_test
{
public:

  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name ) : uvm_test(name)
  {}

  my_catcher ctchr;

  void run_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this);
    uvm_report_cb::add(nullptr, &ctchr);

    // Set severities to INFO and then do a couple of messages of each type
    set_id_severities("id1", UVM_INFO);
    try_severities("id1", "UVM_INFO");

    wait(15, SC_SEC);

    // Set severities to WARNING and then do a couple of messages of each type
    set_id_severities("id2", UVM_WARNING);
    try_severities("id2", "UVM_WARNING");

    wait(10, SC_SEC);

    // Set severities to ERROR and then do a couple of messages of each type
    set_id_severities("id3", UVM_ERROR);
    try_severities("id3", "UVM_ERROR");

    wait(10, SC_SEC);

    // Set severities to FATAL and then do a couple of messages of each type
    set_id_severities("id4", UVM_FATAL);
    try_severities("id4", "UVM_FATAL");

    phase.drop_objection(this);
  }

  void report_phase(uvm_phase& phase) override
  {
    if(ctchr.sev_map.size() != 32)
    {
      cout << "*** UVM TEST FAILED Expected to catch eight different severity/id pairs, but got "
          << ctchr.sev_map.size() << " instead ***" << endl;
      pass = false;
    }

    typedef map<sev_id_pair, int>::iterator itt;

    for(itt it = ctchr.sev_map.begin(); it != ctchr.sev_map.end(); ++it)
    {
      sev_id_pair p = it->first;
      int cnt = it->second;

      if(cnt != 1)
      {
        cout << "*** UVM TEST FAILED Expected to catch 1 messages of type (" << uvm_severity_name[p.first]
             << ", " << p.second << ") but got " << cnt << " instead ***" << endl;
        pass = false;
      }
    }

    if (pass)
      std::cout << "** UVM TEST PASSED **" << std::endl;
    else
      std::cout << "** UVM TEST FAILED! **" << std::endl;
  }

  void set_id_severities(string id, uvm_severity sev)
  {
    set_report_severity_id_override(UVM_INFO, "INFO_" + id, sev);
    set_report_severity_id_override(UVM_WARNING, "WARNING_" + id, sev);
    set_report_severity_id_override(UVM_ERROR, "ERROR_" + id, sev);
    set_report_severity_id_override(UVM_FATAL, "FATAL_" + id, sev);
  }

  void try_severities(string id, string sev)
  {
    // For each type, there is one that will be overridden and one that will be
    // untouched. The message string is the expected verbosity of the message.

    UVM_INFO("INFO_" + id, sev, UVM_NONE);
    UVM_INFO("INFO_" + id + "_SAFE", "UVM_INFO", UVM_NONE);
    UVM_WARNING("WARNING_" + id, sev);
    UVM_WARNING("WARNING_" + id + "_SAFE", "UVM_WARNING");
    UVM_ERROR("ERROR_" + id, sev);
    UVM_ERROR("ERROR_" + id + "_SAFE", "UVM_ERROR");
    UVM_FATAL("FATAL_" + id, sev);
    UVM_FATAL("FATAL_" + id + "_SAFE", "UVM_FATAL");
  }
};


int sc_main(int, char*[])
{
  pass = true;
  run_test("test");
  return 0;
}
