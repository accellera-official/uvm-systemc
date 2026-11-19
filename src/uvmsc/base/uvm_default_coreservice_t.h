//----------------------------------------------------------------------
//   Copyright 2013 Cadence Design Inc
//   Copyright 2016 NXP B.V.
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

#ifndef UVM_DEFAULT_CORESERVICE_T_
#define UVM_DEFAULT_CORESERVICE_T_

#include <systemc>

#include "uvmsc/base/uvm_coreservice_t.h"

namespace uvm {

// forward declaration of necessary classes.
class uvm_factory;
class uvm_report_server;
class uvm_root;
//class uvm_visitor;
class uvm_component;
class uvm_tr_database;

//----------------------------------------------------------------------
// Class: uvm_default_coreservice_t
//
// uvm_default_coreservice_t provides a default implementation of the 
// uvm_coreservice_t API. It instantiates uvm_default_factory, uvm_default_report_server, 
// uvm_root.
//----------------------------------------------------------------------

class uvm_default_coreservice_t : public uvm_coreservice_t
{
 public:
  friend class uvm_coreservice_t;

  uvm_factory* get_factory() const override;
  void set_factory( uvm_factory* f ) override;

//  virtual uvm_tr_database* get_default_tr_database() const;
//  virtual void set_default_tr_database( uvm_tr_database* db );

  uvm_report_server* get_report_server() const override;
  void set_report_server( uvm_report_server* server ) override;

  uvm_root* get_root() const override;

//  virtual void set_component_visitor( uvm_visitor<uvm_component> v );
//  virtual uvm_visitor<uvm_component> get_component_visitor() const;

 private:
  uvm_default_coreservice_t():factory(nullptr),report_server(nullptr){};
  ~uvm_default_coreservice_t() override{};

  mutable uvm_factory* factory;
//  mutable uvm_tr_database* tr_database;
  mutable uvm_report_server* report_server;
//  mutable uvm_visitor<uvm_component> m_visitor;

}; // class uvm_default_coreservice_t

} // namespace uvm

#endif // UVM_DEFAULT_CORESERVICE_T_
