// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>

template<typename REQ = jelly_bean_transaction, typename RSP = REQ>
class one_jelly_bean_sequence : public uvm::uvm_sequence<REQ, RSP> {
public:

    one_jelly_bean_sequence(const std::string& name_) : uvm_sequence<REQ, RSP>(name_) {
    };
    UVM_OBJECT_PARAM_UTILS(one_jelly_bean_sequence<REQ, RSP>);

    void body() {
        REQ* req = new REQ();
        RSP* rsp = new RSP();
        UVM_DO(req);
        this->get_response(rsp);
    }

    virtual ~one_jelly_bean_sequence() {
    };
};
