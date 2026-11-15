// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>
#include <crave2uvm.h>

#include "same_flavored_jelly_beans_sequence.h"

template<typename REQ = jelly_bean_transaction, typename RSP = REQ>
class gift_boxed_jelly_beans_sequence : public uvm_randomized_sequence<REQ, RSP> {
public:
    crv_variable<int> num_jelly_bean_flavors;
    crv_constraint c_num_flavors{2 <= num_jelly_bean_flavors(), num_jelly_bean_flavors() <= 3};

    gift_boxed_jelly_beans_sequence(crave::crv_object_name name) : uvm_randomized_sequence<REQ, RSP>(name) {
    };
    UVM_OBJECT_PARAM_UTILS(gift_boxed_jelly_beans_sequence<REQ, RSP>);

    void body() {
        this->randomize();
        same_flavored_jelly_beans_sequence<jelly_bean_transaction>* jb_seq;
        string output = "jelly_bean_seq length: " + std::to_string(num_jelly_bean_flavors);
        UVM_INFO("gift_boxed_jelly_beans_sequence", output, UVM_LOW);

        REQ* req = new REQ();
        RSP* rsp = new RSP();

        for (unsigned int i = 0; i < num_jelly_bean_flavors; i++) {
            jb_seq = same_flavored_jelly_beans_sequence<jelly_bean_transaction>::type_id::create("jb_sf_seq_gb", 0, this->get_full_name());
            assert(jb_seq->randomize());
            UVM_DO(jb_seq);
        }
    }

    virtual ~gift_boxed_jelly_beans_sequence() {
    };
};
