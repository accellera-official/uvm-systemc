//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
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

#include "uvmsc/base/uvm_component.h"
#include "uvmsc/phasing/uvm_runtime_phases.h"


//////////////

namespace uvm {

//----------------------------------------------------------------------
// Class uvm_pre_reset_phase implementation
//----------------------------------------------------------------------

uvm_pre_reset_phase::uvm_pre_reset_phase()
  : uvm_process_phase("pre_reset")
{}

uvm_pre_reset_phase::uvm_pre_reset_phase( const std::string& name )
  : uvm_process_phase(name)
{}

void uvm_pre_reset_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->pre_reset_phase(*phase);
}

uvm_pre_reset_phase* uvm_pre_reset_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_pre_reset_phase();
 return m_inst;
}

const std::string uvm_pre_reset_phase::get_type_name() const
{
  return type_name;
}

uvm_pre_reset_phase* uvm_pre_reset_phase::m_inst = NULL;
const std::string uvm_pre_reset_phase::type_name = "uvm_pre_reset_phase";

//----------------------------------------------------------------------
// Class uvm_reset_phase implementation
//----------------------------------------------------------------------

uvm_reset_phase::uvm_reset_phase()
  : uvm_process_phase("reset")
{}

uvm_reset_phase::uvm_reset_phase( const std::string& name )
  : uvm_process_phase(name)
{}

void uvm_reset_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->reset_phase(*phase);
}

uvm_reset_phase* uvm_reset_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_reset_phase();
  return m_inst;
}

const std::string uvm_reset_phase::get_type_name() const
{
  return type_name;
}

uvm_reset_phase* uvm_reset_phase::m_inst = NULL;
const std::string uvm_reset_phase::type_name = "uvm_reset_phase";

//----------------------------------------------------------------------
// Class uvm_post_reset_phase implementation
//----------------------------------------------------------------------

uvm_post_reset_phase::uvm_post_reset_phase()
  : uvm_process_phase("post_reset")
{}

uvm_post_reset_phase::uvm_post_reset_phase( const std::string& name )
  : uvm_process_phase(name)
{}

void uvm_post_reset_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->post_reset_phase(*phase);
}

uvm_post_reset_phase* uvm_post_reset_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_post_reset_phase();
 return m_inst;
}

const std::string uvm_post_reset_phase::get_type_name() const
{
  return type_name;
}

uvm_post_reset_phase* uvm_post_reset_phase::m_inst = NULL;
const std::string uvm_post_reset_phase::type_name = "uvm_post_reset_phase";

//----------------------------------------------------------------------
// Class uvm_pre_configure_phase implementation
//----------------------------------------------------------------------

uvm_pre_configure_phase::uvm_pre_configure_phase()
  : uvm_process_phase("pre_configure")
{}

uvm_pre_configure_phase::uvm_pre_configure_phase( const std::string& name )
  : uvm_process_phase(name)
{}

void uvm_pre_configure_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->pre_configure_phase(*phase);
}

uvm_pre_configure_phase* uvm_pre_configure_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_pre_configure_phase();
 return m_inst;
}

const std::string uvm_pre_configure_phase::get_type_name() const
{
  return type_name;
}

uvm_pre_configure_phase* uvm_pre_configure_phase::m_inst = NULL;
const std::string uvm_pre_configure_phase::type_name = "uvm_pre_configure_phase";

//----------------------------------------------------------------------
// Class uvm_configure_phase implementation
//----------------------------------------------------------------------

uvm_configure_phase::uvm_configure_phase()
  : uvm_process_phase("configure")
{}

uvm_configure_phase::uvm_configure_phase( const std::string& name )
  : uvm_process_phase(name)
{}

void uvm_configure_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->configure_phase(*phase);
}

uvm_configure_phase* uvm_configure_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_configure_phase();
 return m_inst;
}

const std::string uvm_configure_phase::get_type_name() const
{
  return type_name;
}

uvm_configure_phase* uvm_configure_phase::m_inst = NULL;
const std::string uvm_configure_phase::type_name = "uvm_configure_phase";

//----------------------------------------------------------------------
// Class uvm_post_configure_phase implementation
//----------------------------------------------------------------------

uvm_post_configure_phase::uvm_post_configure_phase()
 : uvm_process_phase("post_configure")
{}

uvm_post_configure_phase::uvm_post_configure_phase( const std::string& name )
 : uvm_process_phase(name)
{}

void uvm_post_configure_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->post_configure_phase(*phase);
}

uvm_post_configure_phase* uvm_post_configure_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_post_configure_phase();
 return m_inst;
}

const std::string uvm_post_configure_phase::get_type_name() const
{
  return type_name;
}

uvm_post_configure_phase* uvm_post_configure_phase::m_inst = NULL;
const std::string uvm_post_configure_phase::type_name = "uvm_post_configure_phase";

