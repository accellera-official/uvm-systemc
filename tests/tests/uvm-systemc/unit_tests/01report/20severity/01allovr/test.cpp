//---------------------------------------------------------------------- 
//   Copyright 2017-2021 NXP B.V.
//   Copyright 2010-2018 Cadence Design Systems, Inc.
//   Copyright 2010-2014 Mentor Graphics Corporation
//   Copyright 2017 NVIDIA Corporation
//   Copyright 2013 Synopsys, Inc.
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

// This test checks that a generic severity override, INFO->WARNING,
// for example, works and can later be replaced back.

#include <systemc>
#include <uvm>

using namespace uvm;
using namespace sc_core;

// From time 0 to 10 all messages should be INFO
// From time 10 to 20 all messages should be WARNING
// From time 20 to 30 all messages should be ERROR
// From time 30 to 40 all messages should be FATAL

// global variable to track status
bool pass;

class my_catcher : public uvm_report_catcher
{
 public:

  action_e do_catch() override
  {
    uvm_coreservice_t* cs = uvm_coreservice_t::get();

    s = get_severity();

    // Ignore messages from root component
    if(get_client() == cs->get_root())
      return THROW;

    sev[s]++;

    std::cout << sc_time_stamp() << ": got severity " << uvm_severity_name[s] << " for id " << get_id() << std::endl;

    if(sc_time_stamp() < sc_time(10, SC_SEC))
    {
      if(s != UVM_INFO)
      {
        std::cout << "*** UVM TEST FAILED expected UVM_INFO but got " << uvm_severity_name[s] << std::endl;
        pass = false;
      }
    }
    else
      if(sc_time_stamp() <sc_time(20, SC_SEC))
      {
        if(s != UVM_WARNING)
        {
          std::cout << "*** UVM TEST FAILED expected UVM_WARNING but got " << uvm_severity_name[s] << std::endl;
          pass = false;
        }
      }
      else if(sc_time_stamp() < sc_time(30, SC_SEC))
      {
        if(s != UVM_ERROR)
        {
          std::cout << "*** UVM TEST FAILED expected UVM_ERROR but got " << uvm_severity_name[s] << std::endl;
          pass = false;
        }
      }
      else if(sc_time_stamp() < sc_time(40, SC_SEC))
      {
        // Ignore the stop request info
        if(s != UVM_FATAL)
        {
          std::cout << "*** UVM TEST FAILED expected UVM_FATAL but got " << uvm_severity_name[s] << std::endl;
          pass = false;
        }
      }
    return CAUGHT;
  }

 // data members
  std::map<uvm_severity, unsigned int> sev;
  uvm_severity s;
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
    set_all_severities(UVM_INFO);
    try_all_severities();

    wait(15, SC_SEC);

    // Set severities to WARNING and then do a couple of messages of each type
    set_all_severities(UVM_WARNING);
    try_all_severities();

    wait(10, SC_SEC);

    // Set severities to ERROR and then do a couple of messages of each type
    set_all_severities(UVM_ERROR);
    try_all_severities();

    wait(10, SC_SEC);

    // Set severities to FATAL and then do a couple of messages of each type
    set_all_severities(UVM_FATAL);
    try_all_severities();

    phase.drop_objection(this);
  }

  virtual void report(uvm_phase& phase)
  {
    if(ctchr.sev.size() != 4)
    {
      std::cout << "*** UVM TEST FAILED Expected to catch four different severities, but got " << ctchr.sev.size() <<
          " instead ***" << std::endl;
      pass = false;
    }

    typedef std::map<uvm_severity, unsigned int>::iterator sev_mapitt;

    for(sev_mapitt it = ctchr.sev.begin(); it != ctchr.sev.end(); it++)
    {
      unsigned int num = it->second;
      uvm_severity s = it->first;

      if(num != 8)
      {
        std::cout << "*** UVM TEST FAILED Expected to catch 8 messages of type " << uvm_severity_name[s]
            << ", but got " << ctchr.sev[s] << " instead ***" << std::endl;
        pass = false;
      }
    }

    if (pass)
      std::cout << "** UVM TEST PASSED **" << std::endl;
    else
      std::cout << "** UVM TEST FAILED! **" << std::endl;

  }

  void set_all_severities(uvm_severity sev)
  {
    set_report_severity_override(UVM_INFO, sev);
    set_report_severity_override(UVM_WARNING, sev);
    set_report_severity_override(UVM_ERROR, sev);
    set_report_severity_override(UVM_FATAL, sev);
  }

  void try_all_severities()
  {
    UVM_INFO("INFO1", "first info message", UVM_NONE);
    UVM_WARNING("WARNING1", "first warning message");
    UVM_ERROR("ERROR1", "first error message");
    UVM_FATAL("FATAL1", "first fatal message");
    UVM_INFO("INFO2", "second info message", UVM_NONE);
    UVM_WARNING("WARNING2", "second warning message");
    UVM_ERROR("ERROR2", "second error message");
    UVM_FATAL("FATAL2", "second fatal message");
  }
};


int sc_main(int, char*[])
{
  pass = false;
  run_test("test");
  return 0;
}
