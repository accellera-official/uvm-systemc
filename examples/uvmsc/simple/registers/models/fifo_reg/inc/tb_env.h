// 
// -------------------------------------------------------------
//    Copyright 2021 NXP B.V.
//    Copyright 2010-2011 Mentor Graphics Corporation
//    Copyright 2010-2011 Synopsys, Inc.
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

#include "../../../../../integrated/apb/inc/apb.h"
#include "reg_model.h"
#include "dut_top.h"
#include "dut_reset_seq.h"

#include <algorithm>
#include <random>

#define SEED 12345

class tb_env : public uvm::uvm_env
{
    public:
        UVM_COMPONENT_UTILS(tb_env);

        reg_block_B* regmodel;
        apb_agent* apb;
        uvm::uvm_reg_predictor<apb_rw>* predict;

        dut_top* dt;

        tb_env(uvm::uvm_component_name name);
        ~tb_env() override;

        void build_phase(uvm::uvm_phase & phase) override;
        void connect_phase(uvm::uvm_phase & phase) override;
        void run_phase(uvm::uvm_phase & phase) override;

    private:
        std::mt19937 rng {SEED};
        std::uniform_int_distribution<unsigned> dist {0x0, 0xFFFFFFFF};
};
