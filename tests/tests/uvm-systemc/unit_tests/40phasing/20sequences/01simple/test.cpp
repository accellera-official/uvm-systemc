//---------------------------------------------------------------------- 
//   Copyright 2016-2021 NXP B.V.
//   Copyright 2010-2018 Cadence Design Systems, Inc.
//   Copyright 2011 Mentor Graphics Corporation
//   Copyright 2013 NVIDIA Corporation
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

// Test the simple setting of default sequences for a couple of
// different phases, configure and main.

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;
using namespace sc_core;
using namespace std;

class myseq : public uvm_sequence<>
{
 public:

  static int start_cnt, end_cnt;

  UVM_OBJECT_UTILS(myseq);

  void body() override
  {
    start_cnt++;
    UVM_INFO("INBODY", "Starting myseq!!!", UVM_NONE);
    sc_core::wait(10, SC_MS);
    UVM_INFO("INBODY", "Ending myseq!!!", UVM_NONE);
    end_cnt++;
  }

  myseq( string name = "myseq") : uvm_sequence<>(name)
  {
    set_automatic_phase_objection(true);
  }
};

// initialize static members
int myseq::start_cnt = 0;
int myseq::end_cnt = 0;

class myseqr : public uvm_sequencer<>
{
public:
  myseqr( uvm_component_name name) : uvm_sequencer<>(name)
  {}

  UVM_COMPONENT_UTILS(myseqr);

  void main_phase(uvm_phase& phase) override
  {
    UVM_INFO("MAIN","In main!!!", UVM_NONE);
    sc_core::wait(100, SC_MS);
    UVM_INFO("MAIN","Exiting main!!!", UVM_NONE);
  }
};

class test : public uvm_test
{
public:
  myseqr* seqr;

  test( uvm_component_name name = "my_comp" ) : uvm_test(name), seqr(nullptr)
  {}

  UVM_COMPONENT_UTILS(test);

  void build_phase(uvm_phase& phase) override
  {
    seqr = myseqr::type_id::create("seqr");
    uvm_config_db<uvm_object_wrapper*>::set(this, "seqr.configure_phase", "default_sequence", myseq::type_id::get());
    uvm_config_db<uvm_object_wrapper*>::set(this, "seqr.main_phase", "default_sequence", myseq::type_id::get());
  }

  void report_phase(uvm_phase& phase) override
  {
    if(myseq::start_cnt != 2 && myseq::end_cnt != 2)
      cout << "*** UVM TEST FAILED ***" << endl;
    else
      cout << "*** UVM TEST PASSED ***" << endl;
  }
};


int sc_main(int, char*[])
{
  run_test("test");

  return 0;
}
