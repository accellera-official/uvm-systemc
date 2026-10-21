// 
// -------------------------------------------------------------
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

#include "../inc/tb_env.h"
#include "../inc/tb_test.h"

int sc_main(int argc, char* argv[])
{
    tb_env* env = new tb_env("env");
    tb_test* test = new tb_test("test");

    uvm::uvm_coreservice_t* cs_ = uvm::uvm_coreservice_t::get();
    auto svr = cs_->get_report_server();
    svr->set_max_quit_count(10);

    uvm::run_test();

    delete test;
    delete env;

    return 0;
}
