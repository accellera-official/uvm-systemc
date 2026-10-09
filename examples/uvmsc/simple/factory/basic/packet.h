//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2009 Cadence Design Systems, Inc.
//   Copyright 2018 Intel Corp.
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

#ifndef PACKET_H_
#define PACKET_H_

#include <uvm>

class packet : public uvm::uvm_object
{
 public:

  int addr;
  int data;

  UVM_OBJECT_UTILS(packet);

  packet( const std::string& name = "packet" )
    : uvm::uvm_object(name), addr(0x1234), data(0x5678)
  {}

  void do_print(const uvm::uvm_printer& printer) const override
  {
    printer.print_field_int("addr", addr);
    printer.print_field_int("data", data);
  }

  void do_copy(const uvm::uvm_object& rhs) override
  {
    const packet* drhs = dynamic_cast<const packet*>(&rhs);
    addr = drhs->addr;
    data = drhs->data;
  }

  bool do_compare(const uvm::uvm_object& rhs, const uvm::uvm_comparer* comparer = nullptr) const override
  {
    const packet* drhs = dynamic_cast<const packet*>(&rhs);

    if (!(addr == drhs->addr))
      return false;

    if (!(data == drhs->data))
      return false;

    return true;
  }
  
  void do_pack(uvm::uvm_packer& packet) const override
  {
    packet << addr << data;
  }

  void do_unpack(uvm::uvm_packer& packet) override
  {
    packet >> addr >> data;
  }

};

#endif /* PACKET_H_ */
