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

#include <systemc.h>
#include <tlm.h>
#include <uvm>

template <typename REQ = uvm::uvm_sequence_item, typename RSP = REQ>
class sequenceA : public uvm::uvm_sequence<REQ,RSP>
{
 public:
  sequenceA( const std::string& name_ = "sequenceA") : uvm::uvm_sequence<REQ,RSP>( name_ )
  {
  }

  UVM_OBJECT_PARAM_UTILS(sequenceA<REQ,RSP>);

  void body() override
  {
    std::string prstring;
    REQ* req;
    RSP* rsp;   

    UVM_INFO(this->get_name(), "Starting sequence", uvm::UVM_MEDIUM);

    for(unsigned int i = 0; i < num_loops; i++)
    {
      req = REQ::type_id::create("req");
      rsp = RSP::type_id::create("rsp");

      this->start_item(req);
      req->data[0] = i;
      this->finish_item(req);
      this->get_response(rsp); // optional here
      //UVM_INFO(this->get_name(), "Received Response sequence", uvm::UVM_MEDIUM);
      //rsp->print();
      // Cleanup created sequences
      REQ::type_id::destroy(req);
      RSP::type_id::destroy(rsp);
    }

    UVM_INFO(this->get_name(), "Finishing sequence", uvm::UVM_MEDIUM);
  }

	void set_num_loops(unsigned int num_loops)
	{
		this->num_loops = num_loops;
	}

 private:
  unsigned int num_loops=1;
};


#endif /* SEQUENCEA_H_ */
