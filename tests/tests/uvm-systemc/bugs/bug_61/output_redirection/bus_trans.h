//----------------------------------------------------------------------
//   Copyright 2012-2020 NXP B.V.
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
//----------------------------------------------------------------------

#ifndef BUS_TRANS_H_
#define BUS_TRANS_H_

#include <systemc>
#include <uvm>

#include "constants.h"

#include <iomanip>
#include <sstream>
#include <string>

//--------------------------------------------------------------------
// bus_trans
//--------------------------------------------------------------------

class bus_trans : public uvm::uvm_sequence_item
{
 public:

  bus_trans( const std::string& name = "bus_trans_seq_item" ) : uvm::uvm_sequence_item(name)
  {
    addr = 0;
    data = 0;
    op = BUS_READ;
  }

  ~bus_trans() override {}

  UVM_OBJECT_UTILS(bus_trans);

  void do_copy( const uvm::uvm_object& rhs ) override
  {
    const bus_trans* rhs_ = dynamic_cast<const bus_trans*>(&rhs);
    if(rhs_ == nullptr)
      UVM_ERROR("do_copy", "cast failed, check type compatability");

    uvm_sequence_item::do_copy(rhs);

    addr = rhs_->addr;
    data = rhs_->data;
    op = rhs_->op;
  }

   bool do_compare( const uvm::uvm_object& rhs, const uvm::uvm_comparer* comparer ) const override
  {
    const bus_trans* rhs_ = dynamic_cast<const bus_trans*>(&rhs);
    if(rhs_ == nullptr)
      UVM_FATAL("do_compare", "cast failed, check type compatibility");

    return ((op == rhs_->op) && (addr == rhs_->addr) && (data == rhs_->data));
  }

  void do_print( const uvm::uvm_printer& printer ) const override
  {
    printer.print_string("op", (op ? "BUS_WRITE":"BUS_READ"));
    printer.print_field_int("addr", addr);
    printer.print_field_int("data", data);
  }

  std::string convert2string() const override 
  {
    std::ostringstream str;
    str << "op " << (op ? "BUS_WRITE":"BUS_READ");
    str << " addr: 0x" << std::hex << std::setw(3) << std::setfill('0') << addr;
    str << " data: 0x" << std::hex << std::setw(3) << std::setfill('0') << data;
    return str.str();
  }

  // data members
 public:
  int addr;
  int data;
  bus_op_t op;
};

//--------------------------------------------------------------------
// bus_req
//--------------------------------------------------------------------

class bus_req : public bus_trans
{
 public:
  bus_req( const std::string& name = "bus_req_seq_item" ) : bus_trans(name) {}
  ~bus_req() override {}

  UVM_OBJECT_UTILS(bus_req);

};

//--------------------------------------------------------------------
// bus_rsp
//--------------------------------------------------------------------

class bus_rsp : public bus_trans
{
 public:
  bus_rsp( const std::string& name = "bus_rsp_seq_item" ) : bus_trans(name) {
    status = STATUS_NOT_OK;
  }

  ~bus_rsp() override {}

  UVM_OBJECT_UTILS(bus_rsp);

  void do_copy( const uvm::uvm_object& rhs ) override
  {
    const bus_rsp* rhs_ = dynamic_cast<const bus_rsp*>(&rhs);
    if(rhs_ == nullptr)
      UVM_FATAL("do_copy", "cast failed, check type compatibility");

    bus_trans::do_copy(rhs);
    status = rhs_->status;
  }

  std::string convert2string() const override
  {
    std::string statusstr;

    if (status == STATUS_OK)
      statusstr = "STATUS_OK";
    else
      statusstr = "STATUS_NOT_OK";

    std::ostringstream str;
    str << bus_trans::convert2string()
        << " status: "
        << statusstr;
    return str.str();
  }

 private:
  status_t status;
};

#endif /* BUS_TRANS_H_ */
