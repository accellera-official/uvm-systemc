//----------------------------------------------------------------------
//   Copyright 2004-2011 Synopsys, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
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

#ifndef TB_ENV_H_
#define TB_ENV_H_

#include <systemc>
#include <uvm>

#include "reg_agent.h"
#include "regmodel.h"

sc_dt::sc_bv<32> R_reg = 0x00FF0000; 
sc_dt::sc_bv<32> W_reg = 0x00CCCAA0; 
sc_dt::sc_bv<32> R_reg1 = 0x00CCC000; 
sc_dt::sc_bv<32> W_reg1 = 0x00CCCAA0;


class dut
{
    public:

    static void rw( reg_rw& rw )
    {
        switch (rw.addr.to_int())
        {
            case 0x100:
            if (rw.read)
            {
                rw.data = R_reg;
                if (rw.byte_en[2].to_bool())
                    R_reg(23,16) = 0x00;
            }
            else // write
            {
                if (rw.byte_en[0].to_bool())
                    W_reg(7,4) = rw.data(7,4);
                if (rw.byte_en[1].to_bool())
                    W_reg(15,8) = rw.data(15,8);
                if (rw.byte_en[2].to_bool())
                    W_reg(23,16) = rw.data(23,16);
            }
            break;

            case 0x200:
            if (rw.read)
            {
                rw.data = R_reg1;
                if (rw.byte_en[2].to_bool())
                    R_reg1(23,16) = 0x00;
                if (rw.byte_en[1].to_bool())
                    R_reg1(15,12) = 0x0;
            }
            else // write
            {
                if (rw.byte_en[0].to_bool())
                    W_reg1(7,4) = rw.data(7,4);
                if (rw.byte_en[1].to_bool())
                    W_reg1(15,8) = rw.data(15,8);
                if (rw.byte_en[2].to_bool())
                    W_reg1(23,16) = rw.data(23,16);
            }
            break;

            case 0x400:
            if (rw.read)
            {
                rw.data = R_reg;
                if (rw.byte_en[0].to_bool())
                    R_reg[11,4]  = 0x00;
                if (rw.byte_en[1].to_bool())
                    R_reg[23,12] = 0x000;
            }
            else //write
            {
                if (rw.byte_en[0].to_bool())
                    W_reg(11,4) = 0xff;
                if (rw.byte_en[1].to_bool())
                    W_reg(23,12) = 0xfff;
            }
            break;
        } // switch

        // show sequence item changed by DUT
        std::cout << "dut: " << rw.convert2string() << std::endl;
    }
}; // class dut


class tb_env : public uvm::uvm_env
{
    public:

    UVM_COMPONENT_UTILS(tb_env);

    block_B* regmodel;
    reg_agent<dut>* bus;
    uvm::uvm_reg_predictor<reg_rw>* predict;

    tb_env( uvm::uvm_component_name name = "tb_env")
        : uvm::uvm_env(name),
    regmodel(NULL),
    bus(NULL),
    predict(NULL)
    {}

    virtual void build_phase(uvm::uvm_phase& phase)
    {
        uvm::uvm_env::build_phase(phase);

        regmodel = block_B::type_id::create("regmodel");
        regmodel->set_hdl_path_root("top.dut");

        regmodel->has_hdl_path("RTL");
       // regmodel->add_hdl_path("top.dut","RTL");
       
	//regmodel->get_hdl_path("top.dut","RTL");
       
        regmodel->build();
        regmodel->lock_model();

        bus = reg_agent<dut>::type_id::create("bus", this);
        predict = uvm::uvm_reg_predictor<reg_rw>::type_id::create("predict", this);

        set_report_verbosity_level_hier(uvm::UVM_HIGH);
    }

    virtual void connect_phase(uvm::uvm_phase& phase)
    {
        reg2rw_adapter* reg2rw  = new reg2rw_adapter("reg2rw");

        regmodel->default_map->set_sequencer(bus->sqr, reg2rw);

        predict->map = regmodel->default_map;
        predict->adapter = reg2rw;

        bus->mon->ap.connect(predict->bus_in);

        regmodel->default_map->set_auto_predict(false);
    }

}; // class tb_end

#endif // TB_ENV_H_
