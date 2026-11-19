//
// -------------------------------------------------------------
//    Copyright 2021 NXP B.V.
//    Copyright 2010 Mentor Graphics Corporation
//    Copyright 2011 Synopsys, Inc.
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

#ifndef REG_MODEL_H_
#define REG_MODEL_H_

#include <systemc>
#include <uvm>

class fifo_reg : public uvm::uvm_reg_fifo
{
    public:
        fifo_reg(const std::string & name = "fifo_reg") :
            uvm::uvm_reg_fifo(name, 8, 32, uvm::UVM_NO_COVERAGE)
            {}

        UVM_OBJECT_UTILS(fifo_reg);
};

class reg_block_B : public uvm::uvm_reg_block
{
    public:
        fifo_reg* FIFO;

        reg_block_B(const std::string & name = "B") :
            uvm::uvm_reg_block(name, uvm::UVM_NO_COVERAGE)
            {}
        ~reg_block_B() override {
            if (FIFO) {
                delete FIFO;
            }
        }

        virtual void build();

        UVM_OBJECT_UTILS(reg_block_B);
};

#endif /* REG_MODEL_H_ */
