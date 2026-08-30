// 
// -------------------------------------------------------------
//    Copyright 2021 NXP B.V.
//    Copyright 2004-2011 Synopsys, Inc.
//    Copyright 2010 Mentor Graphics Corporation
//    Copyright 2010 Cadence Design Systems, Inc.
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

#ifndef REG2APB_ADAPTER_H_
#define REG2APB_ADAPTER_H_

#include <systemc>
#include <uvm>

class reg2apb_adapter : public uvm::uvm_reg_adapter
{
    public:
        UVM_OBJECT_UTILS(reg2apb_adapter);

        reg2apb_adapter(const std::string & name = "reg2apb_adapter") :
            uvm::uvm_reg_adapter(name) {}

        uvm::uvm_sequence_item* reg2bus(const uvm::uvm_reg_bus_op &) override;

        void bus2reg(const uvm::uvm_sequence_item *, uvm::uvm_reg_bus_op &) override;
};

#endif /* REG2APB_ADAPTER_H_ */
