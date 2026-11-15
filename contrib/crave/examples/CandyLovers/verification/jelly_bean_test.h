// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>
#include <systemc>

#include "jelly_bean_env.h"
#include "transactions/jelly_bean_transaction.h"
#include "transactions/sugar_free_jelly_bean_transaction.h"
#include "sequences/one_jelly_bean_sequence.h"
#include "sequences/same_flavored_jelly_beans_sequence.h"
#include "sequences/gift_boxed_jelly_beans_sequence.h"
#include "jelly_bean_configuration.h"

using uvm::uvm_test;
using uvm::uvm_component_name;
using uvm::uvm_phase;
using uvm::uvm_config_db;
using std::cout;
using std::endl;

class jelly_bean_test : public uvm_test {
private:
    jelly_bean_env* jb_env;

public:
    UVM_COMPONENT_UTILS(jelly_bean_test);

    jelly_bean_test(uvm_component_name name) : uvm_test(name), jb_env(0) {
    };

    virtual ~jelly_bean_test() {
    };

    void build_phase(uvm_phase& phase) {
        uvm_test::build_phase(phase);
        jelly_bean_configuration jb_cfg("jb_cf");
        //assert(jb_cfg.randomize()); TODO: Randomized config?
        uvm_config_db<jelly_bean_configuration>::set(this, "*", "config", jb_cfg);
        jelly_bean_transaction::type_id::set_type_override(sugar_free_jelly_bean_transaction::get_type());
        jb_env = jelly_bean_env::type_id::create("jb_env", this);
    }

    void run_phase(uvm_phase& phase) {
        UVM_INFO("jelly_bean_test", "Test is running!", UVM_LOW);

        cout << std::endl << "######### One jelly bean ##########" << endl << endl;

		//Start with a "sequence" of one single bean
        one_jelly_bean_sequence<jelly_bean_transaction>* jb_seq;
        phase.raise_objection(this);
        jb_seq = one_jelly_bean_sequence<jelly_bean_transaction>::type_id::create("jb_seq", this);
        jb_seq->start(jb_env->jb_agent->jb_seqr);
        sc_core::wait(10, SC_NS);

        cout << "############## Same flavored jelly beans ###############" << endl << endl;

		//Now test a sequence of two to four beans that will have the same flavor
        same_flavored_jelly_beans_sequence<jelly_bean_transaction>* jb_sf_seq;
        jb_sf_seq = same_flavored_jelly_beans_sequence<jelly_bean_transaction>::type_id::create("jb_sf_seq", this);
        jb_sf_seq->start(jb_env->jb_agent->jb_seqr);
        sc_core::wait(10, SC_NS);

        cout << "############## Gift boxed jelly beans ################" << endl << endl;

		//Finally test a gift box: A sequence of two or three independent same flavored 
		//sequences with two to four beens each
        gift_boxed_jelly_beans_sequence<jelly_bean_transaction>* jb_gb_seq;
        jb_gb_seq = gift_boxed_jelly_beans_sequence<jelly_bean_transaction>::type_id::create("jb_gb_seq", this);
        jb_gb_seq->start(jb_env->jb_agent->jb_seqr);
        sc_core::wait(10, SC_NS);

        phase.drop_objection(this);
    }

    void final_phase(uvm_phase& phase) {
        uvm_test::final_phase(phase);
        //Needed to stop the clock properly
        sc_core::sc_stop();
    }
};
