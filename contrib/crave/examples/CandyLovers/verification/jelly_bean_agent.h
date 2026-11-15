// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>
#include <tlm.h>

#include "jelly_bean_driver.h"
#include "jelly_bean_sequencer.h"
#include "jelly_bean_monitor.h"
#include "transactions/jelly_bean_transaction.h"

using uvm::uvm_agent;
using uvm::uvm_component_name;
using uvm::uvm_phase;
using uvm::uvm_analysis_port;

class jelly_bean_agent : public uvm_agent {
public:
    uvm_analysis_port<jelly_bean_transaction> jb_ap;
    jelly_bean_sequencer<jelly_bean_transaction>* jb_seqr;
    jelly_bean_driver<jelly_bean_transaction>* jb_drvr;
    jelly_bean_monitor* jb_mon;

    jelly_bean_agent(uvm_component_name name) : uvm_agent(name), jb_seqr(0), jb_drvr(0), jb_mon(0) {
    };

    virtual ~jelly_bean_agent() {
    };

    void build_phase(uvm_phase& phase) {
        uvm_agent::build_phase(phase);
        jb_seqr = jelly_bean_sequencer<jelly_bean_transaction>::type_id::create("jb_seqr", this);
        jb_mon = jelly_bean_monitor::type_id::create("jb_mon", this);
        jb_drvr = jelly_bean_driver<jelly_bean_transaction>::type_id::create("jb_drvr", this);
    }

    void connect_phase(uvm_phase& phase) {
        uvm_agent::connect_phase(phase);
        jb_drvr->seq_item_port.connect(jb_seqr->seq_item_export);
        jb_mon->analyse.connect(jb_ap);
    }

    UVM_COMPONENT_UTILS(jelly_bean_agent);
};
