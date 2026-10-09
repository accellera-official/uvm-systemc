//----------------------------------------------------------------------
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
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

#ifndef UVM_DERIVED_CALLBACKS_H_
#define UVM_DERIVED_CALLBACKS_H_

#include <typeinfo>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/cb/uvm_callback.h"
#include "uvmsc/cb/uvm_callbacks.h"
#include "uvmsc/cb/uvm_typeid.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"

namespace uvm {

//------------------------------------------------------------------------------
// Class: uvm_derived_callbacks<T,ST,CB>
//
//! This type is not really expected to be used directly by the user, instead they are
//! expected to use the macro `uvm_set_super_type. The sole purpose of this type is to
//! allow for setting up of the derived_type/super_type mapping.
//------------------------------------------------------------------------------

template <typename T = uvm_object, typename ST = uvm_object, typename CB = uvm_callback>
class uvm_derived_callbacks : public uvm_callbacks<T,CB>
{
 public:

  typedef uvm_derived_callbacks<T,ST,CB> this_type;
  typedef uvm_callbacks<T>               this_user_type;
  typedef uvm_callbacks<ST>              this_super_type;

  static uvm_derived_callbacks<T,ST,CB>* get();

  static bool register_super_type( const std::string& tname = "", const std::string& sname = "" );

 private:
  static std::string m_key_prefix()
  {
    return std::string(typeid(T).name()) + "::" + typeid(ST).name() + "::" + typeid(CB).name();
  }

  static this_type*& m_d_inst_ref()
  {
    return uvm_coreservice_t::get()->get_or_create_typed_store<this_type*>(
      std::string("uvm_derived_callbacks::m_d_inst:") + m_key_prefix(), nullptr);
  }

  static this_user_type*& m_user_inst_ref()
  {
    return uvm_coreservice_t::get()->get_or_create_typed_store<this_user_type*>(
      std::string("uvm_derived_callbacks::m_user_inst:") + m_key_prefix(), nullptr);
  }

  static this_super_type*& m_super_inst_ref()
  {
    return uvm_coreservice_t::get()->get_or_create_typed_store<this_super_type*>(
      std::string("uvm_derived_callbacks::m_super_inst:") + m_key_prefix(), nullptr);
  }

  static uvm_typeid_base*& m_s_typeid_ref()
  {
    return uvm_coreservice_t::get()->get_or_create_typed_store<uvm_typeid_base*>(
      std::string("uvm_derived_callbacks::m_s_typeid:") + m_key_prefix(), nullptr);
  }

  // data members

}; // class uvm_derived_callbacks

//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: get (static)
//
//! Implementation defined
//----------------------------------------------------------------------------

template <typename T, typename ST, typename CB>
uvm_derived_callbacks<T,ST,CB>* uvm_derived_callbacks<T,ST,CB>::get()
{
  this_user_type*& m_user_inst = m_user_inst_ref();
  this_super_type*& m_super_inst = m_super_inst_ref();
  uvm_typeid_base*& m_s_typeid = m_s_typeid_ref();
  this_type*& m_d_inst = m_d_inst_ref();

  m_user_inst = this_user_type::get();
  m_super_inst = this_super_type::get();
  m_s_typeid = uvm_typeid<ST>::get();

  if(m_d_inst == nullptr)
    m_d_inst = new uvm_derived_callbacks<T,ST,CB>();

  return m_d_inst;
}

//----------------------------------------------------------------------------
// member function: register_super_type (static)
//
//! Implementation defined
//----------------------------------------------------------------------------

template <typename T, typename ST, typename CB>
bool uvm_derived_callbacks<T,ST,CB>::register_super_type( const std::string& tname, const std::string& sname )
{
  this_user_type* u_inst = this_user_type::get();
  this_type* inst = this_type::get();
  uvm_callbacks_base* s_obj  = nullptr;

  this_user_type::m_typename_ref() = tname;

  if(!sname.empty())
    m_s_typeid_ref()->type_name = sname;

  if(u_inst->m_super_type != nullptr)
  {
    if(u_inst->m_super_type == m_s_typeid_ref())
      return true;

    uvm_report_warning("CBTPREG", "Type " + tname + " is already registered to super type "
        + this_super_type::m_typename_ref() + ". Ignoring attempt to register to super type "
        + sname, UVM_NONE);
    return true;
  }

  if(this_super_type::m_typename_ref().empty())
    this_super_type::m_typename_ref() = sname;

  u_inst->m_super_type = m_s_typeid_ref();
  u_inst->m_base_inst_ref()->m_super_type = m_s_typeid_ref();

  s_obj = uvm_typeid_base::get_cb(m_s_typeid_ref());

  s_obj->m_derived_types.push_back(uvm_callbacks<T,CB>::m_typeid_ref());

  return true;
}


} // namespace uvm

#endif // UVM_DERIVED_CALLBACKS_H_
