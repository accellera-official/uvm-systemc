//----------------------------------------------------------------------
//   Copyright 2004-2011 Synopsys, Inc.
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2015-2016 Agnisys, Inc.
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

#include <systemc>
#include <uvm>

#include "tb_env.h"
#include "tb_test.h"

//----------------------------------------------------------------------
// This example demonstrates how to model a register containing
// read-only fields and a register containing write-only fields
// at the same physical address.
//----------------------------------------------------------------------

int sc_main(int, char*[]) 
{  
  tb_env* env;
  tb_test* test;

  uvm::uvm_root::get()->set_report_verbosity_level(uvm::UVM_FULL);
  uvm::uvm_report_server::get_server()->set_max_quit_count(10);

  env = new tb_env("env");
  test = new tb_test("test");

  uvm::run_test();

  return 0;
}
