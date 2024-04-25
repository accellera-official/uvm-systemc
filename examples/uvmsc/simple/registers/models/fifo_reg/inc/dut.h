// 
// -------------------------------------------------------------
//    Copyright 2021 NXP B.V.
//    Copyright 2004-2011 Synopsys, Inc.
//    Copyright 2010 Mentor Graphics Corporation
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

#include <systemc>

template<unsigned BASE_ADDR=0>
class dut : public sc_core::sc_module
{
    public:
        sc_core::sc_in<sc_dt::sc_logic>   pclk;
        sc_core::sc_in<sc_dt::sc_lv<32>>  paddr;
        sc_core::sc_in<sc_dt::sc_logic>   psel;
        sc_core::sc_in<sc_dt::sc_logic>   penable;
        sc_core::sc_in<sc_dt::sc_logic>   pwrite;
        sc_core::sc_out<sc_dt::sc_lv<32>> prdata;
        sc_core::sc_in<sc_dt::sc_lv<32>>  pwdata;
        sc_core::sc_in<sc_dt::sc_logic>   rst;

        dut(const sc_core::sc_module_name & name) :
            pclk("pclk"),
            paddr("paddr"),
            psel("psel"),
            penable("penable"),
            pwrite("pwrite"),
            prdata("prdata"),
            pwdata("pwdata"),
            rst("rst"),
            in_range(0)
    {
        SC_METHOD(in_range_method);
        sensitive << paddr;
        dont_initialize();
        SC_METHOD(pr_addr_method);
        sensitive << paddr;
        dont_initialize();
        SC_METHOD(prdata_method);
        sensitive << psel << penable << pwrite;
        SC_METHOD(main_method);
        sensitive << pclk.pos();
    }

#if IEEE_1666_SYSTEMC >= 202301L
#else
        SC_HAS_PROCESS(dut);
#endif

    private:
        void in_range_method() { in_range = (paddr.read().to_uint() - BASE_ADDR == 0); };
        void pr_addr_method() { pr_addr = paddr.read().to_uint() - BASE_ADDR; };
        void prdata_method() { prdata = (psel == sc_dt::SC_LOGIC_1 && penable == sc_dt::SC_LOGIC_1 && pwrite == sc_dt::SC_LOGIC_0 && in_range) ? pr_data : sc_dt::sc_lv<32>(sc_dt::SC_LOGIC_Z); };
        void main_method()
        {
            if (rst == sc_dt::SC_LOGIC_1) {
                for (unsigned i = 0; i < 8; ++i) {
                    fifo[i] = 0;
                }
                w_idx = 0;
                r_idx = 0;
                used = 0;
            }
            else {
                if (psel == sc_dt::SC_LOGIC_1 && penable == pwrite && pr_addr == 0) {
                    pr_data = 0;
                    if (pwrite == sc_dt::SC_LOGIC_1) {
                        if (used != 8) {
                            fifo[w_idx] = pwdata.read().to_uint();
                            w_idx++;
                            used++;
                        }
                    }
                    else {
                        if (used != 0) {
                            pr_data = fifo[r_idx];
                            fifo[r_idx] = 0; // just for debug; not necessary
                            r_idx++;
                            used--;
                        }
                    }
                }
            }
        }

        unsigned fifo[8];
        unsigned w_idx;
        unsigned r_idx;
        unsigned used;
        unsigned pr_data;
        unsigned pr_addr;

        bool in_range;
};
