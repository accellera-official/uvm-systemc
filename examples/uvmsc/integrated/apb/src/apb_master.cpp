// 
// -------------------------------------------------------------
//    Copyright 2021 NXP B.V.
//    Copyright 2004-2011 Synopsys, Inc.
//    Copyright 2010 Mentor Graphics Corporation
//    Copyright 2010-2011 Cadence Design Systems, Inc.
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

#include "../inc/apb_master.h"
#include "../inc/apb_rw.h"
#include "../inc/apb_if.h"
#include "../inc/apb_agent.h"

apb_master::apb_master(uvm::uvm_component_name name) :
    uvm::uvm_driver<apb_rw>(name),
    sigs(nullptr)
{}

void apb_master::build_phase(uvm::uvm_phase & phase)
{
    uvm::uvm_driver<apb_rw>::build_phase(phase);

    apb_agent* agent = dynamic_cast<apb_agent*>(get_parent());
    if (agent) {
        sigs = agent->vif;
    }
    else {
        if (!uvm::uvm_config_db<apb_if*>::get(this, "", "vif", sigs)) {
            UVM_FATAL("APB/DRV/NOVIF", "No virtual interface specified for this driver instance");
        }
    }
}

void apb_master::run_phase(uvm::uvm_phase & phase)
{
    this->sigs->psel = sc_dt::SC_LOGIC_0;
    this->sigs->penable = sc_dt::SC_LOGIC_0;

    while (1) {
        sc_core::wait(this->sigs->pclk.posedge_event());

        auto tr = this->seq_item_port->get_next_item();

        sc_core::wait(this->sigs->pclk.posedge_event());

        switch (tr->kind_e) {
            case READ : this->read(tr->addr, tr->data); break;
            case WRITE : this->write(tr->addr, tr->data); break;
        }

        this->seq_item_port->item_done();
        this->trig.notify();
    }
}

void apb_master::read(const sc_dt::sc_lv<32> & addr, sc_dt::sc_lv<32> & data)
{
    this->sigs->paddr = addr;
    this->sigs->pwrite = sc_dt::SC_LOGIC_0;
    this->sigs->psel = sc_dt::SC_LOGIC_1;
    sc_core::wait(this->sigs->pclk.posedge_event());
    this->sigs->penable = sc_dt::SC_LOGIC_1;
    sc_core::wait(this->sigs->pclk.posedge_event());
    data = this->sigs->prdata;
    this->sigs->psel = sc_dt::SC_LOGIC_0;
    this->sigs->penable = sc_dt::SC_LOGIC_0;
}

void apb_master::write(const sc_dt::sc_lv<32> & addr, const sc_dt::sc_lv<32> & data)
{
    this->sigs->paddr = addr;
    this->sigs->pwdata = data;
    this->sigs->pwrite = sc_dt::SC_LOGIC_1;
    this->sigs->psel = sc_dt::SC_LOGIC_1;
    sc_core::wait(this->sigs->pclk.posedge_event());
    this->sigs->penable = sc_dt::SC_LOGIC_1;
    sc_core::wait(this->sigs->pclk.posedge_event());
    this->sigs->psel = sc_dt::SC_LOGIC_0;
    this->sigs->penable = sc_dt::SC_LOGIC_0;
}
