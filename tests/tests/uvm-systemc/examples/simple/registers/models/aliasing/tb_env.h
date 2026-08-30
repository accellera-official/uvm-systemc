//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2004-2011 Synopsys, Inc.
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

#ifndef TB_ENV_H_
#define TB_ENV_H_

#include <systemc>
#include <uvm>

#include "reg_agent.h"
#include "regmodel.h"

class dut
{
 public:

  static sc_dt::sc_bv<8> F1; // = 0
  static sc_dt::sc_bv<8> F2; // = 0
  static sc_dt::sc_bv<8> nulls; // = 0

  static void rw( reg_rw& rw )
  {
    switch (rw.addr.to_uint())
    {
      case 0x0: // Ra
      {
        if (rw.read)
          rw.data = (nulls, F2, nulls, F1);
        else
          if (rw.byte_en[0].to_bool())
            F1 = rw.data(7,0);
        break;
      }

      case 0x100: // Rb
      {
        if (rw.read)
          rw.data = (nulls, F2, nulls, F1);
        else
        {
          if (rw.byte_en[0].to_bool())
            F1 = rw.data(7,0);

          if (rw.byte_en[2].to_bool())
            F2 = rw.data(23,16);
        }
        break;
      }
    } // switch

    std::cout << "dut: " << rw.convert2string() << std::endl;
  }
}; // dut

// initialize static data members of class dut
sc_dt::sc_bv<8> dut::F1 = 0;
sc_dt::sc_bv<8> dut::F2 = 0;
sc_dt::sc_bv<8> dut::nulls = 0;


class tb_env : public uvm::uvm_env
{
 public:

  UVM_COMPONENT_UTILS(tb_env);

  block_B*   regmodel;
  reg_agent<dut>* bus;
  uvm::uvm_reg_predictor<reg_rw>* predict;
  reg2rw_adapter* reg2rw;

  tb_env( uvm::uvm_component_name name = "tb_env")
  : uvm::uvm_env(name),
    regmodel(nullptr),
    bus(nullptr),
    predict(nullptr),
    reg2rw(nullptr)
  {}

  void build_phase(uvm::uvm_phase& phase) override
  {
    uvm::uvm_env::build_phase(phase);

    regmodel = block_B::type_id::create("regmodel");
    regmodel->build();
    regmodel->lock_model();

    bus = reg_agent<dut>::type_id::create("bus", this);

    predict = uvm::uvm_reg_predictor<reg_rw>::type_id::create("predict", this);

    predict->set_report_verbosity_level(uvm::UVM_FULL);
  }

  void connect_phase(uvm::uvm_phase& phase) override
  {
    reg2rw  = new reg2rw_adapter("reg2rw");

    regmodel->default_map->set_sequencer(bus->sqr, reg2rw);

    predict->map = regmodel->default_map;
    predict->adapter = reg2rw;
    bus->mon->ap.connect(predict->bus_in);
    regmodel->default_map->set_auto_predict(false);
  }

  ~tb_env() override
  {
    delete reg2rw;
  }

};

#endif // TB_ENV_H_
