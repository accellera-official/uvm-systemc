//----------------------------------------------------------------------
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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

#include <bitset>
#include <iostream>
#include <sstream>

#include "uvmsc/misc/uvm_status_container.h"
#include "uvmsc/policy/uvm_comparer.h"
#include "uvmsc/base/uvm_root.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/base/uvm_object.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/macros/uvm_message_defines.h"

//////////////

using namespace sc_core;
using namespace sc_dt;

namespace uvm {

//----------------------------------------------------------------------------
// Class: uvm_comparer implementation
//----------------------------------------------------------------------------

uvm_comparer::uvm_comparer()
{
  //uvm_recursion_policy_enum policy = UVM_DEFAULT_POLICY;
  show_max = true;
  verbosity = UVM_LOW;
  sev = UVM_INFO;
  miscompares = "";
  physical = true;
  abstract = false;
  check_type = true;
  result = 0;
}

//----------------------------------------------------------------------------
// member function: compare_field
//
//! Compares two integral values.
//! The \p name input is used for purposes of storing and printing a miscompare.
//! The left-hand-side \p lhs and right-hand-side \p rhs objects are the two
//! objects used for comparison.
//! The size variable indicates the number of bits to compare; size must be
//! less than or equal to 4096.
//! The radix is used for reporting purposes, the default radix is hex.
//----------------------------------------------------------------------------

bool uvm_comparer::compare_field( const std::string& name,
                                  const uvm_bitstream_t& lhs,
                                  const uvm_bitstream_t& rhs,
                                  int size,
                                  uvm_radix_enum radix ) const
{
  uvm_bitstream_t mask;
  std::ostringstream msg;

  if(size > 4096)
    uvm_report_info("COMP", "field size > 4096 not allowed", UVM_LOW);

  if(size <= 64)
    return compare_field_int(name, lhs, rhs, size, radix);

  mask = -1;
  mask >>= 4096-size; // was using UVM_STREAMBITS (=4096)

  sc_lv<4096> lhs_masked = lhs&mask;
  sc_lv<4096> rhs_masked = rhs&mask;

  if ((lhs_masked) != (rhs_masked))
  {
    uvm_object::get_status_container()->scope->set_arg(name);

    switch (radix)
    {
      case UVM_BIN:
      {
        msg << "lhs = 0b" << lhs_masked.to_string(sc_dt::SC_BIN_US, true)
            << " : rhs = 0b" << rhs_masked.to_string(sc_dt::SC_BIN_US, true);
        break;
      }
      case UVM_OCT:
      {
        msg << "lhs = 0o" << std::oct << lhs_masked.to_string(sc_dt::SC_OCT_US, true)
            << " : rhs = 0o" << std::oct << rhs_masked.to_string(sc_dt::SC_OCT_US, true);
        break;
      }
      case UVM_DEC:
      {
        msg << "lhs = " << lhs_masked.to_string(sc_dt::SC_DEC, true)
            << " : rhs = " << rhs_masked.to_string(sc_dt::SC_DEC, true);
        break;
      }
      case UVM_TIME: // TODO how to deal with time/sc_time?
      {
        msg << "lhs = " << lhs_masked.to_string(sc_dt::SC_DEC, true)
            << " : rhs = " << rhs_masked.to_string(sc_dt::SC_DEC, true);
        break;
      }
      case UVM_STRING: // TODO printed as binary ??
      {
        msg << "lhs = 0b" << lhs_masked.to_string(sc_dt::SC_BIN_US, true)
            << " : rhs = 0b" << rhs_masked.to_string(sc_dt::SC_BIN_US, true);
        break;
      }
      case UVM_ENUM: // printed in decimal
      {
        // Printed as decimal, user should use compare string for enum val
        msg << "lhs = " << lhs_masked.to_string(sc_dt::SC_DEC, true)
            << " : rhs = " << rhs_masked.to_string(sc_dt::SC_DEC, true);
        break;
      }
      default: // default is printed in hex
      {
        msg << "lhs = 0x" << lhs_masked.to_string(sc_dt::SC_HEX_US, true)
            << " : rhs = 0x" << rhs_masked.to_string(sc_dt::SC_HEX_US, true);
        break;
      }
    } // switch

    print_msg(msg.str());
    return false;
  }

  return true;
}


//----------------------------------------------------------------------------
// member function: compare_field_int
//
//! This method is the same as #compare_field except that the arguments are
//! small integers, less than or equal to 64 bits. It is automatically called
//! by #compare_field if the operand size is less than or equal to 64.
//----------------------------------------------------------------------------

bool uvm_comparer::compare_field_int( const std::string& name,
                                      const uvm_integral_t& lhs,
                                      const uvm_integral_t& rhs,
                                      int size,
                                      uvm_radix_enum radix ) const
{
  sc_bv<64> mask;
  std::ostringstream msg;

  mask = -1;
  mask >>= (64-size);

  sc_lv<64> lhs_masked = lhs & mask;
  sc_lv<64> rhs_masked = rhs & mask;

  if((lhs_masked) != (rhs_masked)) // TODO was !==
  {
    uvm_object::get_status_container()->scope->set_arg(name);

    switch(radix)
    {
      case UVM_BIN:
      {
        msg << "lhs = 0b" << lhs_masked.to_string(sc_dt::SC_BIN_US, true)
            << " : rhs = 0b" << rhs_masked.to_string(sc_dt::SC_BIN_US, true);
        break;
      }
      case UVM_OCT:
      {
        msg << "lhs = 0o" << lhs_masked.to_string(sc_dt::SC_OCT_US, true)
            << " : rhs = 0o" << rhs_masked.to_string(sc_dt::SC_OCT_US, true);
        break;
      }
      case UVM_DEC:
      {
        msg << "lhs = " << lhs_masked.to_string(sc_dt::SC_DEC, true)
            << " : rhs = " << rhs_masked.to_string(sc_dt::SC_DEC, true);
        break;
      }
      case UVM_TIME: // TODO how to deal with time/sc_time?
      {
        msg << "lhs = " << lhs_masked.to_string(sc_dt::SC_DEC, true)
            << " : rhs = " << rhs_masked.to_string(sc_dt::SC_DEC, true);
        break;
      }
      case UVM_STRING: // show as bit string
      {
        msg << "lhs = 0b" << lhs_masked.to_string(sc_dt::SC_BIN_US, true)
            << " : rhs = 0b" << rhs_masked.to_string(sc_dt::SC_BIN_US, true);
        break;
      }
      case UVM_ENUM:
      {
        // Printed as decimal, user should use compare string for enum val
        msg << "lhs = " << lhs_masked.to_string(sc_dt::SC_DEC, true)
            << " : rhs = " << rhs_masked.to_string(sc_dt::SC_DEC, true);
        break;
      }
      default: // default shown as hex
      {
        msg << "lhs = 0x" << lhs_masked.to_string(sc_dt::SC_HEX_US, true)
            << " : rhs = 0x" << rhs_masked.to_string(sc_dt::SC_HEX_US, true);
        break;
      }
    } // switch

    print_msg(msg.str());
    return false;
  } // if

  return true;
}


//----------------------------------------------------------------------------
// member function: compare_field_real
//
//! This method is the same as #compare_field except that the arguments are
//! real numbers.
//----------------------------------------------------------------------------

bool uvm_comparer::compare_field_real( const std::string& name,
                                       double lhs,
                                       double rhs ) const
{
  std::ostringstream msg;

  if(lhs != rhs)
  {
    uvm_object::get_status_container()->scope->set_arg(name);
    msg << "lhs = " << lhs << " : rhs = " << rhs;
    print_msg(msg.str());
    return false;
  }

  return true;
}

bool uvm_comparer::compare_field_real( const std::string& name,
                                       float lhs,
                                       float rhs ) const
{
  std::ostringstream msg;

  if(lhs != rhs)
  {
    uvm_object::get_status_container()->scope->set_arg(name);
    msg << "lhs = " << lhs << " : rhs = " << rhs;
    print_msg(msg.str());
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
// member function: compare_object
//
//! Compares two class objects using the #policy knob to determine whether the
//! comparison should be deep, shallow, or reference.
//! The name input is used for purposes of storing and printing a miscompare.
//! The \p lhs and \p rhs objects are the two objects used for comparison.
//! The #check_type determines whether or not to verify the object
//! types match (the return from lhs.get_type_name() matches
//! rhs.get_type_name() ).
//----------------------------------------------------------------------------

bool uvm_comparer::compare_object( const std::string& name,
                                   const uvm_object& lhs,
                                   const uvm_object& rhs) const
{
  bool result;

  if (lhs.compare(rhs))
    return true;

  if (policy == UVM_REFERENCE && !(lhs.compare(rhs)))
  {
    uvm_object::get_status_container()->scope->set_arg(name);
    print_msg_object(lhs, rhs);
    return false;
  }

  uvm_object::get_status_container()->scope->down(name);
  result = lhs.compare(rhs, this);
  uvm_object::get_status_container()->scope->up();

  return result;
}


//----------------------------------------------------------------------------
// member function: compare_string
//
//! Compares two string variables.
//! The \p name input is used for purposes of storing and printing a miscompare.
//! The \p lhs and \p rhs objects are the two objects used for comparison.
//----------------------------------------------------------------------------

bool uvm_comparer::compare_string( const std::string& name,
                                   const std::string& lhs,
                                   const std::string& rhs) const
{
  std::ostringstream msg;

  if(lhs != rhs)
  {
    uvm_object::get_status_container()->scope->set_arg(name);
    msg << "lhs = \"" << lhs << "\" : rhs = \"" << rhs + "\"";
    print_msg(msg.str());
    return false;
  }

  return true;
}


//----------------------------------------------------------------------------
// member function: print_msg
//
//! Causes the error count to be incremented and the message, \p msg, to be
//! applied to the #miscompares string (a newline is used to separate
//! messages).
//!
//! If the message count is less than the #show_max setting, then the message
//! is printed to standard-out using the current verbosity and severity
//! settings. See the #verbosity and #sev variables for more information.
//----------------------------------------------------------------------------

void uvm_comparer::print_msg( const std::string& msg ) const
{
  uvm_root* root;
  uvm_coreservice_t* cs;
  cs = uvm_coreservice_t::get();
  root = cs->get_root();

  result++;

  if(result <= show_max)
  {
     std::ostringstream str;
     str << "Miscompare for "
         << uvm_object::get_status_container()->scope->get()
         << ": "
         << msg;

     root->uvm_report(sev, "MISCMP", str.str(), UVM_LOW, UVM_FILE_M, UVM_LINE_M);
  }

  std::ostringstream str;
  str << miscompares
      << uvm_object::get_status_container()->scope->get()
      << ": "
      << msg;
  miscompares = str.str();

}

//----------------------------------------------------------------------------
// member function: set_policy
//
//! The member function set_policy shall set the comparison policy.
//! The following arguments are valid: UVM_DEEP, UVM_REFERENCE,
//! or UVM_SHALLOW. The default policy shall be set to UVM_DEFAULT_POLICY.
//----------------------------------------------------------------------------

void uvm_comparer::set_policy( uvm_recursion_policy_enum policy )
{}

//----------------------------------------------------------------------------
// member function: get_policy
//
//! The member function get_policy shall return the comparison policy.
//----------------------------------------------------------------------------

uvm_recursion_policy_enum uvm_comparer::get_policy() const
{
  return policy;
}

//----------------------------------------------------------------------------
// member function: set_max_messages
//
//! The member function set_max_messages sets the maximum number of messages
//! to send to the printer for miscompares of an object. The default number
//! of messages shall be set to one.
//----------------------------------------------------------------------------

void uvm_comparer::set_max_messages( unsigned int num )
{
  show_max = num;
}

//----------------------------------------------------------------------------
// member function: get_max_messages
//
//! The member function get_max_messages shall return the maximum number of
//! messages to send to the printer for miscompares of an object.
//----------------------------------------------------------------------------

unsigned int uvm_comparer::get_max_messages() const
{
  return show_max;
}

//----------------------------------------------------------------------------
// member function: set_verbosity
//
//! The member function set_verbosity shall set the verbosity for printed
//! messages. The verbosity setting is used by the messaging mechanism to
//! determine whether messages should be suppressed or shown. The default
//! verbosity shall be set to UVM_LOW.
//----------------------------------------------------------------------------

void uvm_comparer::set_verbosity( unsigned int _verbosity )
{
  verbosity = _verbosity;
}

//----------------------------------------------------------------------------
// member function: get_verbosity
//
//! The member function get_verbosity shall return the verbosity for
//! printed messages.
//----------------------------------------------------------------------------

unsigned int uvm_comparer::get_verbosity() const
{
  return verbosity;
}

//----------------------------------------------------------------------------
// member function: set_severity
//
//! The member function set_severity shall set the severity for printed
//! messages. The severity setting is used by the messaging mechanism for
//! printing and filtering messages. The default severity shall be set to
//! UVM_INFO.
//----------------------------------------------------------------------------

void uvm_comparer::set_severity( uvm_severity _sev )
{
  sev = _sev;
}

//----------------------------------------------------------------------------
// member function: get_severity
//
//! The member function get_severity shall return the severity for printed
//! messages.
//----------------------------------------------------------------------------

uvm_severity uvm_comparer::get_severity () const
{
  return sev;
}

//----------------------------------------------------------------------------
// member function: set_miscompare_string
//
//! The member function set_miscompare_string shall set the miscompare string.
//! This string is reset to an empty string when a comparison is started.
//! The string holds the last set of miscompares that occurred during a
//! comparison. The default miscompare string shall be empty.
//----------------------------------------------------------------------------

void uvm_comparer::set_miscompare_string( const std::string& _miscompares )
{
  miscompares = _miscompares;
}

//----------------------------------------------------------------------------
// member function: get_miscompare_string
//
//! The member function get_miscompare_string shall return the last set
//! of miscompares that occurred during a comparison.
//----------------------------------------------------------------------------

std::string uvm_comparer::get_miscompare_string() const
{
  return miscompares;
}

//----------------------------------------------------------------------------
// member function: set_field_attribute
//
//! The member function set_field_attribute shall set the field attribute
//! to UVM_PHYSICAL or UVM_ABSTRACT. The physical and abstract settings
//! allow an object to distinguish between these two different classes of
//! fields.
//! NOTE An application can use the callback uvm_object::do_compare to
//! check the field attribute if it wants to use it as a filter.
//----------------------------------------------------------------------------

void uvm_comparer::set_field_attribute( uvm_field_enum attr )
{
  if (attr == UVM_PHYSICAL) { physical = true; abstract = false; }
  if (attr == UVM_ABSTRACT) { abstract = true; physical = false; }
}

//----------------------------------------------------------------------------
// member function: get_field_attribute
//
//! The member function get_field_attribute shall return the field attribute
//! being UVM_PHYSICAL or UVM_ABSTRACT.
//----------------------------------------------------------------------------

uvm_field_enum uvm_comparer::get_field_attribute() const
{
  if (physical) return UVM_PHYSICAL;
  else return UVM_ABSTRACT;
}

//----------------------------------------------------------------------------
// member function: compare_type
//
//! The member function compare_type shall determine whether the type, given
//! by uvm_object::get_type_name, is used to verify that the types of two
//! objects are the same. If enabled, the member function compare_object
//! is called. By default, type checking shall be enabled.
//! NOTE: In some cases an application may disable type checking, when the
//! two operands are related by inheritance but are of different types.
//----------------------------------------------------------------------------

void uvm_comparer::compare_type( bool enable )
{
  if (enable) check_type = true;
  else check_type = false;
}

//----------------------------------------------------------------------------
// member function: get_result
//
//! The member function get_result shall return the number of miscompares
//! for a given compare operation. An application can use the result to
//! determine the number of miscompares that were found.
//----------------------------------------------------------------------------

unsigned int uvm_comparer::get_result() const
{
  return result;
}



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------

uvm_comparer::~uvm_comparer()
{}

//----------------------------------------------------------------------------
// member function: print_rollup
//
//! Implementation defined
//----------------------------------------------------------------------------

void uvm_comparer::print_rollup( const uvm_object& rhs,
                                 const uvm_object& lhs ) const
{
  std::ostringstream msg;

  uvm_root* root;
  uvm_coreservice_t* cs;
  cs = uvm_coreservice_t::get();
  root = cs->get_root();

  if(uvm_object::get_status_container()->scope->depth() == 0)
  {
    if(result && (show_max || (sev != UVM_INFO)))
    {
      if(show_max < result)
        msg << result
            << " miscompare(s) ("
            << show_max
            << " shown) for object ";
      else
        msg << result
            << " miscompare(s) for object ";

      std::ostringstream str;
      str << msg.str()
          << lhs.get_name()
          << "@"
          << lhs.get_inst_id()
          << " vs. "
          << rhs.get_name()
          << "@"
          << rhs.get_inst_id();

      root->uvm_report(sev, "MISCMP", str.str(), verbosity, UVM_FILE_M, UVM_LINE_M);
    }
  }
}


//----------------------------------------------------------------------------
// member function: print_msg_object
//
//! Implementation defined
//----------------------------------------------------------------------------

void uvm_comparer::print_msg_object( const uvm_object& lhs,
                                     const uvm_object& rhs ) const
{
  uvm_root* root;
  uvm_coreservice_t* cs;
  cs = uvm_coreservice_t::get();
  root = cs->get_root();

  result++;
  if(result <= show_max)
  {
    std::ostringstream msg;
    msg << "Miscompare for "
        << uvm_object::get_status_container()->scope->get()
        << ": lhs = @"
        << lhs.get_inst_id()
        << ": rhs = @"
        << rhs.get_inst_id();

    root->uvm_report(sev, "MISCMP", msg.str(), verbosity, UVM_FILE_M, UVM_LINE_M);
  }

  std::ostringstream str;
  str << miscompares
      << uvm_object::get_status_container()->scope->get()
      << ": lhs = @"
      << lhs.get_inst_id()
      << ": rhs = @"
      << rhs.get_inst_id();

  miscompares = str.str();
}


} // namespace uvm
