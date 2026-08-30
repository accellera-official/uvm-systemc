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

#ifndef UVM_CORESERVICE_T_
#define UVM_CORESERVICE_T_

#include <systemc>
#include <memory>

//////////////

namespace uvm {

// forward declaration of necessary classes.
class uvm_factory;
class uvm_default_factory;
class uvm_report_server;
class uvm_default_report_server;
class uvm_root;
//class uvm_visitor;
//class uvm_component_name_check_visitor;
class uvm_component;
//class uvm_tr_database;
//class uvm_text_tr_database;
class uvm_packer;
class uvm_comparer;
class uvm_printer;
class uvm_default_coreservice_t;

//----------------------------------------------------------------------
// Class: uvm_coreservice_t
//
// The singleton instance of uvm_coreservice_t provides a common point for all central 
// uvm services such as uvm_factory, uvm_report_server, ...
// The service class provides a static <::get> which returns an instance adhering to uvm_coreservice_t
// the rest of the set_<facility> get_<facility> pairs provide access to the internal uvm services
//
// Custom implementations of uvm_coreservice_t can be included in uvm_pkg::*
// and can selected via the define UVM_CORESERVICE_TYPE. They cannot reside in another package.
//----------------------------------------------------------------------

class uvm_coreservice_t
{
 public:

  virtual std::shared_ptr<uvm_factory> get_factory() = 0;
  virtual void set_factory( std::shared_ptr<uvm_factory> f ) = 0;

  virtual std::shared_ptr<uvm_report_server> get_report_server() = 0;
  virtual void set_report_server( std::shared_ptr<uvm_report_server> ) = 0;

//  virtual uvm_tr_database* get_default_tr_database() = 0;
//  virtual void set_default_tr_database( uvm_tr_database db ) = 0;

//  virtual void set_component_visitor( uvm_visitor<uvm_component> v ) = 0;
//  virtual uvm_visitor<uvm_component> get_component_visitor() const = 0;

  virtual uvm_root* get_root() = 0;
  
  virtual std::shared_ptr<uvm_packer> get_default_packer() = 0;
  virtual void set_default_packer(std::shared_ptr<uvm_packer>) = 0;
  

  virtual std::shared_ptr<uvm_comparer> get_default_comparer() = 0;
  virtual void set_default_comparer(std::shared_ptr<uvm_comparer>) = 0;
  

  virtual std::shared_ptr<uvm_printer> get_default_printer() = 0;
  virtual void set_default_printer(std::shared_ptr<uvm_printer>) = 0;
  
  static uvm_default_coreservice_t* get();

 protected:
  uvm_coreservice_t(){};
  virtual ~uvm_coreservice_t(){}

}; // class uvm_coreservice_t



} // namespace uvm

#endif // UVM_CORESERVICE_T_
