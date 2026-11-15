// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>
#include <systemc.h>
#include <string>
#include <set>
#include <iostream>

#include "transactions/jelly_bean_transaction.h"
#include "../dut/jelly_bean_taster.h"
#include "../dut/jelly_bean_if.h"

using uvm::uvm_driver;
using uvm::uvm_component_name;
using uvm::uvm_phase;

template <typename REQ = jelly_bean_transaction, typename RSP = REQ>
class jelly_bean_driver : public uvm_driver<REQ, RSP> {
private:
    jelly_bean_if* interface;

public:

    jelly_bean_driver(uvm_component_name name) : uvm_driver<REQ, RSP>(name), interface(0) {
    };
    UVM_COMPONENT_PARAM_UTILS(jelly_bean_driver<REQ, RSP>);

    virtual ~jelly_bean_driver() {
    };

    void build_phase(uvm_phase& phase) {
        uvm_driver<REQ, RSP>::build_phase(phase);
        if (!uvm_config_db<jelly_bean_if*>::get(this, "", "interface", interface))
            UVM_FATAL(this->name(), "Virtual interface not defined! Simulation aborted!");
    }

    void run_phase(uvm_phase& phase) {
        uvm_driver<REQ, RSP>::run_phase(phase);
        REQ req, rsp;
        while (true) {
            sc_core::wait(interface->clk.posedge_event());
            interface->flavor.write(0x0);
            this->seq_item_port->get_next_item(req);
            sc_core::wait(interface->clk.posedge_event());
            interface->flavor.write(req.flavor._to_integral());
            interface->color.write(req.color._to_integral());
            interface->sour.write(req.sour);
            interface->sugar_free.write(req.sugar_free);
            rsp.set_id_info(req);
            this->seq_item_port->item_done();
            this->seq_item_port->put(rsp);
        }
    }
};
