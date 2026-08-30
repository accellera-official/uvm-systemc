// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once
#include <systemc.h>
#include <uvm>

using namespace uvm;

//The taster gets a bean by its color and flavor. Also if its sour or even sugar free.
//As a result the taster decides if the bean is tasty or not.
SC_MODULE(jelly_bean_taster) {
    sc_in<sc_uint<2> > color;
    sc_in<sc_uint<3> > flavor;
    sc_in<bool > sugar_free;
    sc_in<bool > sour;
    sc_in<bool > clk;
    sc_out<sc_uint<2> > taste;

    void taste_bean() {
        if (flavor.read() == 0x4 && sour.read()) { //Chocolate and sour
            taste.write(0x2); //Yucky
        } else {
            taste.write(0x1); //Yummy
        }
    }

    SC_CTOR(jelly_bean_taster) : color("color"), flavor("flavor"), sugar_free("sugar_free"), 
								 sour("sour"), clk("clk"), taste("taste") {
        SC_METHOD(taste_bean);
        sensitive << clk;
    }
};
