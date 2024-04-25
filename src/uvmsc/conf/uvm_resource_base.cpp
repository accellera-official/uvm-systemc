//----------------------------------------------------------------------
//   Copyright 2011 Cypress Semiconductor
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2011 Cadence Design Systems, Inc.
//   Copyright 2012-2015 NXP B.V.
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2014 Université Pierre et Marie Curie, Paris
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

#include <vector>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/conf/uvm_resource_base.h"
#include "uvmsc/conf/uvm_resource_options.h"
#include "uvmsc/conf/uvm_resource_types.h"
#include "uvmsc/print/uvm_printer.h"
#include "uvmsc/misc/uvm_misc.h"
#include "uvmsc/macros/uvm_message_defines.h"
#include "uvmsc/macros/uvm_string_defines.h"


using namespace sc_core;

namespace uvm {

//----------------------------------------------------------------------
// initialize static data members
//----------------------------------------------------------------------

unsigned int uvm_resource_base::default_precedence = 1000;

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------

uvm_resource_base::uvm_resource_base( const std::string& name,
                                      const std::string& s ) : uvm_object(name)
{
  set_scope(s);
  read_only = false;
  precedence = default_precedence;

  if(uvm_has_wildcard(name))
    m_is_regex_name = true;
  else
	m_is_regex_name = false;
}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------

uvm_resource_base::~uvm_resource_base()
{}

//----------------------------------------------------------------------
// member function: set_read_only
//
//! Establishes this resource as a read-only resource. An attempt
//! to call uvm_resource<T>::write on the resource will cause an error.
//----------------------------------------------------------------------

void uvm_resource_base::set_read_only()
{
  read_only = true;
}

//----------------------------------------------------------------------
// member function: set_read_write
//
//! Implementation defined
//!
//! Returns the resource to normal read-write capability.
//! Implementation question: Not sure if this function is necessary.
//! Once a resource is set to read_only no one should be able to change
//! that. If anyone can flip the read_only bit then the resource is not
//! truly read_only.
//----------------------------------------------------------------------

void uvm_resource_base::set_read_write()
{
  read_only = false;
}

//----------------------------------------------------------------------
// member function: is_read_only
//
//! Returns true if this resource has been set to read-only, and false
//! otherwise
//----------------------------------------------------------------------

bool uvm_resource_base::is_read_only() const
{
  return read_only;
}

//----------------------------------------------------------------------
// member function: wait_modified
//
//! This member function blocks until the resource has been modified -- that is, a
//! uvm_resource<T>::write operation has been performed.  When a
//! uvm_resource<T>::write is performed the modified bit is set which
//! releases the block. Wait_modified() then clears the modified bit so
//! it can be called repeatedly.
//----------------------------------------------------------------------

void uvm_resource_base::wait_modified()
{
  sc_core::wait(modified);
}

//----------------------------------------------------------------------
// member function: set_scope
//
//! Set the value of the regular expression that identifies the set of
//! scopes over which this resource is visible. If the supplied
//! argument is a glob it will be converted to a regular expression
//! before it is stored.
//----------------------------------------------------------------------

void uvm_resource_base::set_scope( const std::string& s )
{
  scope = uvm_glob_to_re(s);
}

//----------------------------------------------------------------------
// member function: get_scope
//
//! Retrieve the regular expression string that identifies the set of
//! scopes over which this resource is visible.
//----------------------------------------------------------------------

std::string uvm_resource_base::get_scope() const
{
  return scope;
}

//----------------------------------------------------------------------
// member function: match_scope
//
//! Using the regular expression facility, determine if this resource
//! is visible in a scope.  Return true if it is, false otherwise.
//----------------------------------------------------------------------

bool uvm_resource_base::match_scope( const std::string& s )
{
  bool err = uvm_re_match(scope, s);
  return !err;
}

//----------------------------------------------------------------------
// member function convert2string
//
//! Implementation defined
//!
//! Create a string representation of the resource value. By default
//! we don't know how to do this so we just return a "?". Resource
//! specializations are expected to override this function to produce a
//! proper string representation of the resource value.
//----------------------------------------------------------------------

std::string uvm_resource_base::convert2string() const
{
  return "?";
}

//----------------------------------------------------------------------
// member function: do_print
//
//! Implementation of do_print which is called by print().
//----------------------------------------------------------------------

void uvm_resource_base::do_print( const uvm_printer& printer ) const
{
  printer.print_string("", get_name() + " [" + get_scope() + "] : " + convert2string() );
}


//----------------------------------------------------------------------
// member function: record_read_access
//----------------------------------------------------------------------

void uvm_resource_base::record_read_access( uvm_object* accessor )
{
  std::string str;

  // If an accessor object is supplied then get the accessor record.
  // Otherwise create a new access record.  In either case populate
  // the access record with information about this access.  Check
  // first to make sure that auditing is turned on.

  if( uvm_resource_options::is_auditing() )
  {
    if(accessor != NULL)
    {
      uvm_resource_types::access_t access_record;
      str = accessor->get_full_name();
      if(access.find(str) != access.end()) // exists
        access_record = access[str];
      else
        init_access_record(access_record);
      access_record.read_count++;
      access_record.read_time = sc_time_stamp();
      access[str] = access_record;
    }
  }
}

//----------------------------------------------------------------------
// function: record_write_access
//----------------------------------------------------------------------

void uvm_resource_base::record_write_access( uvm_object* accessor )
{
  std::string str;

  // If an accessor object is supplied then get the accessor record.
  // Otherwise create a new access record.  In either case populate
  // the access record with information about this access.  Check
  // first that auditing is turned on

  if(uvm_resource_options::is_auditing())
  {
    if(accessor != NULL)
    {
      uvm_resource_types::access_t access_record;
      std::string str;
      str = accessor->get_full_name();
      if(access.find(str) != access.end()) // exists
        access_record = access[str];
      else
        init_access_record(access_record);
      access_record.write_count++;
      access_record.write_time = sc_time_stamp();
      access[str] = access_record;
    }
  }
}

//----------------------------------------------------------------------
// Function: print_accessors
//
//! Dump the access records for this resource
//----------------------------------------------------------------------

void uvm_resource_base::print_accessors() const
{
  uvm_resource_types::access_t access_record;
  std::vector<std::string> qs;

  if( access.size() == 0 )
    return;

  for(access_mapcItT it = access.begin(); it != access.end(); it++)
  {
    std::ostringstream str;
    str << it->first;
    access_record = it->second;
    str << " reads: "
        << access_record.read_count
        << " @ "
        << access_record.read_time
        << "  writes: "
        << access_record.write_count
        << " @ "
        << access_record.write_time
        << "\n";
    qs.push_back(str.str());
  }

  UVM_INFO("UVM/RESOURCE/ACCESSOR", UVM_STRING_QUEUE_STREAMING_PACK(qs), UVM_NONE);
}


//----------------------------------------------------------------------
// member function: init_access_record
//
//! Initialize a new access record
//----------------------------------------------------------------------

void uvm_resource_base::init_access_record( uvm_resource_types::access_t access_record )
{
  access_record.read_time = SC_ZERO_TIME;
  access_record.write_time = SC_ZERO_TIME;
  access_record.read_count = 0;
  access_record.write_count = 0;
}


} // namespace uvm
