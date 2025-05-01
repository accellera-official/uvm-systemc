//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
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

#ifndef VIP_PACKET_H
#define VIP_PACKET_H

#include <systemc>
#include <tlm.h>
#include <uvm>

/////////////////

class vip_packet : public uvm::uvm_sequence_item
{
 public:
  UVM_OBJECT_UTILS(vip_packet);

  vip_packet(const std::string& name = "packet") { data = 17; }
  vip_packet(int i) { data = i; }
  ~vip_packet() override { }

   void do_print(const uvm::uvm_printer& printer) const override
  {
    printer.print_field_int("data", data);
  }

  void do_pack(uvm::uvm_packer& p) const override
  {
    p << data;
  }

  void do_unpack(uvm::uvm_packer& p) override
  {
    p >> data;
  }

  void do_copy(const uvm::uvm_object& rhs) override
  {
    const vip_packet* drhs = dynamic_cast<const vip_packet*>(&rhs);
    if (!drhs) { std::cerr << "ERROR in do_copy" << std::endl; return; }
    data = drhs->data;
  }

   bool do_compare(const uvm_object& rhs, const uvm::uvm_comparer*) const override
  {
    const vip_packet* drhs = dynamic_cast<const vip_packet*>(&rhs);
    if (!drhs) { std::cerr << "ERROR in do_compare" << std::endl; return true; }
    if (!(data == drhs->data)) return false;
    return true;
  }

  std::string convert2string() const override
  {
    std::ostringstream str;
    str << " data: " << data;
    return str.str();
  }

 public:
  int data;
};

#endif /* VIP_PACKET_H_ */
