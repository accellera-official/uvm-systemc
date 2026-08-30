//----------------------------------------------------------------------
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

#ifndef TOP_H_
#define TOP_H_

#include <systemc>
#include <uvm>
#include <tlm>

#include "config.h"
#include "packet.h"
#include "producer.h"
#include "consumer.h"
#include "packet.h"

class top : public uvm::uvm_env
{
  do_config hw;
  producer<packet> p1;
  producer<packet> p2;
  tlm::tlm_fifo<packet> fifo;
  consumer<packet> c;

 public:
  UVM_COMPONENT_UTILS(top);
    
  top(uvm::uvm_component_name name) : uvm::uvm_env(name), p1("p1"), p2("p2"), fifo("fifo"), c("c")
  {
    c.in(fifo);
    p1.out(fifo);
    p2.out(fifo);
  };

};

#endif /* TOP_H */
