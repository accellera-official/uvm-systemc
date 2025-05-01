//----------------------------------------------------------------------
//   Copyright 2021 NXP B.V.
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
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

#include <string>
#include <cstdarg>
#include <random>

#include "reg.h"

using namespace uvm;
using namespace sc_core;
using namespace sc_dt;

class tbtest : public sc_module
{
public:
  class my_catcher;
  class top_;

  class dut_ : public sc_module
  {
  public:

    // Dummy Registers
    sc_vector< reg<sc_uint<64> > > mem{"mem", 8};
    
    // random number generator
    std::mt19937_64 rng{0};

    dut_(sc_module_name nm)
    {
      for(int i = 0; i < 8; i++)
      {
        mem[i]=rng();
        std::cout << "Mem[" << i << "] = " << std::hex << mem[i] << std::endl;
      }
    }

    void print_mem()
    {
      for(int i = 0; i < 8; i++)
        std::cout << "Mem[" << i << "] = " << std::hex << mem[i] << std::endl;
    }

    void reset()
    {
      for(int i = 0; i < 8; i++)
        mem[i] = 0;
    }

    void end_of_simulation() override
    {
      print_mem();
    }
  }; // class dut_

  class top_ : public sc_module
  {
  public:
    top_(sc_module_name nm) :
      sc_module(nm),
      dut("dut")
    {}

    dut_ dut;
  }; // class top_

  //-------------- Register Definitions ----------------

  class mem_type : public uvm_mem
  {
  public:

    mem_type( const std::string& name = "unnamed-mem_type" )
    : uvm_mem(name, 0x8, 64, "RW", UVM_NO_COVERAGE) {}

    UVM_REGISTER_CB(mem_type, uvm_reg_cbs);
    //UVM_SET_SUPER_TYPE(mem_type, uvm_mem);
    UVM_OBJECT_UTILS(mem_type);

    virtual void build() {}
}; // class mem_type


  class mmap_type : public uvm_reg_block
  {
  public:
    /* rand */ mem_type* mem{nullptr};

    mmap_type(const std::string& name = "unnamed-mmap_type")
    : uvm_reg_block(name, UVM_NO_COVERAGE) {}

    ~mmap_type() override {
      if (mem) {
        delete mem;
      }
    }

    UVM_OBJECT_UTILS(mmap_type);

    void build()
    {
      // Now define address mappings
      default_map = create_map("default_map", 0, 8, UVM_LITTLE_ENDIAN);

      mem = mem_type::type_id::create("mem");
      mem->build();
      mem->configure(this, "mem");

      default_map->add_mem(mem, 0x100, "RW");
      set_hdl_path_root("tbtest.top.dut");

      this->lock_model();
    }
  }; // class mmap_type


  //-------------- Register Definition Ends Here ----------------

  // User register sequence
  class user_test_seq : public uvm_reg_sequence<>
  {
  public:
    // The register model on which the sequence work
    mmap_type* model{nullptr};

    user_test_seq( const std::string& name = "user_test_seq")
    : uvm_reg_sequence(name) {}

    UVM_OBJECT_UTILS(user_test_seq);

    // Drive all registers inside model
    void body() override
    {
      uvm_status_e status;
      sc_uint<64> data;

      assert(model);

      UVM_INFO("TEST_SEQ", "<><><><><><><><><><><><><><><><><><><><><><><>", UVM_LOW);
      UVM_INFO("TEST_SEQ", "  Starting Test Sequence", UVM_LOW);
      UVM_INFO("TEST_SEQ", "<><><><><><><><><><><><><><><><><><><><><><><>\n", UVM_LOW);

      // Drive all memory elements frontdoor
      for(int idx = 0; idx < 8; idx++)
      {
        for(int j=0; j < 8; j++)
          data.range(j*8+7, j*8) = (idx*8) + j;

        model->mem->write(status, idx, data, UVM_FRONTDOOR, nullptr, this);
      }

      top->dut.print_mem();
      top->dut.reset();

      // Drive all memory elements backdoor
      for(int idx = 0; idx < 8; idx++)
      {
        for(int j = 0; j < 8; j++)
          data.range(j*8 + 7, j*8) = (idx*8) + j;

        model->mem->write(status, idx, data, UVM_BACKDOOR, nullptr, this);
      }

    }
  }; // class user_test_seq

  class user_transaction : public uvm_sequence_item
  {
  public:
    /* rand */ sc_uint<32> addr;
    /* rand */ sc_uint<64> data;
    /* rand */ bool r_wn;

    user_transaction( const std::string& name = "unnamed-user_transaction")
    : uvm_sequence_item(name) {}

    void do_print(const uvm::uvm_printer& printer) const override {
      uvm_sequence_item::do_print(printer);
      printer.print_field_int("addr", addr);
      printer.print_field_int("data", data);
      printer.print_field_int("r_wn", r_wn);
    }

    UVM_OBJECT_UTILS(user_transaction);
  }; // class user_transaction

  //  `uvm_blocking_put_imp_decl(_reg)

