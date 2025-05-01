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

#ifndef RCV_SUBSCRIBER_H_
#define RCV_SUBSCRIBER_H_

#include <systemc>
#include <tlm.h>
#include <uvm>

#include "vip_packet.h"

class rcv_subscriber : public uvm::uvm_subscriber<vip_packet>
{
 public:
  rcv_subscriber( uvm::uvm_component_name name );

  UVM_COMPONENT_UTILS(rcv_subscriber);

  void write(const vip_packet& p) override;
};

#endif /* RCV_SUBSCRIBER_H_ */
