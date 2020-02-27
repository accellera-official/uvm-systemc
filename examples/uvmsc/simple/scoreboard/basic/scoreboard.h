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

#ifndef SCOREBOARD_H_
#define SCOREBOARD_H_

#include <systemc>
#include <uvm>

#include "xmt_subscriber.h"
#include "rcv_subscriber.h"
#include "vip_packet.h"

class scoreboard : public uvm::uvm_scoreboard
{
 public:
  uvm::uvm_analysis_export<vip_packet> xmt_listener_imp;
  uvm::uvm_analysis_export<vip_packet> rcv_listener_imp;

  xmt_subscriber* xmt_listener;
  rcv_subscriber* rcv_listener;

  scoreboard( uvm::uvm_component_name name ) : uvm::uvm_scoreboard( name )
  {
    std::cout << sc_core::sc_time_stamp() << ": constructor " << name << std::endl;
  }

  UVM_COMPONENT_UTILS(scoreboard);

  void write_xmt(const vip_packet& p)
  {
    std::cout << sc_core::sc_time_stamp() << ": " << name() << " xmt_listener in scoreboard received value " << p.data << std::endl;
  }

  void write_rcv(const vip_packet& p)
  {
    std::cout << sc_core::sc_time_stamp() << ": " << name() << " rcv_listener in scoreboard received value " << p.data << std::endl;
  }

  void build_phase(uvm::uvm_phase& phase)
  {
    uvm::uvm_scoreboard::build_phase(phase);
    std::cout << sc_core::sc_time_stamp() << ": build_phase " << name() << std::endl;

    uvm::uvm_config_db<uvm_object*>::set(this, "xmt_listener", "sb", this);
    uvm::uvm_config_db<uvm_object*>::set(this, "rcv_listener", "sb", this);

    xmt_listener = xmt_subscriber::type_id::create("xmt_listener", this);
    assert(xmt_listener);

    rcv_listener = rcv_subscriber::type_id::create("rcv_listener", this);
    assert(rcv_listener);
  }

  void connect_phase(uvm::uvm_phase& phase)
  {
    xmt_listener_imp.connect(rcv_listener->analysis_export);
    rcv_listener_imp.connect(rcv_listener->analysis_export);
  }

};

#endif /* SCOREBOARD_H_ */
