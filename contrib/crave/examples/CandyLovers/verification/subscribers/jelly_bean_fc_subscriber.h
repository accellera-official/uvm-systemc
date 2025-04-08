// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>
#include <crave/ConstrainedRandom.hpp>
#include <crave/experimental/Experimental.hpp>

#include "../transactions/jelly_bean_transaction.h"

using crave::crv_variable;
using crave::crv_coverpoint;
using crave::crv_object_name;

//First define a covergroup
class jelly_bean_cg : public crave::crv_covergroup {
public:
    //These variables represent the values
    crv_variable<color_e> cover_color;
    crv_variable<flavor_e> cover_flavor;
    crv_variable<bool> cover_sugar_free;
    crv_variable<bool> cover_sour;

	//These coverpoints actual measure coverage
    crv_coverpoint cp_color{"cp_color"};
    crv_coverpoint cp_flavor{"cp_flavor"};
    crv_coverpoint cp_sugar_free{"cp_sugar_free"};
    crv_coverpoint cp_sour{"cp_sour"};

    jelly_bean_cg(crv_object_name) {
		//Each coverpoint has a bin that is described by a constraint
		//If a measured value matches the constraint, the bin is increased by one
        cp_sugar_free.bins(cover_sugar_free() == true);
        cp_sour.bins(cover_sour() == true);
        cp_sugar_free.bins(cover_sugar_free() == false);
        cp_sour.bins(cover_sour() == false);

		//A bin for each color/flavor
        for (int i = 0; i < 3; i++) {
            cp_color.bins(cover_color() == i);
        }
        for (int i = 1; i < flavor_e::_size(); i++) {
            cp_flavor.bins(cover_flavor() == i);
        }
    }

    jelly_bean_cg(const jelly_bean_cg&) {
    }

    virtual ~jelly_bean_cg() {
    }
};

class jelly_bean_fc_subscriber : public uvm::uvm_subscriber<jelly_bean_transaction> {
public:
    UVM_COMPONENT_UTILS(jelly_bean_fc_subscriber);

    jelly_bean_transaction jb_tx;
    jelly_bean_cg jb_cg = jelly_bean_cg("jb_cg");

    jelly_bean_fc_subscriber(uvm::uvm_component_name name) : uvm::uvm_subscriber<jelly_bean_transaction>(name) {
		//The last action needed is to bind transaction variables to its corresponding coverpoint
        jb_cg.cover_color.bind(jb_tx.color);
        jb_cg.cover_flavor.bind(jb_tx.flavor);
        jb_cg.cover_sugar_free.bind(jb_tx.sugar_free);
        jb_cg.cover_sour.bind(jb_tx.sour);
    };

    virtual ~jelly_bean_fc_subscriber() {
    };

    void write(const jelly_bean_transaction& t);

    void report_phase(uvm::uvm_phase& phase) {
        uvm_subscriber<jelly_bean_transaction>::report_phase(phase);
        jb_cg.report();
    }
};
