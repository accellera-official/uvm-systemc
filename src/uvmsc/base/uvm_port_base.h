//------------------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the "License"); you may not
//   use this file except in compliance with the License.  You may obtain a copy
//   of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
//   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
//   License for the specific language governing permissions and limitations
//   under the License.
//------------------------------------------------------------------------------

#ifndef UVM_PORT_BASE_H_
#define UVM_PORT_BASE_H_

#include <systemc>
#include "uvmsc/base/uvm_component.h"
//////////////

namespace uvm {

template <class IF>
class uvm_port_base : public sc_core::sc_port<IF>
{
 public:
  template<typename REQ, typename RSP>
  friend class uvm_driver;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  uvm_port_base();
  explicit uvm_port_base( const std::string& name );

  virtual std::string get_name() const;
  virtual std::string get_full_name() const;
  virtual uvm_component* get_parent() const;
  virtual std::string get_type_name() const;

  virtual void connect( IF& interface_ );
  virtual void connect( uvm_port_base<IF>& port_ );

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  virtual const char* kind() const;

  virtual ~uvm_port_base();

 private:
  std::string m_name;
};

//////////////////////

// Constructors
template<class IF>
inline uvm_port_base<IF>::uvm_port_base()
  : sc_core::sc_port<IF>()
{}

template<class IF>
inline uvm_port_base<IF>::uvm_port_base( const std::string& name )
  : sc_core::sc_port<IF>(name.c_str())
{
  m_name = name;
}

template<class IF>
inline std::string uvm_port_base<IF>::get_name() const
{
  return m_name;
}

template<class IF>
inline std::string uvm_port_base<IF>::get_full_name() const
{
  return this->name();
}

template<class IF>
inline std::string uvm_port_base<IF>::get_type_name() const
{
  return "uvm::uvm_port_base";
}

template<class IF>
inline uvm_component* uvm_port_base<IF>::get_parent() const
{
  sc_core::sc_object* obj = this->get_parent_object();
  uvm_component* comp = dynamic_cast<uvm_component* >(obj);
  return comp;
}

template<class IF>
inline void uvm_port_base<IF>::connect( IF& interface_ )
{
  this->bind(interface_);
}

template<class IF>
inline void uvm_port_base<IF>::connect( uvm_port_base<IF>& port_ )
{
  this->bind(port_);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

// Destructor
template<class IF>
inline uvm_port_base<IF>::~uvm_port_base()
{}

// SystemC compatibility function
template<class IF>
inline const char* uvm_port_base<IF>::kind() const
{
  return "uvm::uvm_port_base";
}


} // namespace uvm


#endif // UVM_PORT_BASE_H_
