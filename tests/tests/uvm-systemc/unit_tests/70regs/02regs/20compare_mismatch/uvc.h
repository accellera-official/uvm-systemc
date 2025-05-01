//----------------------------------------------------------------------
//   Copyright 2007-2011 Cadence Design Systems, Inc.
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

#ifndef UVC_H_
#define UVC_H_

#include <systemc>
#include <uvm>

#include "transaction.h"

/*
    Code for UVC
*/

// TODO where to put this?
//  uvm_blocking_put_imp_decl(_reg)
  
class uvc_sequencer : public uvm::uvm_sequencer<transaction>
{
 public:
  UVM_COMPONENT_UTILS(uvc_sequencer);

  uvc_sequencer( uvm::uvm_component_name name )
  : uvm::uvm_sequencer<transaction>(name)
  {}
};

template <typename T = int>
class uvc_driver : public uvm::uvm_driver<transaction>
{
public:
  T vif;

  void run_phase( uvm::uvm_phase& phase ) override
  {
    transaction req;
    while(true)
    {
      this->seq_item_port->get_next_item(req);
      vif->write(req);
      sc_core::wait(1.0, sc_core::SC_MS); // #1

      std::ostringstream str;
      str << "Received following transaction :" << req.sprint();
      UVM_INFO("USRDRV", str.str(), uvm::UVM_LOW);

      this->seq_item_port->item_done();
    }
  }

  UVM_COMPONENT_PARAM_UTILS(uvc_driver<T>);

  uvc_driver( uvm::uvm_component_name name)
  : uvm::uvm_driver<transaction>(name)
  {}
};

template <typename T = int>
class uvc_env : public uvm::uvm_env
{
public:

  uvc_sequencer* uos;
  uvc_driver<T>* uod;

  void build_phase( uvm::uvm_phase& phase) override
  {
    uvm_env::build_phase(phase);

    // Create UVC sequencer
    uos = uvc_sequencer::type_id::create("uos");

    // Create UVC driver
    uod = uvc_driver<T>::type_id::create("uod");
  }

  void connect_phase( uvm::uvm_phase& phase) override
  {
    uod->seq_item_port.connect(uos->seq_item_export);
  }

  UVM_COMPONENT_PARAM_UTILS(uvc_env<T>);

  uvc_env( uvm::uvm_component_name name ) : uvm::uvm_env(name)
  {}
};


#endif // UVC_H_
