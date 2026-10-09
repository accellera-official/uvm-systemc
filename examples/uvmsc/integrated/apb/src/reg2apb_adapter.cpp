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

#include <systemc>
#include <uvm>

#include "../inc/reg2apb_adapter.h"
#include "../inc/apb_rw.h"

uvm::uvm_handle<uvm::uvm_sequence_item> reg2apb_adapter::reg2bus(const uvm::uvm_reg_bus_op & rw)
{
    auto apb = apb_rw::type_id::create_handle("apb_rw");
    apb->kind_e = (rw.kind == uvm::UVM_READ) ? READ : WRITE;
    apb->addr = rw.addr;
    apb->data = rw.data;

    return apb;
}

void reg2apb_adapter::bus2reg(const uvm::uvm_sequence_item * bus_item,uvm::uvm_reg_bus_op & rw)
{
    const apb_rw* apb = dynamic_cast<const apb_rw*>(bus_item);
    if (!apb) {
        UVM_FATAL("NOT_APB_TYPE","Provided bus_item is not of the correct type");
    }

    rw.kind = apb->kind_e == READ ? uvm::UVM_READ : uvm::UVM_WRITE;
    rw.addr = apb->addr;
    rw.data = apb->data;
    rw.status = uvm::UVM_IS_OK;
}
