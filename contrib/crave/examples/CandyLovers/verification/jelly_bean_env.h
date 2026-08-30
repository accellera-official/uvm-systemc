// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>

#include "jelly_bean_agent.h"
#include "subscribers/jelly_bean_scoreboard.h"
#include "subscribers/jelly_bean_fc_subscriber.h"

using uvm::uvm_env;
using uvm::uvm_component_name;
using uvm::uvm_phase;

class jelly_bean_env : public uvm_env {
public:
    jelly_bean_agent* jb_agent;
    jelly_bean_fc_subscriber* jb_fc_sub;
    jelly_bean_scoreboard* jb_sb;
    UVM_COMPONENT_UTILS(jelly_bean_env);

    jelly_bean_env(uvm_component_name name) : uvm_env(name), jb_agent(0), jb_sb(0), jb_fc_sub(0) {
    };

    virtual ~jelly_bean_env() {
    };

    void build_phase(uvm_phase& phase) {
        uvm_env::build_phase(phase);
        jb_agent = jelly_bean_agent::type_id::create("jb_agent", this);
        jb_fc_sub = jelly_bean_fc_subscriber::type_id::create("jb_fc_sub", this);
        jb_sb = jelly_bean_scoreboard::type_id::create("jb_sb", this);
    }

    void connect_phase(uvm_phase& phase) {
        uvm_env::connect_phase(phase);
        jb_agent->jb_ap.connect(jb_fc_sub->analysis_export);
        jb_agent->jb_ap.connect(jb_sb->jb_analysis_export);
    }
};
