// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>

#include "../transactions/jelly_bean_transaction.h"
#include "jelly_bean_scoreboard.h"

using uvm::uvm_subscriber;
using uvm::uvm_component_name;

class jelly_bean_sb_subscriber : public uvm_subscriber<jelly_bean_transaction> {
public:
    UVM_COMPONENT_UTILS(jelly_bean_sb_subscriber);

    jelly_bean_sb_subscriber(uvm_component_name name) : uvm_subscriber<jelly_bean_transaction>(name) {
    };

    virtual ~jelly_bean_sb_subscriber() {
    };

    void write(const jelly_bean_transaction& t);
};
