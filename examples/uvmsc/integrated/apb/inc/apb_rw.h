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

#ifndef APB_RW_H_
#define APB_RW_H_

#include <systemc>
#include <uvm>

typedef enum {
    READ,
    WRITE
} apb_rw_enum;

class apb_rw : public uvm::uvm_sequence_item {
    using uvm_sequence_item::uvm_report;
    using uvm_sequence_item::uvm_report_info;
    using uvm_sequence_item::uvm_report_warning;
    using uvm_sequence_item::uvm_report_error;
    using uvm_sequence_item::uvm_report_fatal;

    public:
        sc_dt::sc_lv<32> addr;
        sc_dt::sc_lv<32> data;
        apb_rw_enum kind_e;

        UVM_OBJECT_UTILS(apb_rw);

        apb_rw(const std::string & name = "apb_rw") :
            uvm::uvm_sequence_item(name) {}

        virtual void do_print(const uvm::uvm_printer& printer) const;
        virtual void do_pack(uvm::uvm_packer& p) const;
        virtual void do_unpack(uvm::uvm_packer& p);
        virtual void do_copy(const uvm::uvm_object& rhs);
        virtual bool do_compare(const uvm_object& rhs, const uvm::uvm_comparer* comparer = NULL) const;
        std::string convert2string() const;
};

#endif /* APB_RW_ */
