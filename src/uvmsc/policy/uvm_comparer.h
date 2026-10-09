//----------------------------------------------------------------------
//   Copyright 2012-2013 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
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

#ifndef UVM_COMPARER_H_
#define UVM_COMPARER_H_

#include <systemc>
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/misc/uvm_misc.h"
#include "uvmsc/misc/uvm_copy_map.h"

//////////////

namespace uvm {

//------------------------------------------------------------------------------
// CLASS: uvm_comparer
//
//! The class #uvm_comparer provides a policy object for doing comparisons. The
//! policies determine how miscompares are treated and counted. Results of a
//! comparison are stored in the comparer object. The uvm_object::compare
//! and uvm_object::do_compare methods are passed an uvm_comparer policy
//! object.
//------------------------------------------------------------------------------


class uvm_comparer
{
public:
  friend class uvm_default_coreservice_t;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Group: Comparison functions
  //--------------------------------------------------------------------------

  virtual bool compare_field( const std::string& name,
                              const uvm_bitstream_t& lhs,
                              const uvm_bitstream_t& rhs,
                              int size,
                              uvm_radix_enum radix = UVM_NORADIX ) const;

  virtual bool compare_field_int( const std::string& name,
                                  const uvm_integral_t& lhs,
                                  const uvm_integral_t& rhs,
                                  int size,
                                  uvm_radix_enum radix = UVM_NORADIX ) const;

  virtual bool compare_field_real( const std::string& name,
                                   double lhs,
                                   double rhs ) const;

  virtual bool compare_field_real( const std::string& name,
                                   float lhs,
                                   float rhs ) const;

  virtual bool compare_object( const std::string& name,
                               const uvm_object& lhs,
                               const uvm_object& rhs ) const;

  virtual bool compare_string( const std::string& name,
                               const std::string& lhs,
                               const std::string& rhs ) const;

  void print_msg( const std::string& msg ) const;

  //--------------------------------------------------------------------------
  // Group: Comparer settings
  //--------------------------------------------------------------------------

  void set_policy( uvm_recursion_policy_enum policy = UVM_DEFAULT_POLICY );
  uvm_recursion_policy_enum get_policy() const;

  void set_max_messages( unsigned int num = 1 );
  unsigned int get_max_messages() const;

  void set_verbosity( unsigned int verbosity = UVM_LOW );
  unsigned int get_verbosity() const;

  void set_severity( uvm_severity sev = UVM_INFO );
  uvm_severity get_severity () const;

  void set_miscompare_string( const std::string& miscompares = "" );
  std::string get_miscompare_string() const;

  void set_field_attribute( uvm_field_enum attr = UVM_PHYSICAL );
  uvm_field_enum get_field_attribute() const;

  void compare_type( bool enable = true );

  unsigned int get_result() const;


  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

 private:

  uvm_comparer(); // constructor

 public:

  virtual ~uvm_comparer(); // destructor

  void print_rollup( const uvm_object& rhs,
                     const uvm_object& lhs ) const;

  void print_msg_object( const uvm_object& lhs,
                         const uvm_object& rhs) const;

  //--------------------------------------
  // Public Data members according to LRM
  //--------------------------------------

 public:
  // TODO: We need to make all these data members private, since we introduced new public member
  // functions in UVM-SystemC to decouple access methods from data members,
  // see above in new group "Comparer settings"

  //--------------------------------------------------------------------------
  // Data member: policy
  //
  //! Determines whether comparison is UVM_DEEP, UVM_REFERENCE, or UVM_SHALLOW.
  //--------------------------------------------------------------------------

  uvm_recursion_policy_enum policy;

  //--------------------------------------------------------------------------
  // Data member: show_max
  //
  //! Sets the maximum number of messages to send to the messager for miscompares
  //! of an object.
  //--------------------------------------------------------------------------

  unsigned int show_max;

  //--------------------------------------------------------------------------
  // Data member: verbosity
  //
  //! Sets the verbosity for printed messages.
  //! The verbosity setting is used by the messaging mechanism to determine
  //! whether messages should be suppressed or shown.
  //--------------------------------------------------------------------------

  unsigned int verbosity;

  //--------------------------------------------------------------------------
  // Data member: sev
  //
  //! Sets the severity for printed messages.
  //! The severity setting is used by the messaging mechanism for printing and
  //! filtering messages.
  //--------------------------------------------------------------------------

  uvm_severity sev;

  //--------------------------------------------------------------------------
  // Data member: miscompares
  //
  //! This string is reset to an empty string when a comparison is started.
  //! The string holds the last set of miscompares that occurred during a
  //! comparison.
  //--------------------------------------------------------------------------

  mutable std::string miscompares;

  //--------------------------------------------------------------------------
  // Data member: physical
  //
  //! This variable provides a filtering mechanism for fields.
  //! The abstract and physical settings allow an object to distinguish between
  //! two different classes of fields.
  //! It is up to you, in the <uvm_object::do_compare> method, to test the
  //! setting of this field if you want to use the physical trait as a filter.
  //--------------------------------------------------------------------------

  bool physical;

  //--------------------------------------------------------------------------
  // data member: abstract
  //
  //! This variable provides a filtering mechanism for fields.
  //! The abstract and physical settings allow an object to distinguish between
  //! two different classes of fields.
  //! It is up to you, in the <uvm_object::do_compare> method, to test the
  //! setting of this field if you want to use the abstract trait as a filter.
  //--------------------------------------------------------------------------

  bool abstract;

  //--------------------------------------------------------------------------
  // data member: check_type
  //
  //! This variable determines whether the type, given by <uvm_object::get_type_name>,
  //! is used to verify that the types of two objects are the same.
  //!
  //! This variable is used by the <compare_object> method. In some cases it is useful
  //! to set this to false when the two operands are related by inheritance but are
  //! different types.
  //--------------------------------------------------------------------------

  bool check_type;

  //--------------------------------------------------------------------------
  // data member: result
  //
  //! This variable stores the number of miscompares for a given compare operation.
  //! You can use the result to determine the number of miscompares that
  //! were found.
  //--------------------------------------------------------------------------

  mutable unsigned int result;

 private:

  // TODO do we need these members?
  //  int depth;                 // current depth of objects
  //  uvm_copy_map* compare_map; // mapping of rhs to lhs objects
  //  uvm_scope_stack* scope;

}; // class comparer

} // namespace uvm

#endif /* UVM_COMPARER_H_ */