//----------------------------------------------------------------------
// Class uvm_pre_main_phase implementation
//----------------------------------------------------------------------

uvm_pre_main_phase::uvm_pre_main_phase()
  : uvm_process_phase("pre_main")
{}

uvm_pre_main_phase::uvm_pre_main_phase( const std::string& name )
  : uvm_process_phase(name)
{}

void uvm_pre_main_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->pre_main_phase(*phase);
}

uvm_pre_main_phase* uvm_pre_main_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_pre_main_phase();
 return m_inst;
}

const std::string uvm_pre_main_phase::get_type_name() const
{
  return type_name;
}

uvm_pre_main_phase* uvm_pre_main_phase::m_inst = NULL;
const std::string uvm_pre_main_phase::type_name = "uvm_pre_main_phase";

//----------------------------------------------------------------------
// Class uvm_main_phase implementation
//----------------------------------------------------------------------

uvm_main_phase::uvm_main_phase()
  : uvm_process_phase("main")
{}

uvm_main_phase::uvm_main_phase( const std::string& name )
  : uvm_process_phase(name)
{}

void uvm_main_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->main_phase(*phase);
}

uvm_main_phase* uvm_main_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_main_phase();
 return m_inst;
}

const std::string uvm_main_phase::get_type_name() const
{
  return type_name;
}

uvm_main_phase* uvm_main_phase::m_inst = NULL;
const std::string uvm_main_phase::type_name = "uvm_main_phase";

//----------------------------------------------------------------------
// Class uvm_post_main_phase implementation
//----------------------------------------------------------------------

uvm_post_main_phase::uvm_post_main_phase()
  : uvm_process_phase("post_main")
{}

uvm_post_main_phase::uvm_post_main_phase( const std::string& name )
  : uvm_process_phase(name)
{}

void uvm_post_main_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->post_main_phase(*phase);
}

uvm_post_main_phase* uvm_post_main_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_post_main_phase();
 return m_inst;
}

const std::string uvm_post_main_phase::get_type_name() const
{
  return type_name;
}

uvm_post_main_phase* uvm_post_main_phase::m_inst = NULL;
const std::string uvm_post_main_phase::type_name = "uvm_post_main_phase";

//----------------------------------------------------------------------
// Class uvm_pre_shutdown_phase implementation
//----------------------------------------------------------------------

uvm_pre_shutdown_phase::uvm_pre_shutdown_phase()
  : uvm_process_phase("pre_shutdown")
{}

uvm_pre_shutdown_phase::uvm_pre_shutdown_phase( const std::string& name )
  : uvm_process_phase(name)
{}

void uvm_pre_shutdown_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->pre_shutdown_phase(*phase);
}

uvm_pre_shutdown_phase* uvm_pre_shutdown_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_pre_shutdown_phase();
 return m_inst;
}

const std::string uvm_pre_shutdown_phase::get_type_name() const
{
  return type_name;
}

uvm_pre_shutdown_phase* uvm_pre_shutdown_phase::m_inst = NULL;
const std::string uvm_pre_shutdown_phase::type_name = "uvm_pre_shutdown_phase";

//----------------------------------------------------------------------
// Class uvm_shutdown_phase implementation
//----------------------------------------------------------------------

uvm_shutdown_phase::uvm_shutdown_phase()
  : uvm_process_phase("shutdown")
{}

uvm_shutdown_phase::uvm_shutdown_phase( const std::string& name )
  : uvm_process_phase(name)
{}

void uvm_shutdown_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->shutdown_phase(*phase);
}

uvm_shutdown_phase* uvm_shutdown_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_shutdown_phase();
 return m_inst;
}

const std::string uvm_shutdown_phase::get_type_name() const
{
  return type_name;
}

uvm_shutdown_phase* uvm_shutdown_phase::m_inst = NULL;
const std::string uvm_shutdown_phase::type_name = "uvm_shutdown_phase";

//----------------------------------------------------------------------
// Class uvm_post_shutdown_phase implementation
//----------------------------------------------------------------------

uvm_post_shutdown_phase::uvm_post_shutdown_phase( )
  : uvm_process_phase("post_shutdown")
{}

uvm_post_shutdown_phase::uvm_post_shutdown_phase( const std::string& name )
  : uvm_process_phase(name)
{}

void uvm_post_shutdown_phase::exec_process( uvm_component* comp, uvm_phase* phase )
{
  comp->post_shutdown_phase(*phase);
}

uvm_post_shutdown_phase* uvm_post_shutdown_phase::get()
{
  if(m_inst == NULL)
    m_inst = new uvm_post_shutdown_phase();
 return m_inst;
}

const std::string uvm_post_shutdown_phase::get_type_name() const
{
  return type_name;
}

uvm_post_shutdown_phase* uvm_post_shutdown_phase::m_inst = NULL;
const std::string uvm_post_shutdown_phase::type_name = "uvm_post_shutdown_phase";

} // namespace uvm
