//----------------------------------------------------------------------
//   Copyright 2012-2015 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
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

#ifndef UVM_PRINTER_H_
#define UVM_PRINTER_H_

#include <list>
#include <memory>
#include <systemc>

#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/print/uvm_printer_knobs.h"
#include "uvmsc/misc/uvm_scope_stack.h"

//////////////

namespace uvm {

// forward declaration
class uvm_object;

typedef struct {
  int    level;
  std::string name;
  std::string type_name;
  std::string size;
  std::string val;
} uvm_printer_row_info;


//------------------------------------------------------------------------------
// Class: uvm_printer
//
//! The class #uvm_printer provides an interface for printing objects of type
//! #uvm_object in various formats. Subtypes of #uvm_printer implement
//! different print formats or policies.
//------------------------------------------------------------------------------

class uvm_printer
{
 public:
  friend class uvm_object;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  static std::shared_ptr<uvm_printer> get_default();

  static void set_default(std::shared_ptr<uvm_printer>);

  //--------------------------------------------------------------------------
  // Group: Methods for printer usage
  //--------------------------------------------------------------------------

  virtual void print_field( const std::string& name,
                            const uvm_bitstream_t& value,
                            int size = -1, // TODO default size not part of standard, but convenient?
                            uvm_radix_enum radix = UVM_NORADIX,
                            const char* scope_separator = ".",
                            const std::string& type_name = "" ) const;

  virtual void print_field_int( const std::string& name,
                                const uvm_integral_t& value,
                                int size = -1, // TODO default size not part of standard, but convenient?
                                uvm_radix_enum radix = UVM_NORADIX,
                                const char* scope_separator = ".",
                                const std::string& type_name = "" ) const;

  virtual void print_real( const std::string& name,
                           double value,
                           const char* scope_separator = "." ) const;

  virtual void print_double( const std::string& name,
                             double value,
                             const char* scope_separator = "." ) const;

  virtual void print_object( const std::string& name,
                             const uvm_object& value,
                             const char* scope_separator = "." ) const;

  virtual void print_object_header( const std::string& name,
                                    const uvm_object& value,
                                    const char* scope_separator = "." ) const;

  virtual void print_string( const std::string& name,
                             const std::string& value,
                             const char* scope_separator = "." ) const;

  virtual void print_time( const std::string& name,
                           const sc_core::sc_time& value,
                           const char* scope_separator = "." ) const;

  virtual void print_generic( const std::string& name,
                              const std::string& type_name,
                              int size,
                              const std::string& value,
                              const char* scope_separator = "." ) const;

  //--------------------------------------------------------------------------
  // Group: Methods for printer subtyping
  //--------------------------------------------------------------------------

  virtual std::string emit();

  virtual std::string format_row( const uvm_printer_row_info& row );

  virtual std::string format_header();

  virtual std::string format_footer();

  std::string adjust_name( const std::string& id,
                           const char* scope_separator = "." ) const;

  virtual void print_array_header( const std::string& name,
                                   int size,
                                   const std::string& arraytype = "array",
                                   const char* scope_separator = "." ) const;

  void print_array_range( int min, int max ) const;

  void print_array_footer( int size = 0 ) const;

  // member variable: knobs
  //
  //! The knob object provides access to the variety of knobs associated with a
  //! specific printer instance.

  uvm_printer_knobs knobs;

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  // destructor
  virtual ~uvm_printer();

 private:

  bool istop() const;

  // member variables below

  mutable uvm_scope_stack m_scope;

  std::string m_string;

 protected:
  // constructor disabled for application
  uvm_printer();

  typedef std::vector<uvm_printer_row_info> m_row_vecT;
  typedef m_row_vecT::iterator m_row_vecItT;
  mutable m_row_vecT m_rows;

  std::list<bool> m_array_stack;

}; // class uvm_printer


//////////////

} // namespace uvm

#endif /* UVM_PRINTER_H_ */
