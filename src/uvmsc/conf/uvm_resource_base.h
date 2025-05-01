//----------------------------------------------------------------------
//   Copyright 2012-2015 NXP B.V.
//   Copyright 2011 Cypress Semiconductor
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2011 Cadence Design Systems, Inc.
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

#ifndef UVM_RESOURCE_BASE_H_
#define UVM_RESOURCE_BASE_H_

#include <systemc>
#include <map>
#include <string>

#include "uvmsc/conf/uvm_resource_types.h"

namespace uvm {


// forward class references
class uvm_resource_base;
//class m_uvm_resource_converter;

//----------------------------------------------------------------------
// Class: uvm_resource_base
//
//! Non-parameterized base class for resources.  Supports interfaces for
//! scope matching, and virtual functions for printing the resource and
//! for printing the accessor list
//----------------------------------------------------------------------

class uvm_resource_base : public uvm_object
{
  friend class uvm_resource_pool;

public:

  //--------------------------------------------------------------------------
  // Constructor and destructor
  //--------------------------------------------------------------------------

  uvm_resource_base( const std::string& name = "",
                     const std::string& s = "*" );

  ~uvm_resource_base() override;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Group: Resource database interface
  //--------------------------------------------------------------------------

  virtual uvm_resource_base* get_type_handle() const = 0;

  //--------------------------------------------------------------------------
  // Group: Read-only Interface
  //--------------------------------------------------------------------------

  void set_read_only();
  bool is_read_only() const;

  //--------------------------------------------------------------------------
  // Group: Notification
  //--------------------------------------------------------------------------

  void wait_modified();

  //--------------------------------------------------------------------------
  // Group: Scope Interface
  //--------------------------------------------------------------------------

  void set_scope( const std::string& s );
  std::string get_scope() const;
  bool match_scope( const std::string& s );

  //--------------------------------------------------------------------------
  // Group: Priority
  //--------------------------------------------------------------------------

  virtual void set_priority( uvm_resource_types::priority_e pri ) = 0;

  //--------------------------------------------------------------------------
  // Group: Utility Functions
  //--------------------------------------------------------------------------

  void do_print( const uvm_printer& printer ) const override;

  //--------------------------------------------------------------------------
  // Group: Audit Trail
  //--------------------------------------------------------------------------

  void record_read_access( uvm_object* accessor = nullptr );
  void record_write_access( uvm_object* accessor = nullptr );

  virtual void print_accessors() const;

  void init_access_record( uvm_resource_types::access_t access_record );

  //--------------------------------------------------------------------------
  // public variables
  //--------------------------------------------------------------------------

  unsigned int precedence;

  static int unsigned default_precedence;

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

private:

  std::string convert2string() const override;

  void set_read_write();

  // disable copy constructor
  uvm_resource_base( const uvm_resource_base& );

  // data members

  typedef std::map<std::string, uvm_resource_types::access_t > access_mapT;
  typedef access_mapT::iterator access_mapItT;
  typedef access_mapT::const_iterator access_mapcItT;
  access_mapT access;
  access_mapItT access_mapIt;

  bool m_is_regex_name;

 protected:
  std::string scope;
  sc_core::sc_event modified;
  bool read_only;

}; // class uvm_resource_base


} // namespace uvm

#endif // UVM_RESOURCE_BASE_H_