  class user_sequencer : public uvm_sequencer<user_transaction>
  {
  public:
    UVM_COMPONENT_UTILS(user_sequencer);

    user_sequencer( uvm_component_name name)
    : uvm_sequencer<user_transaction>(name) {}
  }; // class user_sequencer


  class user_driver : public uvm_driver<user_transaction>
  {
  public:

    user_driver(uvm_component_name name)
    : uvm_driver<user_transaction>(name) {}

    UVM_COMPONENT_UTILS(user_driver);

    void run_phase(uvm_phase & phase) override
    {
      user_transaction req;

      while(true)
      {
        seq_item_port.get_next_item(req);
        wait(1, SC_US);
        UVM_INFO("USRDRV", "Received following transaction :\n" +  req.sprint(), UVM_LOW);

        if(!req.r_wn)
          top->dut.mem[(req.addr-0x100)/8] = req.data;
        else
          req.data = top->dut.mem[(req.addr-0x100)/8].read();

        seq_item_port.item_done();
      }
    }
  }; // class user_driver

  class reg2bus_adapter : public uvm_reg_adapter
  {
  public:
     uvm_sequence_item* reg2bus(const uvm_reg_bus_op & rw) override
    {
      user_transaction* txn = user_transaction::type_id::create("txn");
      txn->r_wn = (rw.kind == UVM_READ) ? 1 : 0;
      txn->addr = rw.addr;
      txn->data = rw.data;
      return txn;
    }

     void bus2reg(const uvm_sequence_item* bus_item, uvm_reg_bus_op & rw) override
    {
      const user_transaction* txn = dynamic_cast<const user_transaction*>(bus_item);
      if (!txn)
      {
        UVM_FATAL("NOT_TXN_TYPE","Provided bus_item not correct type");
      }

      rw.kind = txn->r_wn ? UVM_READ : UVM_WRITE;
      rw.addr = txn->addr;
      rw.data = txn->data;
      rw.status = UVM_IS_OK;
    }

    UVM_OBJECT_UTILS(reg2bus_adapter);

    reg2bus_adapter( std::string name = "reg2bus_adapter") : uvm_reg_adapter(name)
    {}
  }; // class reg2bus_adapter

  class test : public uvm_test
  {
  public:
    mmap_type*      model;
    user_sequencer* sqr;
    user_driver*    drv;
    user_test_seq*  seq;

     void build_phase(uvm_phase & phase) override
    {
      uvm_test::build_phase(phase);
      // Create register model
      model = mmap_type::type_id::create("model",this);
      model->build();
      sqr = user_sequencer::type_id::create("sqr", this);
      drv = user_driver::type_id::create("drv", this);
    }

     void connect_phase(uvm_phase & phase) override
    {
      // Set model's sequencer and adapter sequence
      reg2bus_adapter* reg2bus = new reg2bus_adapter();
      model->default_map->set_sequencer(sqr, reg2bus);
      drv->seq_item_port.connect(sqr->seq_item_export);
    }

    void end_of_elaboration_phase(uvm_phase & phase) override
    {
      uvm_default_printer = uvm_default_tree_printer;
      this->print();
      model->print();
    }

    void run_phase(uvm_phase & phase) override
    {
      phase.raise_objection(this);
      // Create register sequence
      seq = user_test_seq::type_id::create("user_test_seq", this);
      // Set sequence's container
      seq->model = model;
      // Procedurally start sequence
      seq->start(nullptr);
      phase.drop_objection(this);
    }

    UVM_COMPONENT_UTILS(test);

    test( uvm_component_name name) : uvm_test(name)
    {}

     void report_phase(uvm_phase & phase) override
    {
      uvm_coreservice_t* cs_;
      uvm_report_server* svr;
      cs_ = uvm_coreservice_t::get();
      svr =  cs_->get_report_server();

      if (svr->get_severity_count(UVM_FATAL) +
          svr->get_severity_count(UVM_ERROR) +
          svr->get_severity_count(UVM_WARNING) == 0)
        std::cout << "** UVM TEST PASSED **" << std::endl;
      else
        std::cout << "** UVM TEST FAILED **" << std::endl;
    }
  }; // class test

  class my_catcher : public uvm_report_catcher
  {
  public:
    UVM_OBJECT_UTILS(my_catcher);

    my_catcher(const std::string & name = "unnamed-my_catcher") :
      uvm_report_catcher(name)
    {}

     action_e do_catch() override
    {
      return THROW;
    }
  }; // class my_catcher

  tbtest(sc_module_name nm) :
    sc_module(nm)
  {
    catcher = my_catcher::type_id::create("catcher");
    uvm_report_cb::add(nullptr, catcher);
    top = new top_("top");
  }

  ~tbtest() override {
    if (catcher) {
      delete catcher;
    }
    if (top) {
      delete top;
    }
  }

  my_catcher* catcher{nullptr};
  static top_* top;

}; // class tbtest

tbtest::top_* tbtest::top = nullptr;


int sc_main(int argc, char* argv[])
{
  tbtest tb("tbtest");

  run_test("test");

  return 0;
}
