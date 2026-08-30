// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>
#include <crave2uvm.h>

template<typename REQ = jelly_bean_transaction, typename RSP = REQ>

//Use CRAVE in a sequence, hence inherit from uvm_randomized_sequence
class same_flavored_jelly_beans_sequence : public uvm_randomized_sequence<REQ, RSP> {
public:
	//Again define some randomizable attributes
    crv_variable<int> length;
    crv_variable<flavor_e> flavor;
	
	//The length of the sequence may differ between two and four
    crv_constraint c_length{2 <= length(), length() <= 4};
	//Make sure the flavor that persits in this sequence is not flavorless
    crv_constraint c_no_no_flavor{flavor() != flavor_e::NO_FLAVOR};

    void body() {
		//Randomize the sequence once to get its flavor and length
        this->randomize();
        REQ* req = new REQ();
        RSP* rsp = new RSP();
        for (unsigned int i = 0; i < length; i++) {
			//Create transactions and rerandomize them to have the flavor of the sequence
			//The makro UVM_DO_WITH of UVM_SystemC can be used for this
            UVM_DO_WITH(req, req->flavor() == flavor._to_integral());
            this->get_response(rsp);
        }
    }
	
	same_flavored_jelly_beans_sequence(crave::crv_object_name name) : uvm_randomized_sequence<REQ, RSP>(name) {
    };
    UVM_OBJECT_PARAM_UTILS(same_flavored_jelly_beans_sequence<REQ, RSP>);

    virtual ~same_flavored_jelly_beans_sequence() {
    };
};
