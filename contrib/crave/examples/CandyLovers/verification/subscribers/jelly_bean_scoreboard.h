// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>
#include <sstream>

#include "jelly_bean_sb_subscriber.h"
#include "../transactions/jelly_bean_transaction.h"

using uvm::uvm_scoreboard;
using uvm::uvm_component_name;
using uvm::uvm_phase;

class jelly_bean_sb_subscriber;

class jelly_bean_scoreboard : public uvm_scoreboard {
private:
    jelly_bean_sb_subscriber* jb_sb_sub;
public:
    uvm::uvm_analysis_export<jelly_bean_transaction> jb_analysis_export;

    UVM_COMPONENT_UTILS(jelly_bean_scoreboard)
    jelly_bean_scoreboard(uvm_component_name name) : uvm_scoreboard(name), jb_sb_sub(0) {
    };

    virtual ~jelly_bean_scoreboard() {
    };

    void build_phase(uvm_phase& phase);

    void connect_phase(uvm_phase& phase);

    void check_jelly_bean_taste(jelly_bean_transaction jb_tx);

};
