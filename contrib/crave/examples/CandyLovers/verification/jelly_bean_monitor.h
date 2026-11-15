// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>

#include "transactions/jelly_bean_transaction.h"
#include "../dut/jelly_bean_if.h"

using uvm::uvm_monitor;
using uvm::uvm_phase;
using uvm::uvm_component_name;
using uvm::uvm_config_db;
using uvm::uvm_analysis_port;

class jelly_bean_monitor : public uvm_monitor {
private:
    jelly_bean_if* interface;

public:
    uvm_analysis_port<jelly_bean_transaction> analyse;

    jelly_bean_monitor(uvm_component_name name) : uvm_monitor(name), analyse("jba_port"), interface(0) {
    };
    UVM_COMPONENT_UTILS(jelly_bean_monitor);

    virtual ~jelly_bean_monitor() {
    };

    void build_phase(uvm_phase& phase) {
        uvm_monitor::build_phase(phase);

        if (!uvm_config_db<jelly_bean_if*>::get(this, "", "interface", interface))
            UVM_FATAL(this->get_name(), "Virtual interface not defined! Simulation aborted!");
    }

    void run_phase(uvm::uvm_phase& phase) {
        uvm_monitor::run_phase(phase);
        while (true) {
            jelly_bean_transaction trans;
            sc_core::wait(interface->flavor.default_event());
            if (interface->flavor.read() != flavor_e::NO_FLAVOR) {
                // TODO possible to eliminate _unchecked in the following?
                trans = jelly_bean_transaction();
                trans.color = color_e::_from_integral_unchecked(interface->color.read().to_int());
                trans.sour = interface->sour.read();
                trans.sugar_free = interface->sugar_free.read();
                trans.flavor = flavor_e::_from_integral_unchecked(interface->flavor.read().to_int());
                sc_core::wait(interface->clk.posedge_event());
                trans.taste = taste_e::_from_integral_unchecked(interface->taste.read().to_int());
                analyse.write(trans);
            }
        }
    }
};
