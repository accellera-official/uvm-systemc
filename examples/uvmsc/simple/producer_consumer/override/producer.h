//----------------------------------------------------------------------
//   Copyright 2009 Cadence Design Systems, Inc.
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2019 COSEDA Technologies GmbH
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

#ifndef PRODUCER_H_
#define PRODUCER_H_

#include <fstream>
#include <string>

#include <systemc>
#include <uvm>

// define a producer component that produces data at its output port
// by reading the data from a file; the file name is a configurable
// string parameter
template <typename T>
class producer : public uvm::uvm_component
{
 public:
  // output port
  uvm::uvm_blocking_put_port<T> out;

  // configurable string parameter
  std::string in_file;

  // constructor
  producer(uvm::uvm_component_name nm) : uvm::uvm_component(nm), out("out")
  {
    // get parameter value for in_file through the configuration database
    uvm::uvm_config_db<std::string>::get(this, "", "in_file", in_file);

    std::cout << "in producer ctor, in_file is " << in_file << std::endl;
    in_file_strm = new std::ifstream(in_file.c_str());
  }

  // factory registration
  UVM_COMPONENT_PARAM_UTILS(producer<T>);

  // produce tokens in the run task
  void run_phase(uvm::uvm_phase& phase)
  {
    phase.raise_objection(this);

    int val = 0;
    // generate data from file
    for (int i = 0; i < 7; i++)
    {
      sc_core::wait(10, sc_core::SC_NS);

      // read token from file
      *in_file_strm >> val;

      // use token to construct a data packet
      T data(val);

      // give packets a unique string name
      std::stringstream nr;
      nr << i;
      data.set_name("packet-"+ nr.str());

      // output data packet on output port
      std::ostringstream info;
      info << "Producer sending " << val;
      UVM_INFO(get_full_name(), info.str(), uvm::UVM_NONE);
      out.put(data);
    }

    // done producing tokens, wait a little, and then stop the test
    std::ostringstream info;
    info << "*** Done producing tokens, wait for 100 ns now so "
         << "remaining tokens in fifo can be processed... ***";
    UVM_INFO(get_full_name(), info.str(), uvm::UVM_NONE);

    sc_core::wait(100, sc_core::SC_NS);

    phase.drop_objection(this);
  }

  // destructor
  virtual ~producer() 
  {
    delete in_file_strm;
  }

 protected:
  std::ifstream* in_file_strm;
};

#endif /* PRODUCER_H_ */
