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

#ifndef SEQUENCE_H_
#define SEQUENCE_H_

#include <systemc>
#include <tlm.h>
#include <uvm>

template <typename REQ = uvm::uvm_sequence_item, typename RSP = REQ>
class sequence : public uvm::uvm_sequence<REQ,RSP>
{
 public:
  sequence( const std::string& name = "seq" ) : uvm::uvm_sequence<REQ,RSP>( name )
  {
    std::cout << sc_core::sc_time_stamp() << ": constructor " << name << std::endl;
  }

  UVM_OBJECT_PARAM_UTILS(sequence<REQ,RSP>);

  void pre_body()
  {
    // raise objection if started as a root sequence
    if(this->get_starting_phase() != NULL)
      this->get_starting_phase()->raise_objection(this);
  }

  void body()
  {
    REQ* req;
    RSP* rsp;

    UVM_INFO(this->get_name(), "Starting sequence", uvm::UVM_MEDIUM);

    for(int i = 1; i < 10; i++)
    {
      req = new REQ();
      rsp = new RSP();

      req->data = i;
      std::cout << sc_core::sc_time_stamp() << ": " << this->get_full_name() << " start_item value " << i << std::endl;

      this->start_item(req);
      this->finish_item(req);
      this->get_response(rsp);
      
      delete req;
      delete rsp;
    }

    UVM_INFO(this->get_name(), "Finishing sequence", uvm::UVM_MEDIUM);
  }

  void post_body()
  {
    // drop objection if started as a root sequence
    if(this->get_starting_phase() != NULL)
      this->get_starting_phase()->drop_objection(this);
  }

};

#endif /* SEQUENCE_H_ */
