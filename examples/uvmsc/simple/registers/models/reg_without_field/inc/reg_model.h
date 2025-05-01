//
//------------------------------------------------------------------------------
//   Copyright 2021 NXP B.V.
//   Copyright 2011 Mentor Graphics Corporation
//   Copyright 2011 Cadence Design Systems, Inc. 
//   Copyright 2011 Synopsys, Inc.
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License"); you may not use this file except in
//   compliance with the License.  You may obtain a copy of
//   the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in
//   writing, software distributed under the License is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//   CONDITIONS OF ANY KIND, either express or implied.  See
//   the License for the specific language governing
//   permissions and limitations under the License.
//------------------------------------------------------------------------------
//

#ifndef REG_MODEL_H_
#define REG_MODEL_H_

#include <systemc>
#include <uvm>

class reg_R : public uvm::uvm_reg
{
    public:
        /* rand */ uvm::uvm_reg_data_t value;
    private:
        /* rand */ uvm::uvm_reg_field* _dummy;

    public:
        // constraint _dummy_is_reg {
        //   _dummy.value == value;
        // }

        reg_R(const std::string & name = "R") :
            uvm::uvm_reg(name, 8, uvm::UVM_NO_COVERAGE)
            {}
        ~reg_R() override;

        virtual void build();

        UVM_OBJECT_UTILS(reg_R);
};

class block_B : public uvm::uvm_reg_block
{
    public:
        /* rand */ reg_R* R;
        block_B(const std::string & name = "B") :
            uvm::uvm_reg_block(name, uvm::UVM_NO_COVERAGE)
            {}
        ~block_B() override {
            if (R) {
                delete R;
            }
        }

        virtual void build();

        UVM_OBJECT_UTILS(block_B);
};

#endif /* REG_MODEL_H_ */
