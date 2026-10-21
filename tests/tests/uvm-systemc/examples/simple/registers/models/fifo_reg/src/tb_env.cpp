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

#include "../inc/tb_env.h"
#include "../inc/dut_reset_seq.h"

tb_env::tb_env(uvm::uvm_component_name name) :
    uvm::uvm_env(name),
    regmodel(nullptr),
    apb(nullptr),
    predict(nullptr) {}


void tb_env::build_phase(uvm::uvm_phase & phase)
{
    if (regmodel == nullptr) {
        regmodel = reg_block_B::type_id::create("regmodel");
        regmodel->build();
        regmodel->lock_model();
    }

    apb = apb_agent::type_id::create("apb", this);
    uvm::uvm_config_db<int>::set(this, "apb", "is_active", uvm::UVM_ACTIVE);
    predict = uvm::uvm_reg_predictor<uvm::uvm_handle<apb_rw>>::type_id::create("predict", this);

}

void tb_env::connect_phase(uvm::uvm_phase & phase)
{
    if (regmodel->get_parent() == nullptr) {
        reg2apb_adapter* apb_adapter = reg2apb_adapter::type_id::create("apb_adapter");
        regmodel->default_map->set_sequencer(apb->sqr, apb_adapter);
        regmodel->default_map->set_auto_predict(0);

        apb->mon->ap.connect(predict->bus_in);

        predict->map = regmodel->default_map;
        predict->adapter = apb_adapter;
    }
}

void tb_env::run_phase(uvm::uvm_phase & phase)
{
    uvm::uvm_status_e status {};
    uvm::uvm_reg_data_t data;
    std::vector<uvm::uvm_reg_data_t> expected;
    unsigned max{};
    fifo_reg* FIFO{};

    phase.raise_objection(this);

    UVM_INFO("Test", "Resetting DUT and Register Model...", uvm::UVM_LOW);
    auto rst_seq = dut_reset_seq::type_id::create_handle("rst_seq", this);
    rst_seq->dt = dt;
    rst_seq->start(nullptr);
    regmodel->reset();

    FIFO = regmodel->FIFO;
    max = FIFO->capacity();

    FIFO->set_compare(uvm::UVM_CHECK);

    UVM_INFO("FIFO Example", (std::string) "Initializing FIFO reg of max size " + std::to_string(max) + (std::string) " with set()...", uvm::UVM_LOW);

    expected.resize(max);

    // SET - preload regmodel; remodel now has full FIFO; DUT still empty
    for (unsigned i = 0; i < max; ++i) {
        data = dist(rng);
        expected[i] = data;
        FIFO->set(data);
    }

    UVM_INFO("FIFO Example", "Updating DUT FIFO reg with mirror using update()...", uvm::UVM_LOW);

    // UPDATE - write regmodel contents to DUT; DUT now has full FIFO
    FIFO->update(status);
    if (status == uvm::UVM_NOT_OK) {
        UVM_FATAL("FIFO Update Error", "Received status UVM_NOT_OK updating DUT");
    }

    UVM_INFO("FIFO Example", " Read back DUT FIFO reg into mirror using read()...", uvm::UVM_LOW);

    // READ - read contents of DUT back to regmodel; DUT is empty now, regmodel FULL
    for (unsigned i = 0; i < max; ++i) {
        FIFO->read(status, data);
        if (status == uvm::UVM_NOT_OK) {
            UVM_FATAL("FIFO Read Error", "Received status UVM_NOT_OK updating Regmodel");
        }
    }

    phase.drop_objection(this);
}
