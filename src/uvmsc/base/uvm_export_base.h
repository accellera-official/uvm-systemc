//------------------------------------------------------------------------------
//   Copyright 2014 NXP B.V.
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

#ifndef UVM_EXPORT_BASE_H_
#define UVM_EXPORT_BASE_H_

#include <systemc>
#include "uvmsc/base/uvm_component.h"

//////////////

namespace uvm {

//--------------------------------------------------------------------------
// Note: This class in not part of the UVM Standard
//--------------------------------------------------------------------------

template <class IF>
class uvm_export_base : public sc_core::sc_export<IF>
{
 public:
  uvm_export_base();
  explicit uvm_export_base( const std::string& name );

  virtual std::string get_name() const;
  virtual std::string get_full_name() const;
  virtual uvm_component* get_parent() const;
  virtual std::string get_type_name() const;

  // exports can only bind to interfaces or other exports, not to ports
  virtual void connect( IF& interface_ );
  virtual void connect( uvm_export_base<IF>& export_ );

  virtual const char* kind() const;

 protected:
  virtual ~uvm_export_base();

  virtual void m_connect( IF& interface_ );

 private:
  std::string m_name;
};

//////////////////////

// Constructors
template<class IF>
inline uvm_export_base<IF>::uvm_export_base()
  : sc_core::sc_export<IF>()
{}

template<class IF>
inline uvm_export_base<IF>::uvm_export_base( const std::string& name )
  : sc_core::sc_export<IF>(name.c_str())
{
  m_name = name;
}

template<class IF>
inline std::string uvm_export_base<IF>::get_name() const
{
  return m_name;
}

template<class IF>
inline std::string uvm_export_base<IF>::get_full_name() const
{
  return this->name();
}

template<class IF>
inline std::string uvm_export_base<IF>::get_type_name() const
{
  return "uvm::uvm_export_base";
}

template<class IF>
inline uvm_component* uvm_export_base<IF>::get_parent() const
{
  sc_core::sc_object* obj = this->get_parent_object();
  uvm_component* comp = dynamic_cast<uvm_component* >(obj);
  return comp;
}

template<class IF>
inline void uvm_export_base<IF>::connect( IF& interface_ )
{
  this->bind(interface_);
}

template<class IF>
inline void uvm_export_base<IF>::connect( uvm_export_base<IF>& export_ )
{
  this->bind(export_);
}

// Destructor
template<class IF>
inline uvm_export_base<IF>::~uvm_export_base()
{}

// SystemC compatibility function
template<class IF>
inline const char* uvm_export_base<IF>::kind() const
{
  return "uvm::uvm_port_base";
}

// helper function for derived classes to bind to itself
template<class IF>
inline void uvm_export_base<IF>::m_connect( IF& interface_ )
{
  this->bind(interface_);
}

} // namespace uvm


#endif // UVM_EXPORT_BASE_H_
