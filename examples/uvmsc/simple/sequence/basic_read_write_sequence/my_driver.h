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

#ifndef MY_DRIVER_H_
#define MY_DRIVER_H_

#include <systemc>
#include <tlm.h>
#include <uvm>

#include "bus_trans.h"

template <typename REQ = uvm::uvm_sequence_item, typename RSP = REQ>
class my_driver : public uvm::uvm_driver<REQ,RSP>
{
 public:
  my_driver( uvm::uvm_component_name name ) : uvm::uvm_driver<REQ,RSP>( name ) {}

  UVM_COMPONENT_PARAM_UTILS(my_driver<REQ,RSP>);

  void run_phase(uvm::uvm_phase& phase)
  {
    REQ req;
    RSP rsp;

    for(;;) // forever loop
    {
      this->seq_item_port->get_next_item(req); // or alternative this->seq_item_port->peek(req)
      rsp.set_id_info(req);

      // Actually do the read or write here
      if (req.op == BUS_READ)
      {
        rsp.addr = req.addr;
        rsp.data = data_array[rsp.addr];
        UVM_INFO("my_driver", rsp.convert2string(), uvm::UVM_MEDIUM);
      }
      else // req.op == BUS_WRITE
      {
        data_array[req.addr] = req.data;
        UVM_INFO("my_driver", req.convert2string(), uvm::UVM_MEDIUM);
      }

      //introduce static error for testing purpose
      //data_array[0x13] = 0;

      this->seq_item_port->item_done();       // or alternative this->seq_item_port->get(tmp) to flush item from fifo
      this->seq_item_port->put_response(rsp); // or alternative: this->seq_item_port->put(rsp)
    }
  }

 private:
  unsigned int data_array[512];

};

#endif /* MY_DRIVER_H_ */
