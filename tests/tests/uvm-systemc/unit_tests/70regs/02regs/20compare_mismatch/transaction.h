//----------------------------------------------------------------------
//   Copyright 2007-2011 Cadence Design Systems, Inc.
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

#ifndef TRANSACTION_H_
#define TRANSACTION_H_

#include <systemc>
#include <uvm>

class transaction : public uvm::uvm_sequence_item
{
public:
  /* rand */ sc_dt::sc_bv<32> addr;
  /* rand */ sc_dt::sc_bv<32> data;
  /* rand */ uvm::uvm_access_e dir {uvm::UVM_READ};

  UVM_OBJECT_UTILS(transaction);

  void do_copy( const uvm::uvm_object& rhs ) override
  {
    const transaction* rhs_ = dynamic_cast<const transaction*>(&rhs);
    if(rhs_ == nullptr)
      UVM_ERROR("do_copy", "dynamic_cast failed, check type compatability");

    uvm::uvm_sequence_item::do_copy(rhs);

    addr = rhs_->addr;
    data = rhs_->data;
    dir = rhs_->dir;
  }

  bool do_compare( const uvm::uvm_object& rhs, const uvm::uvm_comparer* comparer ) const override
  {
    const transaction* rhs_ = dynamic_cast<const transaction*>(&rhs);
    if(rhs_ == nullptr)
      UVM_FATAL("do_compare", "dynamic_cast failed, check type compatibility");

    return ( (addr == rhs_->addr)
          && (data == rhs_->data)
          && (dir == rhs_->dir)
          );
  }

  bool operator==(const transaction& rhs) {
    return ( (addr == rhs.addr)
          && (data == rhs.data)
          && (dir == rhs.dir)
          );
  }

  void do_print(const uvm::uvm_printer& printer ) const override
  {
    printer.print_field_int("addr", addr);
    printer.print_field_int("data", data);
    printer.print_string("dir", uvm::uvm_access_name[dir]);
  }

  std::string convert2string() const override
  {
    std::ostringstream str;
    str << "dir: " << uvm::uvm_access_name[dir];
    str << " addr: 0x" << std::hex << addr.to_uint64();
    str << " data: 0x" << std::hex << data.to_uint64();
    return str.str();
  }

  transaction( const std::string& name = "unnamed-transaction") : uvm_sequence_item(name)
  {}
};

inline void sc_trace(sc_core::sc_trace_file*, const transaction&, const std::string&) {
  UVM_FATAL("sc_trace","not implemented for transaction objects");
}


#endif // TRANSACTION_H_
