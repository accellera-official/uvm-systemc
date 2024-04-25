// 
// -------------------------------------------------------------
//    Copyright 2021 NXP B.V.
//    Copyright 2004-2011 Synopsys, Inc.
//    Copyright 2010 Mentor Graphics Corporation
//    Copyright 2010-2011 Cadence Design Systems, Inc.
//    All Rights Reserved Worldwide
// 
//    Licensed under the Apache License, Version 2.0 (the
//    "License"); you may not use this file except in
//    compliance with the License.  You may obtain a copy of
//    the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in
//    writing, software distributed under the License is
//    distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//    CONDITIONS OF ANY KIND, either express or implied.  See
//    the License for the specific language governing
//    permissions and limitations under the License.
// -------------------------------------------------------------
// 

#ifndef DUT_TOP_H_
#define DUT_TOP_H_

#include <systemc>

#include "../../../../../integrated/apb/inc/apb_if.h"
#include "dut.h"

class dut_top : public sc_core::sc_module
{
    public:
        dut_top(const sc_core::sc_module_name & name);
        ~dut_top();

#if IEEE_1666_SYSTEMC >= 202301L
#else
        SC_HAS_PROCESS(dut_top);
#endif

        apb_if* apb0;
        dut<>* dut0;

        sc_core::sc_signal<sc_dt::sc_logic> rst;

    private:
        void clk_thread();
};

#endif
