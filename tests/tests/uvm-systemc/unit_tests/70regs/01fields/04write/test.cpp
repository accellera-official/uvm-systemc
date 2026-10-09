//----------------------------------------------------------------------
// Copyright 2026 COSEDA Technologies GmbH
// Copyright 2021 Intel Corporation
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
#include <map>
#include <sstream>

#include "reg_agent.h"

using namespace uvm;

class dut_rw
{
 public:
  static std::map<uvm_reg_addr_t, uvm_reg_data_t> dut_val;

  static void rw( field_reg_rw& rw )
  {
    if (rw.read)
      rw.data = dut_val[rw.addr.to_uint64()];
    else
      dut_val[rw.addr.to_uint64()] = rw.data;

    UVM_INFO("DUT_VIEW", rw.convert2string(), UVM_LOW);
    sc_core::wait(10, sc_core::SC_NS);
  }
};

std::map<uvm_reg_addr_t, uvm_reg_data_t> dut_rw::dut_val;

class regA : public uvm_reg
{
 public:
  /* rand */ uvm_reg_field* fA1;

  UVM_OBJECT_UTILS(regA);

  regA( const std::string& name = "regA" )
  : uvm_reg(name, 64, UVM_NO_COVERAGE), fA1(nullptr)
  {}

  virtual void build()
  {
    fA1 = uvm_reg_field::type_id::create("fA1");
    fA1->configure(this, 32, 0, "RW", false, 0, true, true, true);
  }
};

class blk1_reg_block : public uvm_reg_block
{
 public:
  /* rand */ regA* r1i1;
  uvm_reg_map* map1;

  UVM_OBJECT_UTILS(blk1_reg_block);

  blk1_reg_block( const std::string& name = "blk1_reg_block" )
  : uvm_reg_block(name, UVM_NO_COVERAGE), r1i1(nullptr), map1(nullptr)
  {}

  virtual void build()
  {
    r1i1 = regA::type_id::create("r1i1");
    r1i1->configure(this, nullptr, "");
    r1i1->build();

    map1 = create_map("map1", 0, 4, UVM_LITTLE_ENDIAN, true);
    map1->set_auto_predict(true);
    map1->add_reg(r1i1, 0, "RW", false);
  }
};

class tb_env : public uvm_env
{
 public:
  blk1_reg_block* reg_model;
  reg_agent<dut_rw>* bus;
  reg2rw_adapter reg2rw;

  UVM_COMPONENT_UTILS(tb_env);

  tb_env( uvm_component_name name )
  : uvm_env(name), reg_model(nullptr), bus(nullptr), reg2rw("reg2rw")
  {}

  virtual void build_phase( uvm_phase& phase )
  {
    if (reg_model == nullptr)
    {
      reg_model = blk1_reg_block::type_id::create("reg_model", this);
      reg_model->build();
      reg_model->lock_model();
      reg_model->reset();
      reg_model->print();
      bus = reg_agent<dut_rw>::type_id::create("bus", this);
    }
  }

  virtual void connect_phase( uvm_phase& phase )
  {
    uvm_env::connect_phase(phase);
    reg_model->default_map->set_sequencer(bus->sqr, &reg2rw);
  }
};

class test : public uvm_test
{
 public:
  tb_env* env;

  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name )
  : uvm_test(name), env(nullptr)
  {}

  virtual void build_phase( uvm_phase& phase )
  {
    env = tb_env::type_id::create("env", this);
  }

  void check_desired_mirrored( uvm_reg_field* f, uvm_reg_data_t exp )
  {
    uvm_reg_data_t des = f->get();
    uvm_reg_data_t mir = f->get_mirrored_value();
    if (des != mir)
    {
      std::ostringstream str;
      str << "Desired and mirrored value do not match. Desired value=[h"
          << std::hex << des.to_uint64() << "], Mirrored value=[h"
          << mir.to_uint64() << "]";
      UVM_ERROR("DATA_MISMATCH", str.str());
    }
    if (mir != exp)
    {
      std::ostringstream str;
      str << "Mirrored value does not match expected value. Expected value=[h"
          << std::hex << exp.to_uint64() << "], Mirrored value=[h"
          << mir.to_uint64() << "]";
      UVM_ERROR("DATA_MISMATCH", str.str());
    }
  }

  virtual void run_phase( uvm_phase& phase )
  {
    uvm_status_e status;
    uvm_reg_data_t wr_value = 0x1eaf;
    uvm_reg_data_t rd_value;
    uvm_reg_field* fld_under_test = env->reg_model->r1i1->fA1;

    phase.raise_objection(this);

    UVM_INFO("TEST", "Performing field write", UVM_LOW);
    fld_under_test->write(status, wr_value);
    check_desired_mirrored(fld_under_test, wr_value);

    UVM_INFO("TEST", "Performing field read", UVM_LOW);
    fld_under_test->read(status, rd_value);
    check_desired_mirrored(fld_under_test, rd_value);

    if (rd_value != wr_value)
    {
      std::ostringstream str;
      str << "Read value mismatched previously write value. Read value=[h"
          << std::hex << rd_value.to_uint64() << "], Write value=[h"
          << wr_value.to_uint64() << "]";
      UVM_ERROR("DATA_MISMATCH", str.str());
    }

    phase.drop_objection(this);
  }

  virtual void report_phase( uvm_phase& phase )
  {
    uvm_test::report_phase(phase);
    uvm_report_server* svr = uvm_coreservice_t::get()->get_report_server();
    if (svr->get_severity_count(UVM_FATAL) +
        svr->get_severity_count(UVM_ERROR) == 0)
      std::cout << "** UVM TEST PASSED **" << std::endl;
    else
      std::cout << "!! UVM TEST FAILED !!" << std::endl;
    std::cout << "UVM TEST EXPECT 0 UVM_ERROR" << std::endl;
  }
};

int sc_main(int, char*[])
{
  run_test("test");
  return 0;
}
