//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2004-2011 Synopsys, Inc.
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2010-2011 Cadence Design Systems, Inc.
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

#ifndef TB_TEST_H_
#define TB_TEST_H_

#include <systemc>
#include <uvm>

#include "regmodel.h"
#include "tb_env.h"

class tb_test : public uvm::uvm_test
{
 public:
  tb_env* env;
  uvm::uvm_reg_sequence<>* seq;

  tb_test( uvm::uvm_component_name name = "tb_test")
  : uvm::uvm_test(name), env(NULL), seq(NULL) {}

  UVM_COMPONENT_UTILS(tb_test);

  void build_phase(uvm::uvm_phase& phase)
  {
    uvm::uvm_test::build_phase(phase);

    env = tb_env::type_id::create("tb_env");
    seq = uvm::uvm_reg_bit_bash_seq::type_id::create("seq");
  }

  void run_phase(uvm::uvm_phase& phase)
  {
 /* TODO
    uvm::uvm_status_e status;
 */

    phase.raise_objection(this);

    env->regmodel->reset();
    seq->model = env->regmodel;
    seq->start(env->bus->sqr);
    seq->wait_for_sequence_state(uvm::UVM_FINISHED);

 /* TODO
    UVM_INFO("Test", "Verifying aliasing...", uvm::UVM_NONE);

    env->regmodel->Ra->write(status, 0xDEADBEEF, uvm::UVM_DEFAULT_PATH, NULL, seq);
    env->regmodel->mirror(status, uvm::UVM_CHECK, uvm::UVM_DEFAULT_PATH, seq);

    env->regmodel->Rb->write(status, 0x87654320, uvm::UVM_DEFAULT_PATH, NULL, seq);
    env->regmodel->mirror(status, uvm::UVM_CHECK, uvm::UVM_DEFAULT_PATH, seq);

    env->regmodel->Ra->F1->write(status, 0xA5, uvm::UVM_DEFAULT_PATH, NULL, seq);
    env->regmodel->mirror(status, uvm::UVM_CHECK, uvm::UVM_DEFAULT_PATH, seq);

    env->regmodel->Rb->F1->write(status, 0xC3, uvm::UVM_DEFAULT_PATH, NULL, seq);
    env->regmodel->mirror(status, uvm::UVM_CHECK, uvm::UVM_DEFAULT_PATH, seq);

    env->regmodel->Ra->F2->write(status, 0xBD, uvm::UVM_DEFAULT_PATH, NULL, seq);
    env->regmodel->mirror(status, uvm::UVM_CHECK, uvm::UVM_DEFAULT_PATH, seq);

    env->regmodel->Rb->F2->write(status, 0x2A, uvm::UVM_DEFAULT_PATH, NULL, seq);
    env->regmodel->mirror(status, uvm::UVM_CHECK, uvm::UVM_DEFAULT_PATH, seq);
*/
    phase.drop_objection(this);
  }
};

#endif // TB_TEST_H_
