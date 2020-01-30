//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   All Rights Reserved
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

//----------------------------------------------------------------------
// This example shows the jumping mechanism
//----------------------------------------------------------------------

#include <systemc>
#include <uvm>


using namespace sc_core;
using namespace uvm;

// Create topology:
//
//          t1 (top)
//       |            |
//     a1(A)         a2(A)
//    |     |      |      |
//   b1(B) d1(D)  b1(B)  d1(D)


// Class D has no run phases

class D : public uvm_component
{
 public:
  // constructor
  D(uvm_component_name name) : uvm_component(name) {}

  void build_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void connect_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void end_of_elaboration_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void start_of_simulation_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void extract_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void check_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void report_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() << std::endl;
  }

  void final_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() << std::endl;
  }
};

// Class B has run phases
class B : public uvm_component
{
 public:
  // constructor
  B(uvm_component_name name) : uvm_component(name)
  {
    delay = sc_time(1.1, SC_MS);
    jump_once = true;
  }

  void build_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void connect_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void end_of_elaboration_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void start_of_simulation_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void extract_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void check_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void report_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void run_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " start phase: " << phase.get_name() << std::endl;
    wait(delay);
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " end phase: " << phase.get_name() << std::endl;
  }

  void pre_reset_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void reset_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_reset_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void pre_configure_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void configure_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_configure_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void pre_main_phase(uvm_phase& phase)
  {
    if (jump_once)
    {
      jump_once = false;
      phase.raise_objection(this);

      std::cout << sc_time_stamp() << ": " << get_full_name()
                << " phase: " << phase.get_name() <<  std::endl;

      // This will clear the objection
      UVM_INFO("TEST", "Jumping back to reset phase...", UVM_NONE);
      phase.jump(uvm_reset_phase::get());
    }
  }

  void main_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_main_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void pre_shutdown_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void shutdown_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_shutdown_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  // member data objects
  sc_time delay;
  bool jump_once;
};

// Class A has run phase and contains subcomponents
class A : public uvm_component
{
 public:
  // constructor
  A(uvm_component_name name) : uvm_component(name)
  {
    b1 = new B("b1");
    d1 = new D("d1");
    delay = sc_time(5.5, SC_MS);
  }

  void build_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void connect_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void end_of_elaboration_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void start_of_simulation_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void extract_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void check_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void report_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() << std::endl;
  }

  void final_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() << std::endl;
  }

  void run_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " start phase: " << phase.get_name() << std::endl;
    wait(delay);
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " end phase: " << phase.get_name() << std::endl;
  }

  void pre_reset_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void reset_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_reset_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void pre_configure_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void configure_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_configure_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void pre_main_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void main_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_main_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void pre_shutdown_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void shutdown_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_shutdown_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void phase_started(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase started: " << phase.get_name() <<  std::endl;
  }

  void phase_ended(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase ended: " << phase.get_name() <<  std::endl;
  }

  // destructor
  virtual ~A()
  {
    delete b1;
    delete d1;
  }

  // member data objects
  B* b1;
  D* d1;
  sc_time delay;
};

class AA : public uvm_component
{
 public:
  A* a;

  // constructor
  AA(uvm_component_name name) : uvm_component(name)
  {
    a = new A("a");
  }

  // destructor
  virtual ~AA()
  {
    delete a;
  }
};


// Top level contains two A components
class top : public uvm_env
{
 public:
  // constructor
  top(uvm_component_name name) : uvm_env(name)
  {
    a1 = new AA("a1");
    a2 = new AA("a2");
  }

  void build_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() <<  std::endl;
  }

  void connect_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void end_of_elaboration_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void start_of_simulation_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void extract_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void check_phase(uvm_phase& phase)
  {
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " phase: " << phase.get_name() << std::endl;
  }

  void report_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() << std::endl;
  }

  void final_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() << std::endl;
  }

  void run_phase(uvm_phase& phase)
  {
    phase.raise_objection(this);
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " start phase: " << phase.get_name() << std::endl;
    wait(50, SC_MS);
    std::cout << sc_time_stamp() << ": " << get_full_name()
              << " end phase: " << phase.get_name() << std::endl;
    phase.drop_objection(this);
  }

  void pre_reset_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void reset_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_reset_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void pre_configure_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void configure_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_configure_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void pre_main_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void main_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_main_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void pre_shutdown_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void shutdown_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  void post_shutdown_phase(uvm_phase& phase)
  {
     std::cout << sc_time_stamp() << ": " << get_full_name()
               << " phase: " << phase.get_name() <<  std::endl;
  }

  // destructor
  virtual ~top()
  {
    delete a1;
    delete a2;
  }

  // member data objects
  AA* a1;
  AA* a2;
};

int sc_main(int, char*[])
{
  // instantiate top
  top* t1 = new top("t1");

  //uvm_set_verbosity_level(UVM_DEBUG);

  run_test();

  delete t1;

  return 0;
}
