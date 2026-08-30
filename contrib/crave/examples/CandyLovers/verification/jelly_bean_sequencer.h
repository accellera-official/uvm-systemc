// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <systemc>
#include <uvm>

using uvm::uvm_sequencer;
using uvm::uvm_component_name;

template <class REQ = jelly_bean_transaction, typename RSP = REQ>
class jelly_bean_sequencer : public uvm_sequencer<REQ, RSP> {
public:

    jelly_bean_sequencer(uvm_component_name name) : uvm_sequencer<REQ, RSP>(name) {
    };
    UVM_COMPONENT_PARAM_UTILS(jelly_bean_sequencer<REQ, RSP>);

    virtual ~jelly_bean_sequencer() {
    };
};
