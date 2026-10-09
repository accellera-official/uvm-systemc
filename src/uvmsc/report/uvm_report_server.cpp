//----------------------------------------------------------------------------
//   Copyright 2016-2017 NXP B.V.
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc. 
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2014 NVIDIA Corporation
//   Copyright 2018 Intel Corp.
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
//----------------------------------------------------------------------------

#include <ostream>
#include <iostream>
#include <iomanip>  // setw

#include "uvmsc/report/uvm_report_server.h"
#include "uvmsc/report/uvm_report_catcher.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"

namespace uvm {

//----------------------------------------------------------------------------
// Constructor
//
//! Creates an instance of the class.
//----------------------------------------------------------------------------

uvm_report_server::uvm_report_server( const std::string& name )
: uvm_object(name)
{}

//----------------------------------------------------------------------------
// member function: do_copy

void uvm_report_server::do_copy( const uvm_object& rhs )
{
  uvm_object::do_copy(rhs);

  const uvm_report_server* rhs_ = dynamic_cast<const uvm_report_server*>(&rhs);

  if(rhs_ == nullptr)
    UVM_ERROR("UVM/REPORT/SERVER/RPTCOPY", "Cannot copy to report_server from the given datatype.");
    //TODO should be FATAL?

  {
    std::vector<uvm_severity> q;
    rhs_->get_severity_set(q);
    for( unsigned int i = 0; i < q.size(); i++ )
      set_severity_count(q[i],rhs_->get_severity_count(q[i]));
  }

  {
    std::vector<std::string> q;
    rhs_->get_id_set(q);
    for( unsigned int i = 0; i < q.size(); i++ )
      set_id_count(q[i],rhs_->get_id_count(q[i]));
  }

  //TODO Transaction recording copy
  //set_message_database(rhs_->get_message_database());
  set_max_quit_count(rhs_->get_max_quit_count());
  set_quit_count(rhs_->get_quit_count());
}

//----------------------------------------------------------------------------
// member function: set_server (static)
//
//! Sets the global report server to use for reporting. The report
//! server is responsible for formatting messages.
//----------------------------------------------------------------------------

void uvm_report_server::set_server( std::shared_ptr<uvm_report_server> server )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  server->copy(*cs->get_report_server());
  cs->set_report_server(server);
}

//----------------------------------------------------------------------------
// member function: get_server (static)
//
//! Gets the global report server. The method will always return
//! a valid handle to a report server.
//----------------------------------------------------------------------------

std::shared_ptr<uvm_report_server> uvm_report_server::get_server()
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  return cs->get_report_server();
}

//----------------------------------------------------------------------------
// member function: get_type_name
//
// Internal member function.
//----------------------------------------------------------------------------

const std::string uvm_report_server::get_type_name() const
{
  return "uvm_report_server";
}


} // namespace uvm
