//------------------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
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
//------------------------------------------------------------------------------

#ifndef UVM_RESOURCE_CONVERTER_H_
#define UVM_RESOURCE_CONVERTER_H_

#include <string>
#include <typeinfo>
#include <sstream>
#include <type_traits>
#include <utility>

namespace uvm {

template <typename T, typename = void>
struct m_uvm_resource_is_streamable : std::false_type {};

template <typename T>
struct m_uvm_resource_is_streamable<T,
  std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T&>())>>
  : std::true_type {};

//------------------------------------------------------------------------------
// Class: uvm_resource_converter<T>
//
//! Implementation-defined class
//! The class uvm::uvm_resource_converter provides a policy object for doing
//! conversion from resource value to a string.
//------------------------------------------------------------------------------

template <typename T = int>
class m_uvm_resource_converter
{
 public:
  m_uvm_resource_converter() {};

  //----------------------------------------------------------------------------
  // member function: get_typeid
  //
  //! Return the typeid of the object passed as argument of this member
  //! function as string.
  //! It is used to print/dump the content of the resourced database
  //----------------------------------------------------------------------------

  std::string get_typeid(T val) const
  {
    if (typeid(val) == typeid(int))
      return "int";
    if (typeid(val) == typeid(int*))
      return "int*";
    if (typeid(val) == typeid(unsigned int))
      return "unsigned int";
    if (typeid(val) == typeid(unsigned int*))
      return "unsigned int*";
    if (typeid(val) == typeid(const unsigned int*))
      return "const unsigned int*";
    if (typeid(val) == typeid(long))
      return "long";
    if (typeid(val) == typeid(long*))
      return "long*";
    if (typeid(val) == typeid(const long*))
      return "const long*";
    if (typeid(val) == typeid(unsigned long))
      return "unsigned long";
    if (typeid(val) == typeid(unsigned long*))
      return "unsigned long*";
    if (typeid(val) == typeid(const unsigned long*))
      return "const unsigned long*";
    if (typeid(val) == typeid(long long))
      return "long long";
    if (typeid(val) == typeid(long long*))
      return "long long*";
    if (typeid(val) == typeid(const long long*))
      return "const long long*";
    if (typeid(val) == typeid(short))
      return "short";
    if (typeid(val) == typeid(short*))
      return "short*";
    if (typeid(val) == typeid(const short*))
      return "const short*";
    if (typeid(val) == typeid(unsigned short))
      return "unsigned short";
    if (typeid(val) == typeid(unsigned short*))
      return "unsigned short*";
    if (typeid(val) == typeid(const unsigned short*))
      return "const unsigned short*";
    if (typeid(val) == typeid(bool))
      return "bool";
    if (typeid(val) == typeid(bool*))
      return "bool*";
    if (typeid(val) == typeid(const bool*))
      return "const bool*";
    if (typeid(val) == typeid(std::string))
      return "std::string";
    if (typeid(val) == typeid(std::string*))
      return "std::string*";
    if (typeid(val) == typeid(const std::string*))
      return "const std::string*";
    if (typeid(val) == typeid(double))
      return "double";
    if (typeid(val) == typeid(double*))
      return "double*";
    if (typeid(val) == typeid(const double*))
      return "const double*";
    if (typeid(val) == typeid(long double))
      return "long double";
    if (typeid(val) == typeid(long double*))
      return "long double*";
    if (typeid(val) == typeid(const long double*))
      return "const long double*";
    if (typeid(val) == typeid(float))
      return "float";
    if (typeid(val) == typeid(float*))
      return "float*";
    if (typeid(val) == typeid(const float*))
      return "const float*";
    if (typeid(val) == typeid(char))
      return "char";
    if (typeid(val) == typeid(char*))
      return "char*";
    if (typeid(val) == typeid(const char*))
      return "const char*";
    if (typeid(val) == typeid(void))
      return "void";
    if (typeid(val) == typeid(void*))
      return "void*";
    if (typeid(val) == typeid(wchar_t))
      return "wchar_t";

    return "<unknown>"; // TODO add more types?
  }

  //----------------------------------------------------------------------------
  // member function: convert2string
  //
  //! Convert a value of type T to a string that can be displayed.
  //! By default, returns the name of the type
  //----------------------------------------------------------------------------

  virtual std::string convert2string(T val)
  {
    std::ostringstream s;
    s << "(" << get_typeid(val) << ") ";
    if constexpr (m_uvm_resource_is_streamable<T>::value)
      s << val;
    else
      s << "<value not printable>";
    return s.str();
  }

 private:
  // disabled
  virtual ~m_uvm_resource_converter() {};
};

} // namespace uvm

#endif // UVM_RESOURCE_CONVERTER_H_
