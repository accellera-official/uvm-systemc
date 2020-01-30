//----------------------------------------------------------------------
//   Copyright 2004-2011 Synopsys, Inc.
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2013-2014 NXP B.V.
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
//   permissions and limitations under t,he License.
//----------------------------------------------------------------------

#ifndef TB_TEST_H_
#define TB_TEST_H_

#include <systemc>
#include <uvm>

#include "regmodel.h"
#include "tb_env.h"

class tb_test : public uvm::uvm_test
{
 public:
  tb_test( uvm::uvm_component_name name ) : uvm::uvm_test(name)
  {
    set_report_verbosity_level(uvm::UVM_FULL);
  }

  UVM_COMPONENT_UTILS(tb_test);

  virtual void run_phase( uvm::uvm_phase& phase )
  {
    tb_env* env;
    uvm::uvm_status_e   status;
    uvm::uvm_reg_data_t data;

    phase.raise_objection(this);

    env = dynamic_cast<tb_env*>(uvm::uvm_root::get()->find("env"));

    if ( env == NULL )
      UVM_FATAL("test", "Cannot find tb_env");

    UVM_INFO("Test", "Reset RegModel...", uvm::UVM_NONE);
    env->regmodel->reset();

    uvm::uvm_sequence_base* seq = new uvm::uvm_sequence_base("seq");

    UVM_INFO("Test", "Demonstrating RO/WO sharing...", uvm::UVM_NONE);

    env->regmodel->R->read(status, data, uvm::UVM_DEFAULT_PATH, NULL, seq);
    env->regmodel->W->write(status, 0xDEADBEEF, uvm::UVM_DEFAULT_PATH, NULL, seq);

    UVM_INFO("Test", "Done.", uvm::UVM_NONE);

    delete seq;

    phase.drop_objection(this);
  }

}; // class tb_test

#endif // TB_TEST_H_
