// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once
#include <systemc.h>
#include <uvm>

using namespace uvm;

class jelly_bean_if {
public:
    sc_core::sc_signal<sc_uint<2> > color;
    sc_core::sc_signal<sc_uint<3> > flavor;
    sc_core::sc_signal<bool > sugar_free;
    sc_core::sc_signal<bool > sour;
    sc_core::sc_clock clk;
    sc_core::sc_signal<sc_uint<2> > taste;
};
