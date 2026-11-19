/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/

/*****************************************************************************

  uvm-systemc/simple/processes/basic/test.cpp -- Testing of dynamically spawned
                                                 processes and process list

  Original Author: Stephan Gerth, Bosch, 2019-02-07

 *****************************************************************************/

#include <systemc>
#include <uvm>

using namespace std;
using namespace sc_core;

class sub_component: public uvm::uvm_component {
public:

	void process_a() {
		while(true) {
			cout << sc_time_stamp() << " " << name() << " process_a" << endl;
			#if IEEE_1666_SYSTEMC >= 202301L
			wait(sc_event::none());
			#else
			wait(sc_event::none);
            #endif
		}
	}

	void process_b() {
		cout << sc_time_stamp() << " " << name() << " process_b - begin" << endl;
		wait(50,SC_NS);
		cout << sc_time_stamp() << " " << name() << " process_b - end" << endl;
	}

	void run_phase(uvm::uvm_phase& phase) override {
		uvm::uvm_component::run_phase(phase);

		cout << sc_time_stamp() << " " << name() << " run_phase(...) in process " << sc_get_current_process_handle().name() << endl;

		std::vector<sc_process_handle> spawned_processes;

		sc_process_handle process_a_hdl = sc_spawn(sc_bind(&sub_component::process_a, this), "process_a");
		sc_process_handle process_b_hdl = sc_spawn(sc_bind(&sub_component::process_b, this), "process_b");
		spawned_processes.push_back(process_a_hdl);
		spawned_processes.push_back(process_b_hdl);

		sc_event_or_list termination_events;
		for (std::vector<sc_process_handle>::iterator it=spawned_processes.begin(); it!=spawned_processes.end(); it++) {
			termination_events |= it->terminated_event();
		}
		cout << "waiting for any process to finish" << endl;
		sc_core::wait(termination_events);

		cout << "at least one process has finished!" << endl;

	}

	sub_component(uvm::uvm_component_name name = "uvm_top2") :
			uvm::uvm_component(name) {
	}

};

class top_component: public uvm::uvm_component {
public:

	sub_component* i_sub_component;

	void logic() {
		wait(40, SC_NS);
	}

	void build_phase(uvm::uvm_phase& phase) override {
		i_sub_component = new sub_component("sub_component");
	}

	void run_phase(uvm::uvm_phase& phase) override {
		uvm::uvm_component::run_phase(phase);

		phase.raise_objection(this);

		cout << sc_time_stamp() << " " << name() << " run_phase: spawning processes" << endl;

		SC_FORK
			sc_spawn(sc_bind(&top_component::logic, this))
		SC_JOIN

		cout << sc_time_stamp() << " " << name() << " run_phase: all spawned processes finished" << endl;
		cout << sc_time_stamp() << " " << name() << " run_phase: done" << endl;

		phase.drop_objection(this);

	}

	top_component(uvm::uvm_component_name name = "uvm_top1") :
			uvm::uvm_component(name),i_sub_component(0) {
	}

};

int sc_main(int argc, char **argv) {

	top_component i_top_component("top_component");
	uvm::run_test("");

	return 0;
}
