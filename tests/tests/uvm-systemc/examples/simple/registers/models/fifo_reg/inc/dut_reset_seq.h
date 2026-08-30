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

#ifndef DUT_RESET_SEQ_H_
#define DUT_RESET_SEQ_H_

#include <systemc>
#include <uvm>

#include "dut_top.h"

class dut_reset_seq : public uvm::uvm_sequence<> {
    public:
        UVM_OBJECT_UTILS(dut_reset_seq);

        dut_reset_seq(const std::string & name) :
            uvm::uvm_sequence<>(name),
            dt(nullptr) {}

        void body() override;

        dut_top* dt;
};

#endif
