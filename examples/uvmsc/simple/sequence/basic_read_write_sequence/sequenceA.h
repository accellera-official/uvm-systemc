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

#ifndef SEQUENCEA_H_
#define SEQUENCEA_H_

#include <systemc>
#include <tlm.h>
#include <uvm>

#include "constants.h"

#define NUM_LOOPS 10

template <typename REQ = uvm::uvm_sequence_item, typename RSP = REQ>
class sequenceA : public uvm::uvm_sequence<REQ,RSP>
{
 public:
  sequenceA( const std::string& name_ ) : uvm::uvm_sequence<REQ,RSP>( name_ )
  {
    my_id = g_my_id++;
  }

  UVM_OBJECT_PARAM_UTILS(sequenceA<REQ,RSP>);

  void body()
  {
    std::string prstring;
    REQ* req;
    RSP* rsp;
    rsp = new RSP();

    UVM_INFO(this->get_name(), "Starting sequence", uvm::UVM_MEDIUM);

    for(unsigned int i = 0; i < NUM_LOOPS; i++)
    {
      req = new REQ();
      req->addr = (my_id * NUM_LOOPS) + i;
      req->data = my_id + i + 55;
      req->op   = BUS_WRITE;

      this->wait_for_grant();
      this->send_request(req);
      this->get_response(rsp);

      delete req;

      req = new REQ();
      req->addr = (my_id * NUM_LOOPS) + i;
      req->data = 0;
      req->op   = BUS_READ;

      this->wait_for_grant();
      this->send_request(req);
      this->get_response(rsp);

      delete req;

      if (rsp->data != my_id + i + 55 )
      {
        std::ostringstream str;
        str << "Error, address: 0x" << std::hex << req->addr;
        str << " expected data: 0x" << std::hex << my_id + i + 55;
        str << " actual data: 0x" << std::hex << rsp->data << std::endl;
        UVM_ERROR(this->get_name(), str.str());
      }
    }
    delete rsp;

    UVM_INFO(this->get_name(), "Finishing sequence", uvm::UVM_MEDIUM);
  }

 private:
  static unsigned int g_my_id;
  unsigned int my_id;
};

template <typename REQ, typename RSP>
unsigned int sequenceA<REQ,RSP>::g_my_id = 1;


#endif /* SEQUENCEA_H_ */
