//-----------------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2013 NXP B.V.
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License") { you may not use this file except in
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
//-----------------------------------------------------------------------------

#include <systemc>

#include "uvmsc/policy/uvm_recorder.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/misc/uvm_status_container.h"

using namespace sc_core;
using namespace sc_dt;

namespace uvm {

//----------------------------------------------------------------------
// Static data member initialization here
//----------------------------------------------------------------------

// Former globals: uvm_recorder::handle and uvm_recorder::m_handles() moved to uvm_coreservice_t.

int& uvm_recorder::handle_ref()
{
  return uvm_coreservice_t::get()->get_uvm_recorder_handle();
}

//--------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------
uvm_recorder::uvm_recorder( const std::string& name ) : uvm_object(name)
{
  tr_handle = 0;
  default_radix = UVM_HEX;
  physical = true;
  abstract = true;
  identifier = true;
  policy = UVM_DEFAULT_POLICY;
  scope = new uvm_scope_stack();
  filename = "tr_db.log";

  outstr = nullptr;
}

//--------------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------------
uvm_recorder::~uvm_recorder()
{
  delete scope;
}

//--------------------------------------------------------------------------
// member function: get_type_name
//--------------------------------------------------------------------------
// Implemented via the UVM_OBJECT_UTILS macro

//--------------------------------------------------------------------------
// member function: record_field
//
//! Records an integral field (less than or equal to 4096 bits). \p name is the
//! name of the field.
//!
//! \p value is the value of the field to record. \p size is the number of bits
//! of the field which apply. \p radix is the #uvm_radix_enum to use.
//--------------------------------------------------------------------------

void uvm_recorder::record_field( const std::string& name,
                                 uvm_bitstream_t value,
                                 int size,
                                 uvm_radix_enum radix )
{
  if(tr_handle==0)
    return;

  scope->set_arg(name);

  if(!radix)
    radix = default_radix;

  std::ostringstream str;
  str << value.to_string(SC_BIN);

  set_attribute(tr_handle, scope->get(), str.str(), radix, size);
}

//--------------------------------------------------------------------------
// member function: record_field_real
//
//! Records an real field. \p value is the value of the field to record.
//--------------------------------------------------------------------------

void uvm_recorder::record_field_real( const std::string& name,
                                      double value)
{
  std::ostringstream str;
  str << value;

  if(tr_handle == 0)
    return;

  scope->set_arg(name);

  set_attribute( tr_handle, scope->get(), str.str(), UVM_REAL, 64);
}


//--------------------------------------------------------------------------
// member function: record_object
//
//! Records an object field. \p name is the name of the recorded field.
//!
//! This method uses the #recursion_policy to determine whether or not to
//! recurse into the object.
//--------------------------------------------------------------------------

void uvm_recorder::record_object( const std::string& name, uvm_object* value )
{
  std::ostringstream str;

  if(identifier)
  {
    if(value != nullptr)
    {
      str << value->get_inst_id();
    }
    scope->set_arg(name);
    set_attribute(tr_handle, scope->get(), str.str(), UVM_DEC, 32);
  }

  if(policy != UVM_REFERENCE)
  {
    if(value != nullptr)
    {
      if(value->get_status_container()->cycle_check.find(value) != value->get_status_container()->cycle_check.end()) // exists
        return;

      value->get_status_container()->cycle_check[value] = 1;
      scope->down(name);
      value->record(this);
      scope->up();
      value->get_status_container()->cycle_check.erase(value);
    }
  }
}

//--------------------------------------------------------------------------
// member function: record_string
//
//! Records a string field. \p name is the name of the recorded field.
//--------------------------------------------------------------------------

void uvm_recorder::record_string( const std::string& name, const std::string& value)
{
  scope->set_arg(name);

  set_attribute( tr_handle,
                 scope->get(),
                 value,
                 UVM_STRING,
                 8*(int)value.length());
}


//--------------------------------------------------------------------------
// member function: record_time
//
//! Records a time value. \p name is the name to record to the database.
//--------------------------------------------------------------------------

void uvm_recorder::record_time( const std::string& name, const sc_time& value )
{
  scope->set_arg(name);

  set_attribute( tr_handle,
                 scope->get(),
                 value.to_string(),
                 UVM_TIME,
                 64 );
}


//--------------------------------------------------------------------------
// member function: record_generic
//
//! Records the \p name - \p value pair, where \p value has been converted
//! to a string.
//--------------------------------------------------------------------------

void uvm_recorder::record_generic( const std::string& name, const std::string& value )
{
  scope->set_arg(name);

  set_attribute( tr_handle,
                 scope->get(),
                 value,
                 UVM_STRING,
                 8*(int)value.length());
}


//--------------------------------------------------------------------------
// Group: Vendor-Independent API
//
// UVM provides only a text-based default implementation.
// Vendors provide subtype implementations and overwrite the
// #uvm_default_recorder handle.
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// member function: open_file
//
//! Opens the file in the #filename property and assigns to the
//! file descriptor #FILE.
//
//--------------------------------------------------------------------------

bool uvm_recorder::open_file()
{
  std::ios_base::openmode mode = std::ios_base::out;

  if(!file)
    file.open(filename.c_str(), mode);

  if(file)
  {
    outstr = &file;
    return true;
  }
  else
  {
    outstr = nullptr;
    return false;
  }
}


//--------------------------------------------------------------------------
// member function: create_stream
//--------------------------------------------------------------------------

int uvm_recorder::create_stream( const std::string& name,
                                 const std::string& t,
                                 const std::string& scope )
{
  if (open_file())
  {
    m_handles()[++handle_ref()] = 1;

   *outstr << "CREATE_STREAM @" << sc_time_stamp()
           << "NAME: " << name
           << "T: " << t
           << "SCOPE: " << scope
           << "STREAM: " << handle_ref();

    return handle_ref();
  }
  return 0;
}


//--------------------------------------------------------------------------
// member function: m_set_attribute
//--------------------------------------------------------------------------

void uvm_recorder::m_set_attribute( int txh,
                                    const std::string& nm,
                                    const std::string& value )
{
  if (open_file())
  {
    *outstr << "SET_ATTR @" << sc_time_stamp()
            << "TXH: " << txh
            << "NAME: " << nm
            << "VALUE: " << value;
  }
}


//--------------------------------------------------------------------------
// member function: set_attribute
//--------------------------------------------------------------------------

void uvm_recorder::set_attribute( int txh,
                                  const std::string& nm,
                                  const std::string& value, // TODO was logic [1023:0]
                                  uvm_radix_enum radix,
                                  int numbits)
{
  if (open_file())
  {
    *outstr << "SET_ATTR @" << sc_time_stamp()
            << "TXH: " << txh
            << "NAME: " << nm
            << "VALUE: " << value
            << "RADIX: " << uvm_radix_enum_name[radix]
            << "BITS = " << numbits;
  }

}


//--------------------------------------------------------------------------
// member function: check_handle_kind
//--------------------------------------------------------------------------

bool uvm_recorder::check_handle_kind( const std::string& htype, int handle )
{
  return (m_handles().find(handle) != m_handles().end()); //exists
}


//--------------------------------------------------------------------------
// member function: begin_tr
//--------------------------------------------------------------------------

int uvm_recorder::begin_tr( const std::string& txtype,
                            int stream,
                            const std::string& nm,
                            const std::string& label,
                            const std::string& desc,
                            const sc_time begin_time )
{
  if (open_file())
  {
    m_handles()[++handle_ref()] = 1;

    *outstr << "BEGIN @" << sc_time_stamp()
            << "TXH: " << handle_ref()
            << "STREAM: " << stream
            << "NAME: " << nm
            << "TIME = " << begin_time
            << "TYPE = " << txtype
            << "LABEL: " << label
            << "DESC: " << desc;

    return handle_ref();
  }
  return -1;
}


//--------------------------------------------------------------------------
// member function: end_tr
//--------------------------------------------------------------------------

void uvm_recorder::end_tr( int handle, sc_time end_time )
{
  if (open_file())
  {
    *outstr << "END @" << sc_time_stamp()
            << "TXH: " << handle
            << "TIME= " << end_time;
  }
}

//--------------------------------------------------------------------------
// member function: link_tr
//--------------------------------------------------------------------------

void uvm_recorder::link_tr( int h1,
                            int h2,
                            const std::string& relation )
{
  if (open_file())
  {
    *outstr << "LINK @" << sc_time_stamp()
            << "TXH1: " << h1
            << "TXH2: " << h2
            << "RELATION= " << relation;
  }
}

//--------------------------------------------------------------------------
// member function: free_tr
//--------------------------------------------------------------------------

void uvm_recorder::free_tr( int handle )
{
  if (open_file())
  {
    *outstr << "FREE @" << sc_time_stamp()
            << "TXH: " << handle;

    if (m_handles().find(handle) != m_handles().end()) //exists
      m_handles().erase(handle);
  }
}

//--------------------------------------------------------------------------
// member function: m_handles (static)
//--------------------------------------------------------------------------

std::map<int, bool>& uvm_recorder::m_handles()
{
  return uvm_coreservice_t::get()->get_uvm_recorder_m_handles();
}

/////////////

} // namespace uvm
