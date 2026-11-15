// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>
#include <systemc>
#include <crave2uvm.h>
#include <crave/SystemC.hpp>
#include <crave/ConstrainedRandom.hpp>
#include <crave/experimental/Experimental.hpp>
#include <stdlib.h>
#include <set>
#include <iostream>
#include <string>

using crave::crv_variable;
using crave::crv_constraint;
using crave::if_then;
using crave::crv_object_name;

CRAVE_BETTER_ENUM(color_e, RED = 0, GREEN = 1, BLUE = 2);
CRAVE_BETTER_ENUM(flavor_e, NO_FLAVOR = 0, APPLE = 1, BLUEBERRY = 2, 
							BUBBLE_GUM = 3, CHOCOLATE = 4);
CRAVE_BETTER_ENUM(taste_e, UNKNOWN = 0, YUMMY = 1, YUCKY = 2);

//Include CRAVE into a sequence_item by using uvm_randomized_sequence_item for inheritence
class jelly_bean_transaction : public uvm_randomized_sequence_item {
public:
    UVM_OBJECT_UTILS(jelly_bean_transaction);
	
	//Define some randomziable attributes of a bean
    crv_variable<color_e> color;
    crv_variable<flavor_e> flavor;
    crv_variable<taste_e> taste;
    crv_variable<bool> sugar_free;
    crv_variable<bool> sour;

	//A bean must have a flavor
    crv_constraint c_flavor{flavor() != flavor_e::NO_FLAVOR};
	//A apple flavored bean implies it cannot be blue
    crv_constraint c_color1{if_then(flavor() == flavor_e::APPLE, color() != color_e::BLUE)};
	//A blueberry flavored bean implies it must be blue
    crv_constraint c_color2{if_then(flavor() == flavor_e::BLUEBERRY, color() == color_e::BLUE)};
	//And finally, a green bean must be sour
    crv_constraint c_sour{if_then(color() == color_e::GREEN, sour() == true)};

    jelly_bean_transaction(crv_object_name name = "jelly_beans_trans") : uvm_randomized_sequence_item(name) {
    };

    virtual ~jelly_bean_transaction() {
    };

    virtual void do_print(const uvm::uvm_printer& printer) const {
        printer.print_string("color", color._to_string());
        printer.print_string("flavor", flavor._to_string());
        printer.print_string("taste", taste._to_string());
        printer.print_field_int("sugar_free", (int) sugar_free);
        printer.print_field_int("sour", (int) sour);
    }
};

