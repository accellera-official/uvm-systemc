//------------------------------------------------------------------------------
//   Copyright 2011 Cadence Design Systems, Inc.
//   Copyright 2015 NXP B.V.
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
//------------------------------------------------------------------------------

#include <systemc>
#include <uvm>

#include "rdb.h"
#include "uvc.h"
#include "dut.h"
#include "uvc_intf.h"
#include "transaction.h"

using namespace uvm;

// User register sequence
class test_seq : public uvm_reg_sequence<>
{
 public:

  // The register model on which the sequence work
  rfile0_t* model;

  // Drive all registers inside model

  void body() override
  {
    uvm_status_e status;

    UVM_INFO("TEST_SEQ", "<><><><><><><><><><><><><><><><><><><><><><><>", UVM_LOW);
    UVM_INFO("TEST_SEQ", "  Starting Test Sequence", UVM_LOW);
    UVM_INFO("TEST_SEQ", "<><><><><><><><><><><><><><><><><><><><><><><>\n", UVM_LOW);

    UVM_INFO("TEST_SEQ", "Backdoor mirror the shadow. Expect mismatch errors", UVM_LOW);

    // Directly set the mirror value to 'habcd, which is different then the DUT value of 'h0
    model->ureg0->predict(0xabcd);

    // Call mirror() to update the mirror to reflect the DUT value of 0. But UVM_CHECK says
    // to check the current mirror against the DUT value before we make that update.
    // We expect a miscompare ('habcd != 0)
    model->ureg0->mirror(status, UVM_CHECK, UVM_BACKDOOR, nullptr, dynamic_cast<uvm_sequence_base*>(this));
  }

  UVM_OBJECT_UTILS(test_seq);

  test_seq( const std::string& name = "test_seq") : uvm_reg_sequence<>(name)
  {}
}; // test_seq


class reg2uvc_adapter : public uvm_reg_adapter
{
 public:

  uvm_sequence_item* reg2bus(const uvm_reg_bus_op& rw) override
  {
    transaction* txn = transaction::type_id::create("txn");
    txn->dir = rw.kind;
    txn->addr = rw.addr;
    txn->data = rw.data;
    return txn;
  }

  void bus2reg( const uvm_sequence_item* bus_item, uvm_reg_bus_op& rw) override
  {
    const transaction* txn = dynamic_cast<const transaction*>(bus_item);
    if (txn == nullptr)
    {
      UVM_FATAL("NOT_TXN_TYPE","Provided bus_item not correct type");
      return;
    }

    rw.kind = txn->dir;
    rw.addr = txn->addr;
    rw.data = txn->data;
    rw.status = UVM_IS_OK;
  }

  UVM_OBJECT_UTILS(reg2uvc_adapter)

  reg2uvc_adapter( const std::string& name="reg2uvc_adapter") : uvm_reg_adapter(name)
  {}
};


class test : public uvm_test
{
 public:

  mmap0_type* model;
  test_seq* seq;
  uvc_env< sc_core::sc_signal<transaction>* >* uenv;

  sc_core::sc_signal<transaction>* pif;

   void build_phase( uvm_phase& phase ) override
  {
    uvm_test::build_phase(phase);

    uvm_config_db<int>::set(this, "uenv.uos", "count", 0);
    uvm_reg::include_coverage("*", UVM_CVR_ALL);

    // Create register model
    model = mmap0_type::type_id::create("model",this);
    model->build();

    // Create UVC
    uenv = uvc_env<uvc_intf*>::type_id::create("uenv", this);

    if (!uvm_config_db<uvc_intf*>::get(this, "*", "pif", pif))
      UVM_FATAL(this->name(), "Interface not found! Simulation aborted!");

  }

  void connect_phase( uvm_phase& phase ) override
  {
    // Set model's sequencer and adapter sequence
    reg2uvc_adapter* reg2uvc = new reg2uvc_adapter();
    model->default_map->set_sequencer(uenv->uos, reg2uvc);
    model->print();
    model->default_map->print();

    uenv->uod->vif = pif;
  }

  void end_of_elaboration_phase( uvm_phase& phase ) override
  {
    model->reset();
    //uvm_default_printer = uvm_default_tree_printer;
    //print();
  }

  void run_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this);

    // Create register sequence
    seq = test_seq::type_id::create("test_seq", this);

    // Set sequence's container
    seq->model = model->rfile0;

    // Procedurally start sequence
    seq->start(nullptr);

    phase.drop_objection(this);
  }

  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name) : uvm_test(name)
  {}

   void report_phase( uvm_phase& phase ) override
  {
    std::cout << "** UVM TEST PASSED **" << std::endl;
    std::cout << "UVM TEST EXPECT 1 UVM_ERROR" << std::endl;
  }
};

int sc_main(int, char*[])
{
  sc_core::sc_signal<transaction> pif;
  dut mydut("dut");
  mydut.in(pif);

  // register interfaces via configuration mechanism
  uvm::uvm_config_db<uvc_intf*>::set(0, "*", "pif", &pif);

  run_test("test");

  return 0;
}
