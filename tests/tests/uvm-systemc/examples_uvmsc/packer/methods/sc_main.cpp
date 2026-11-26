//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2009 Cadence Design Systems, Inc.
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

//----------------------------------------------------------------------
// This is a simple test that will cover the creation of two uvm_events
// using the uvm_event_pool. Then use the uvm_event_pool methods to print
// those objects.
//----------------------------------------------------------------------

#include <systemc>
#include <uvm>

#include <limits>

#include "packet.h"

int sc_main(int, char*[])
{
  uvm::uvm_root* r = uvm::uvm_root::get();
  r->set_report_verbosity_level(uvm::UVM_DEBUG);

  uvm::uvm_coreservice_t::get()->get_default_packer()->use_metadata = true;
  std::vector<unsigned int> pkdbytes;

  packet packet1("packet1", 0x12, "bla", 0x100, 1.07967, sc_core::sc_time(1.1, sc_core::SC_MS), 1, 2);
  packet packet2("packet2"); // empty packet

  std::cout << "content before packing:" << std::endl;
  packet1.print();
  packet2.print();

  std::cout << "start packing..." << std::endl;
  packet1.pack_ints(pkdbytes);

  std::cout << "Size of packet int vectors: " << pkdbytes.size() << std::endl;

  std::cout << "start unpacking..." << std::endl;
  packet2.unpack_ints(pkdbytes);

  std::cout << "content after unpacking:" << std::endl;
  packet1.print();
  packet2.print();

  if(packet2.compare(packet1))
    std::cout << "packet1 and packet2 are the same." << std::endl;
  else
    std::cout << "packet1 and packet2 are not the same." << std::endl;

  return 0;
}
