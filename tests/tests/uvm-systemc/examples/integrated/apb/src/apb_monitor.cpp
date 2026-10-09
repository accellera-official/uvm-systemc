// 
// -------------------------------------------------------------
//    Copyright 2021 NXP B.V.
//    Copyright 2004-2011 Synopsys, Inc.
//    Copyright 2010-2011 Mentor Graphics Corporation
//    All Rights Reserved Worldwide
// 
//    Licensed under the Apache License, Version 2.0 (the
//    "License"); you may not use this file except in
//    compliance with the License.  You may obtain a copy of
//    the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in
//    writing, software distributed under the License is
//    distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//    CONDITIONS OF ANY KIND, either express or implied.  See
//    the License for the specific language governing
//    permissions and limitations under the License.
// -------------------------------------------------------------
// 

#include <systemc>
#include <uvm>

#include "../inc/apb_monitor.h"
#include "../inc/apb_rw.h"
#include "../inc/apb_if.h"
#include "../inc/apb_agent.h"

apb_monitor::apb_monitor(uvm::uvm_component_name name) :
    uvm::uvm_monitor(name),
    sigs(nullptr)
{}

void apb_monitor::build_phase(uvm::uvm_phase & phase)
{
    uvm::uvm_monitor::build_phase(phase);

    apb_agent* agent = dynamic_cast<apb_agent*>(get_parent());
    if (agent) {
        sigs = agent->vif;
    }
    else {
        if (!uvm::uvm_config_db<apb_if*>::get(this, "", "vif", sigs)) {
            UVM_FATAL("APB/MON/NOVIF", "No virtual interface specified for this monitor instance");
        }
    }
}

void apb_monitor::run_phase(uvm::uvm_phase & phase)
{
    while (1) {
        do {
            sc_core::wait(this->sigs->pclk.posedge_event());
        }
        while ( this->sigs->psel != sc_dt::SC_LOGIC_1 ||
                this->sigs->penable != sc_dt::SC_LOGIC_0);

        auto tr = apb_rw::type_id::create_handle("tr", this);
        tr->kind_e = (this->sigs->pwrite == sc_dt::SC_LOGIC_1) ? WRITE : READ;
        tr->addr = this->sigs->paddr;

        sc_core::wait(this->sigs->pclk.posedge_event());

        if (this->sigs->penable != sc_dt::SC_LOGIC_1) {
            UVM_ERROR("APB", "APB protocol violation: SETUP cycle not followed by ENABLE cycle");
        }

        tr->data = (tr->kind_e == READ) ? this->sigs->prdata : this->sigs->pwdata;
        ap.write(tr);
    }
}
