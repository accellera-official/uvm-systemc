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

#include "../inc/dut_top.h"
#include "../inc/tb_env.h"

int sc_main(int argc, char* argv[])
{
    dut_top* my_dut_top = new dut_top("my_dut_top");
    tb_env* my_tb_env = new tb_env("my_tb_env");
    my_tb_env->dt = my_dut_top;

    uvm::uvm_coreservice_t* cs = uvm::uvm_coreservice_t::get();

    auto svr = cs->get_report_server();
    svr->set_max_quit_count(10);

    uvm::uvm_config_db<apb_if*>::set(nullptr, "*apb*", "vif", my_dut_top->apb0);

    sc_core::sc_trace_file* tracefile = sc_core::sc_create_vcd_trace_file("fifo_reg_traces");
    sc_core::sc_trace(tracefile, my_dut_top->dut0->pclk, "pclk");
    sc_core::sc_trace(tracefile, my_dut_top->dut0->paddr, "paddr");
    sc_core::sc_trace(tracefile, my_dut_top->dut0->psel, "psel");
    sc_core::sc_trace(tracefile, my_dut_top->dut0->penable, "penable");
    sc_core::sc_trace(tracefile, my_dut_top->dut0->pwrite, "pwrite");
    sc_core::sc_trace(tracefile, my_dut_top->dut0->prdata, "prdata");
    sc_core::sc_trace(tracefile, my_dut_top->dut0->pwdata, "pwdata");
    sc_core::sc_trace(tracefile, my_dut_top->dut0->rst, "rst");

    uvm::run_test();

    sc_core::sc_close_vcd_trace_file(tracefile);

    delete svr;
    delete my_tb_env;
    delete my_dut_top;

    return 0;
}
