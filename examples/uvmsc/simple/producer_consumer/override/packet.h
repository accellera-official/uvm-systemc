//----------------------------------------------------------------------
//   Copyright 2020 Bosch Sensortec GmbH
//   Copyright 2019 COSEDA Technologies GmbH
//   Copyright 2009 Cadence Design Systems, Inc.
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

#ifndef PACKET_H
#define PACKET_H

#include <systemc>
#include <uvm>

/////////////////

class packet : public uvm::uvm_object
{
 public:

  UVM_OBJECT_UTILS(packet);

  packet( const std::string& name = "packet" ) : uvm::uvm_object(name)
  {
    data = 17;
  }

  packet(int i)
  {
    data = i;
  }

  virtual ~packet() {}

  virtual void do_print(const uvm::uvm_printer& printer) const
  {
    printer.print_field_int("data", data);
  }

  virtual void do_pack(uvm::uvm_packer& p) const
  {
    p << data;
  }

  virtual void do_unpack(uvm::uvm_packer& p)
  {
    p >> data;
  }

  virtual void do_copy(const uvm_object& rhs)
  {
    const packet* drhs = dynamic_cast<const packet*>(&rhs);
    if (!drhs)
    {
      std::cerr << "ERROR in do_copy" << std::endl;
      return;
    }
    data = drhs->data;
  }

  virtual bool do_compare(const uvm_object& rhs, const uvm::uvm_comparer*) const
  {
    const packet* drhs = dynamic_cast<const packet*>(&rhs);
    if (!drhs)
    {
      std::cerr << "ERROR in do_compare" << std::endl;
      return true;
    }
    if (!(data == drhs->data)) return false;
    return true;
  }

 public:
  int data;
};

/////////////////

#endif /* PACKET_H_ */
