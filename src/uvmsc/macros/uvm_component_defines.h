//----------------------------------------------------------------------
//   Copyright 2009 Cadence Design Systems, Inc.
//   Copyright 2012-2014 NXP B.V.
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

#ifndef UVM_COMPONENT_DEFINES_H_
#define UVM_COMPONENT_DEFINES_H_

// -------------------------------------------
// MACRO: UVM_COMPONENT_UTILS
//
//! This macro is used to register the #uvm_component
//! to the factory and to add some dedicated
//! member functions
// -------------------------------------------

// both macros support template parameters, even though
// the UVM_COMPONENT_UTILS normally is for non-template classes
// but it is expected that some users will not know the difference

#define UVM_COMPONENT_UTILS(...) \
  M_UVM_COMPONENT_REGISTRY_INTERNAL_PARAM(__VA_ARGS__) \
  M_UVM_COMPONENT_GET_TYPE_NAME_FUNC_PARAM(__VA_ARGS__) \

#define UVM_COMPONENT_PARAM_UTILS(...) \
  M_UVM_COMPONENT_REGISTRY_INTERNAL_PARAM(__VA_ARGS__) \
  M_UVM_COMPONENT_GET_TYPE_NAME_FUNC_PARAM(__VA_ARGS__) \

// ----------------------------------------------
// MACRO: M_UVM_COMPONENT_REGISTRY_INTERNAL_PARAM
//
// Implementation-defined macro  ----
// ----------------------------------------------

#define M_UVM_COMPONENT_REGISTRY_INTERNAL_PARAM(...) \
  static const std::string m_register_type_name() { \
    return std::string(#__VA_ARGS__); } \
  typedef ::uvm::uvm_component_registry<__VA_ARGS__ > type_id; \
  static type_id* get_type() { return type_id::get(); } \
  using ::uvm::uvm_report_object::uvm_report_info; \
  using ::uvm::uvm_report_object::uvm_report_warning; \
  using ::uvm::uvm_report_object::uvm_report_error; \
  using ::uvm::uvm_report_object::uvm_report_fatal; \

/* TODO need this macro addition?
  virtual uvm_component_wrapper get_object_type() const { \
     return type_id::get(); \
   }
*/

// -------------------------------------------
// MACRO: M_UVM_GET_TYPE_NAME_FUNC_PARAM
//
// Implementation-defined macro
// -------------------------------------------

#define M_UVM_COMPONENT_GET_TYPE_NAME_FUNC_PARAM(...) \
  const std::string get_type_name() const override { \
    return #__VA_ARGS__; \
  }


#endif /* UVM_COMPONENT_DEFINES_H_ */
