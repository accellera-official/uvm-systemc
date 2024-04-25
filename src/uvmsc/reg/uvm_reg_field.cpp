//----------------------------------------------------------------------
//   Copyright 2013-2020 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2014 Université Pierre et Marie Curie, Paris
//   Copyright 2018 Intel Corp.
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

#include <iomanip>

#include "uvmsc/reg/uvm_reg_field.h"
#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/reg/uvm_reg_item.h"
#include "uvmsc/reg/uvm_reg_map.h"
#include "uvmsc/reg/uvm_reg_adapter.h"
#include "uvmsc/reg/uvm_reg_block.h"
#include "uvmsc/reg/uvm_reg_cbs.h"

//////////////

namespace uvm {

//------------------------------------------------------------------------------
// static data member initialization
//------------------------------------------------------------------------------

bool uvm_reg_field::m_predefined = false;
bool uvm_reg_field::m_predefined_policies = uvm_reg_field::m_predefine_policies();
unsigned int uvm_reg_field::m_max_size = 0;

//----------------------------------------------------------------------
// Constructor
//
//! Create a new field instance
//!
//! This method should not be used directly.
//! The uvm_reg_field::type_id::create() factory method
//! should be used instead.
//----------------------------------------------------------------------

uvm_reg_field::uvm_reg_field( const std::string& name ) : uvm_object(name)
{
  m_parent = NULL;
  m_mirrored = 0;
  m_desired = 0;
  m_access = "";
  m_lsb = 0;
  m_size = 0;
  m_volatile = false;
  m_reset.clear();
  m_written = false;
  m_read_in_progress = false;
  m_write_in_progress = false;
  std::string m_fname = "";
  m_lineno = 0;
  m_cover_on = 0;
  m_individually_accessible = false;
  m_check = UVM_NO_CHECK;
}

//----------------------------------------------------------------------
// member function: configure
//
//! Instance-specific configuration
//!
//! Specify the \p parent register of this field, its
//! \p size in bits, the position of its least-significant bit
//! within the register relative to the least-significant bit
//! of the register, its \p access policy, volatility,
//! "HARD" \p reset value,
//! whether the field value is actually reset
//! (the \p reset value is ignored if false),
//! whether the field value may be randomized and
//! whether the field is the only one to occupy a byte lane in the register.
//!
//! See #set_access for a specification of the pre-defined
//! field access policies.
//!
//! If the field access policy is a pre-defined policy and NOT one of
//! "RW", "WRC", "WRS", "WO", "W1", or "WO1",
//! the value of ~is_rand~ is ignored and the rand_mode() for the
//! field instance is turned off since it cannot be written.
//----------------------------------------------------------------------

void uvm_reg_field::configure( uvm_reg* parent,
                               unsigned int size,
                               unsigned int lsb_pos,
                               const std::string& access,
                               bool is_volatile, // renamed icw UVM-SV
                               uvm_reg_data_t reset,
                               bool has_reset,
                               bool is_rand,
                               bool individually_accessible )
{
  m_parent = parent;
  if(size == 0)
  {
    UVM_ERROR("RegModel",
      "Field '" +  get_full_name() + "' cannot have 0 bits");
    size = 1;
  }

  m_size      = size;
  m_volatile  = is_volatile;
  m_access    = uvm_toupper(access);
  m_lsb       = lsb_pos;
  m_cover_on  = UVM_NO_COVERAGE;
  m_written   = false;
  m_check     = is_volatile ? UVM_NO_CHECK : UVM_CHECK;
  m_individually_accessible = individually_accessible;

  if(has_reset)
    set_reset(reset);
  /*  TODO add support in resource container for const& values
  else
    uvm_resource_db<bool>::set("REG::"+get_full_name(),
      "NO_REG_HW_RESET_TEST", true);
  */

  m_parent->add_field(this);

  if(m_policy_names().find(m_access) == m_policy_names().end()) // not exists
  {
    UVM_ERROR("RegModel", "Access policy '" + access +
        "' for field '" + get_full_name() + "' is not defined. Setting to RW");
        m_access = "RW";
  }

  if (size > m_max_size)
    m_max_size = size;

  // Ignore is_rand if the field is known not to be writeable
  // i.e. not "RW", "WRC", "WRS", "WO", "W1", "WO1"
  if( (access == "RO") || (access == "RC") || (access == "RS") ||
      (access == "WC") || (access == "WS") || (access == "W1C") ||
      (access == "W1S") || (access == "W1T") || (access == "W0C") ||
      (access == "W0S") || (access == "W0T") || (access == "W1SRC") ||
      (access == "W1CRS") || (access == "W0SRC") || (access == "W0CRS") ||
      (access == "WSRC") || (access == "WCRS") || (access == "WOC") ||
      (access == "WOS") )
    is_rand = false;

   /* TODO randomization
   if (!is_rand)
     m_value.rand_mode(0);
   */
}

//----------------------------------------------------------------------
// Group: Introspection
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: get_name
//
//! Get the simple name
//!
//! Return the simple object name of this field
//----------------------------------------------------------------------

// Inherit from base class

//----------------------------------------------------------------------
// member function: get_full_name
//
//! Get the hierarchical name
//!
//! Return the hierarchical name of this field
//! The base of the hierarchical name is the root block.
//----------------------------------------------------------------------

const std::string uvm_reg_field::get_full_name() const
{
  return m_parent->get_full_name() + "." + get_name();
}

//----------------------------------------------------------------------
// member function: get_parent
//
//! Get the parent register
//----------------------------------------------------------------------

uvm_reg* uvm_reg_field::get_parent() const
{
  return m_parent;
}


//----------------------------------------------------------------------
// member function: get_lsb_pos
//
//! Return the position of the field
//!
//! Returns the index of the least significant bit of the field
//! in the register that instantiates it.
//! An offset of 0 indicates a field that is aligned with the
//! least-significant bit of the register.
//----------------------------------------------------------------------

unsigned int uvm_reg_field::get_lsb_pos() const
{
  return m_lsb;
}

//----------------------------------------------------------------------
// member function: get_n_bits
//
//! Returns the width, in number of bits, of the field.
//----------------------------------------------------------------------

unsigned int uvm_reg_field::get_n_bits() const
{
  return m_size;
}

//----------------------------------------------------------------------
// member function: get_max_size
//
//! Returns the width, in number of bits, of the largest field.
//----------------------------------------------------------------------

unsigned int uvm_reg_field::get_max_size()
{
  return m_max_size;
}

//----------------------------------------------------------------------
// member function: set_access
//
//! Modify the access policy of the field
//!
//! Modify the access policy of the field to the specified one and
//! return the previous access policy.
//!
//! The pre-defined access policies are as follows.
//! The effect of a read operation are applied after the current
//! value of the field is sampled.
//! The read operation will return the current value,
//! not the value affected by the read operation (if any).
//!
//! "RO"    - W: no effect, R: no effect
//! "RW"    - W: as-is, R: no effect
//! "RC"    - W: no effect, R: clears all bits
//! "RS"    - W: no effect, R: sets all bits
//! "WRC"   - W: as-is, R: clears all bits
//! "WRS"   - W: as-is, R: sets all bits
//! "WC"    - W: clears all bits, R: no effect
//! "WS"    - W: sets all bits, R: no effect
//! "WSRC"  - W: sets all bits, R: clears all bits
//! "WCRS"  - W: clears all bits, R: sets all bits
//! "W1C"   - W: 1/0 clears/no effect on matching bit, R: no effect
//! "W1S"   - W: 1/0 sets/no effect on matching bit, R: no effect
//! "W1T"   - W: 1/0 toggles/no effect on matching bit, R: no effect
//! "W0C"   - W: 1/0 no effect on/clears matching bit, R: no effect
//! "W0S"   - W: 1/0 no effect on/sets matching bit, R: no effect
//! "W0T"   - W: 1/0 no effect on/toggles matching bit, R: no effect
//! "W1SRC" - W: 1/0 sets/no effect on matching bit, R: clears all bits
//! "W1CRS" - W: 1/0 clears/no effect on matching bit, R: sets all bits
//! "W0SRC" - W: 1/0 no effect on/sets matching bit, R: clears all bits
//! "W0CRS" - W: 1/0 no effect on/clears matching bit, R: sets all bits
//! "WO"    - W: as-is, R: error
//! "WOC"   - W: clears all bits, R: error
//! "WOS"   - W: sets all bits, R: error
//! "W1"    - W: first one after ~HARD~ reset is as-is, other W have no effects, R: no effect
//! "WO1"   - W: first one after ~HARD~ reset is as-is, other W have no effects, R: error
//!
//! It is important to remember that modifying the access of a field
//! will make the register model diverge from the specification
//! that was used to create it.
//----------------------------------------------------------------------

std::string uvm_reg_field::set_access( const std::string& mode )
{
  std::string access = m_access;

  m_access = uvm_toupper(mode);

  if (m_policy_names().find(m_access) == m_policy_names().end()) // not exists
  {
      UVM_ERROR("RegModel", "Access policy '" + m_access +
          "' is not a defined field access policy");
      m_access = access;
  }
  return access;
}

//----------------------------------------------------------------------
// member function: define_access (static)
//
//! Define a new access policy value
//!
//! Because field access policies are specified using string values,
//! there is no way for SystemVerilog to verify if a specific access
//! value is valid or not.
//! To help catch typing errors, user-defined access values
//! must be defined using this method to avoid being reported as an
//! invalid access policy.
//!
//! The name of field access policies are always converted to all uppercase.
//!
//! Returns TRUE if the new access policy was not previously
//! defined.
//! Returns FALSE otherwise but does not issue an error message.
//----------------------------------------------------------------------

bool uvm_reg_field::define_access( std::string name )
{
  if (!m_predefined)
    m_predefined = m_predefine_policies();

  name = uvm_toupper(name);

  if (m_policy_names().find(name) != m_policy_names().end()) // exists
    return false;

  m_policy_names()[name] = true;
  return true;
}


//----------------------------------------------------------------------
// member function: get_access
//
//! Get the access policy of the field
//!
//! Returns the current access policy of the field
//! when written and read through the specified address \p map.
//! If the register containing the field is mapped in multiple
//! address map, an address map must be specified.
//! The access policy of a field from a specific
//! address map may be restricted by the register's access policy in that
//! address map.
//! For example, a RW field may only be writable through one of
//! the address maps and read-only through all of the other maps.
//----------------------------------------------------------------------

std::string uvm_reg_field::get_access( uvm_reg_map* map ) const
{
  std::string _access = m_access;
  std::string _rights = m_parent->get_rights(map);

  if( map == uvm_reg_map::backdoor() )
    return _access;

  // Is the register restricted in this map?
  if (_rights == "RW")   // No restrictions
    return _access;

  if (_rights == "RO")
  {
    if ( (_access == "RW") || (_access == "RO") ||
         (_access == "WC") || (_access == "WS") ||
         (_access == "W1C") || (_access == "W1S") ||
         (_access == "W1T") || (_access == "W0C") ||
         (_access == "W0S") || (_access == "W0T") ||
         (_access == "W1") )
      _access = "RO";

    if ( (_access == "RC") || (_access == "WRC") ||
         (_access == "W1SRC") || (_access == "W0SRC") ||
         (_access == "WSRC") )
      _access = "RC";

    if ( (_access == "RS") || (_access == "WRS") ||
         (_access == "W1CRS") || (_access == "W0CRS") ||
         (_access == "WCRS") )
      _access = "RS";

    if ( (_access == "WO") || (_access == "WOC") ||
         (_access == "WOS") || (_access == "WO1") )
    {
      UVM_ERROR("RegModel",
        _access + " field '" + get_name() + "' restricted to RO in map '"
        + map->get_full_name() + "'");
    }
  } // if "RO"
  else
  if (_rights == "WO")
  {
    if ( (_access == "RW") || (_access == "WO") )
      _access = "WO";
    else       // No change f|| the other modes
    {
      UVM_ERROR("RegModel",
        _access + " field '" + get_full_name() + "' restricted to WO in map '"
        + map->get_full_name() + "'");
    }
  } // if "WO"
  else // default to error
    UVM_ERROR("RegModel",
      "Register '" + m_parent->get_full_name() +
      "' containing field '" + get_name() + "' is mapped in map '" +
      map->get_full_name() + "' with unknown access right '" +
      m_parent->get_rights(map) + "'");

  return _access;
}

//----------------------------------------------------------------------
// member function: is_known_access
//
//! Check if access policy is a built-in one.
//!
//! Returns TRUE if the current access policy of the field,
//! when written and read through the specified address \p map,
//! is a built-in access policy.
//----------------------------------------------------------------------

bool uvm_reg_field::is_known_access( uvm_reg_map* map ) const
{
  std::string acc = get_access(map);

  if( (acc == "RO") || (acc == "RW") || (acc == "RC") ||
      (acc == "RS") || (acc == "WC") || (acc == "WS") ||
      (acc == "W1C") || (acc == "W1S") || (acc == "W1T") ||
      (acc == "W0C") || (acc == "W0S") || (acc == "W0T") ||
      (acc == "WRC") || (acc == "WRS") || (acc == "W1SRC") ||
      (acc == "W1CRS") || (acc == "W0SRC") || (acc == "W0CRS") ||
      (acc == "WSRC") || (acc == "WCRS") || (acc == "WO") || (acc == "WOC") ||
      (acc == "WOS") || (acc == "W1") || (acc == "WO1" ) )
    return true;

  return false;
}


//----------------------------------------------------------------------
// member function: set_volatility
//
//! Modify the volatility of the field to the specified one.
//!
//! It is important to remember that modifying the volatility of a field
//! will make the register model diverge from the specification
//! that was used to create it.
//----------------------------------------------------------------------

void uvm_reg_field::set_volatility( bool is_volatile )
{
  m_volatile = is_volatile;
}

//----------------------------------------------------------------------
// member function: is_volatile
//
//! Indicates if the field value is volatile
//!
//! UVM uses the IEEE 1685-2009 IP-XACT definition of "volatility".
//! If TRUE, the value of the register is not predictable because it
//! may change between consecutive accesses.
//! This typically indicates a field whose value is updated by the DUT.
//! The nature or cause of the change is not specified.
//! If FALSE, the value of the register is not modified between
//! consecutive accesses.
//----------------------------------------------------------------------

bool uvm_reg_field::is_volatile() const
{
  return m_volatile;
}

//----------------------------------------------------------------------
// Group: Access
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// member function: set
//
//! Set the desired value for this field
//!
//! It sets the desired value of the field to the specified \p value
//! modified by the field access policy.
//! It does not actually set the value of the field in the design,
//! only the desired value in the abstraction class.
//! Use the uvm_reg::update() method to update the actual register
//! with the desired value or the <uvm_reg_field::write()> method
//! to actually write the field and update its mirrored value.
//!
//! The final desired value in the mirror is a function of the field access
//! policy and the set value, just like a normal physical write operation
//! to the corresponding bits in the hardware.
//! As such, this method (when eventually followed by a call to
//! uvm_reg::update() )
//! is a zero-time functional replacement for the uvm_reg_field::write()
//! method.
//! For example, the desired value of a read-only field is not modified
//! by this method and the desired value of a write-once field can only
//! be set if the field has not yet been
//! written to using a physical (for example, front-door) write operation.
//!
//! Use the uvm_reg_field::predict() to modify the mirrored value of
//! the field.
//----------------------------------------------------------------------

void uvm_reg_field::set( uvm_reg_data_t value,
                         const std::string& fname,
                         int lineno )
{
  uvm_reg_data_t mask = (1 << m_size)-1;

  m_fname = fname;
  m_lineno = lineno;

  if (value >> m_size)
  {
    std::ostringstream str;
    str << "Specified value (0x"
        << value
        << ") greater than field '"
        << get_name()
        << "' size ("
        << m_size
        << " bits)";
    UVM_WARNING("RegModel", str.str() );
    value &= mask;
  }

  if (m_parent->is_busy())
  {
    UVM_WARNING("UVM/FLD/SET/BSY",
        "Setting the value of field '" + get_name() +
        " while containing register '" + m_parent->get_full_name() +
        "' is being accessed may result in loss of desired field value. A race condition between threads concurrently accessing the register model is the likely cause of the problem.");
  }

  //if( m_access == "RO" )    m_desired = m_desired; // TODO original UVM-SV code! remove?
  if( m_access == "RW" )    m_desired = value;
  //if( m_access == "RC" )    m_desired = m_desired; // TODO original UVM-SV code! remove?
  //if( m_access == "RS" )    m_desired = m_desired; // TODO original UVM-SV code! remove?
  if( m_access == "WC" )    m_desired = 0;
  if( m_access == "WS" )    m_desired = mask;
  if( m_access == "WRC" )   m_desired = value;
  if( m_access == "WRS" )   m_desired = value;
  if( m_access == "WSRC" )  m_desired = mask;
  if( m_access == "WCRS" )  m_desired = 0;
  if( m_access == "W1C" )   m_desired = m_desired & (~value);
  if( m_access == "W1S" )   m_desired = m_desired | value;
  if( m_access == "W1T" )   m_desired = m_desired ^ value;
  if( m_access == "W0C" )   m_desired = m_desired & value;
  if( m_access == "W0S" )   m_desired = m_desired | (~value & mask);
  if( m_access == "W0T" )   m_desired = m_desired ^ (~value & mask);
  if( m_access == "W1SRC" ) m_desired = m_desired | value;
  if( m_access == "W1CRS" ) m_desired = m_desired & (~value);
  if( m_access == "W0SRC" ) m_desired = m_desired | (~value & mask);
  if( m_access == "W0CRS" ) m_desired = m_desired & value;
  if( m_access == "WO" )    m_desired = value;
  if( m_access == "WOC" )   m_desired = 0;
  if( m_access == "WOS" )   m_desired = mask;
  if( m_access == "W1" )    m_desired = (m_written) ? m_desired : value;
  if( m_access == "WO1" )   m_desired = (m_written) ? m_desired : value;

  // default:
  m_desired = value;

  m_value = m_desired;
}

//----------------------------------------------------------------------
// member function: get
//
//! Return the desired value of the field
//!
//! It does not actually read the value
//! of the field in the design, only the desired value
//! in the abstraction class. Unless set to a different value
//! using the uvm_reg_field::set(), the desired value
//! and the mirrored value are identical.
//!
//! Use the uvm_reg_field::read() or uvm_reg_field::peek()
//! method to get the actual field value.
//!
//! If the field is write-only, the desired/mirrored
//! value is the value last written and assumed
//! to reside in the bits implementing it.
//! Although a physical read operation would something different,
//! the returned value is the actual content.
//----------------------------------------------------------------------

uvm_reg_data_t uvm_reg_field::get( const std::string& fname,
                                   int lineno ) const
{
   m_fname = fname;
   m_lineno = lineno;
   return m_desired;
}

//----------------------------------------------------------------------
// member function: get_mirrored_value
//
//! Return the mirrored value of the field
//!
//! It does not actually read the value of the field in the design, only the
//! mirrored value in the abstraction class.
//!
//! If the field is write-only, the desired/mirrored
//! value is the value last written and assumed
//! to reside in the bits implementing it.
//! Although a physical read operation would something different,
//! the returned value is the actual content.
//----------------------------------------------------------------------

uvm_reg_data_t uvm_reg_field::get_mirrored_value( const std::string& fname,
                                                  int lineno ) const
{
  m_fname = fname;
  m_lineno = lineno;
  return m_mirrored;
}
//----------------------------------------------------------------------
// member function: reset
//
//! Reset the desired/mirrored value for this field.
//!
//! It sets the desired and mirror value of the field
//! to the reset event specified by \p kind.
//! If the field does not have a reset value specified for the
//! specified reset ~kind~ the field is unchanged.
//!
//! It does not actually reset the value of the field in the design,
//! only the value mirrored in the field abstraction class.
//!
//! Write-once fields can be modified after
//! a "HARD" reset operation.
//----------------------------------------------------------------------

void uvm_reg_field::reset( const std::string& kind )
{
  if (m_reset.find(kind) == m_reset.end()) // not exists
    return;

  m_mirrored = m_reset[kind];
  m_desired  = m_mirrored;
  m_value    = m_mirrored;

  if (kind == "HARD")
    m_written  = false;
}

//----------------------------------------------------------------------
// member function: get_reset
//
//! Get the specified reset value for this field
//!
//! Return the reset value for this field
//! for the specified reset \p kind.
//! Returns the current field value is no reset value has been
//! specified for the specified reset event.
//----------------------------------------------------------------------

uvm_reg_data_t uvm_reg_field::get_reset( const std::string& kind ) const
{
  if (m_reset.find(kind) == m_reset.end()) // not exists
    return m_desired;

  return m_reset.find(kind)->second;
}

//----------------------------------------------------------------------
// member function: has_reset
//
//! Check if the field has a reset value specified
//!
//! Return TRUE if this field has a reset value specified
//! for the specified reset \p kind.
//! If \p delete is true, removes the reset value, if any.
//----------------------------------------------------------------------

bool uvm_reg_field::has_reset( const std::string& kind,
                               bool do_delete )
{
  if (m_reset.find(kind) == m_reset.end()) // not exists
     return false;

  if (do_delete)
    m_reset.erase(kind);

  return true;
}

//----------------------------------------------------------------------
// member function: set_reset
//
//! Specify or modify the reset value for this field
//!
//! Specify or modify the reset value for this field corresponding
//! to the cause specified by \p kind.
//----------------------------------------------------------------------

void uvm_reg_field::set_reset( uvm_reg_data_t value,
                               const std::string& kind )
{
  m_reset[kind] = value & ((1ul<<m_size) - 1);
}

//----------------------------------------------------------------------
// member function: needs_update
//
//! Check if the abstract model contains different desired and mirrored values.
//!
//! If a desired field value has been modified in the abstraction class
//! without actually updating the field in the DUT,
//! the state of the DUT (more specifically what the abstraction class
//! thinks the state of the DUT is) is outdated.
//! This method returns TRUE
//! if the state of the field in the DUT needs to be updated
//! to match the desired value.
//! The mirror values or actual content of DUT field are not modified.
//! Use the uvm_reg::update() to actually update the DUT field.
//----------------------------------------------------------------------

bool uvm_reg_field::needs_update() const
{
  return (m_mirrored != m_desired);
}

//----------------------------------------------------------------------
// member function: write
//
//! Write the specified value in this field
//!
//! Write \p value in the DUT field that corresponds to this
//! abstraction class instance using the specified access
//! \p path.
//! If the register containing this field is mapped in more
//!  than one address map,
//! an address \p map must be
//! specified if a physical access is used (front-door access).
//! If a back-door access path is used, the effect of writing
//! the field through a physical access is mimicked. For
//! example, read-only bits in the field will not be written.
//!
//! The mirrored value will be updated using the uvm_reg_field::predict()
//! method.
//!
//! If a front-door access is used, and
//! if the field is the only field in a byte lane and
//! if the physical interface corresponding to the address map used
//! to access the field support byte-enabling,
//! then only the field is written.
//! Otherwise, the entire register containing the field is written,
//! and the mirrored values of the other fields in the same register
//! are used in a best-effort not to modify their value.
//!
//! If a backdoor access is used, a peek-modify-poke process is used.
//! in a best-effort not to modify the value of the other fields in the
//! register.
//----------------------------------------------------------------------

void uvm_reg_field::write( uvm_status_e& status,
                           uvm_reg_data_t value,
                           uvm_path_e path,
                           uvm_reg_map* map,
                           uvm_sequence_base* parent,
                           int prior,
                           uvm_object* extension,
                           const std::string& fname,
                           int lineno )
{
   uvm_reg_item* rw;
   rw = uvm_reg_item::type_id::create("field_write_item", NULL, get_full_name());
   rw->element      = this;
   rw->element_kind = UVM_FIELD;
   rw->access_kind  = UVM_WRITE;
   rw->value[0]     = value;
   rw->path         = path;
   rw->map          = map;
   rw->parent       = parent;
   rw->prior        = prior;
   rw->extension    = extension;
   rw->fname        = fname;
   rw->lineno       = lineno;

   do_write(rw);

   status = rw->status;

   uvm_reg_item::type_id::destroy(rw);
}

//----------------------------------------------------------------------
// member function: read
//
//! Read the current value from this field
//!
//! Read and return \p value from the DUT field that corresponds to this
//! abstraction class instance using the specified access
//! \p path.
//! If the register containing this field is mapped in more
//! than one address map, an address \p map must be
//! specified if a physical access is used (front-door access).
//! If a back-door access path is used, the effect of reading
//! the field through a physical access is mimicked. For
//! example, clear-on-read bits in the filed will be set to zero.
//!
//! The mirrored value will be updated using the uvm_reg_field::predict()
//! method.
//!
//! If a front-door access is used, and
//! if the field is the only field in a byte lane and
//! if the physical interface corresponding to the address map used
//! to access the field support byte-enabling,
//! then only the field is read.
//! Otherwise, the entire register containing the field is read,
//! and the mirrored values of the other fields in the same register
//! are updated.
//!
//! If a backdoor access is used, the entire containing register is peeked
//! and the mirrored value of the other fields in the register is updated.
//----------------------------------------------------------------------

void uvm_reg_field::read( uvm_status_e& status, // output
                          uvm_reg_data_t& value, // output
                          uvm_path_e path,
                          uvm_reg_map* map,
                          uvm_sequence_base* parent,
                          int prior,
                          uvm_object* extension,
                          const std::string& fname,
                          int lineno )
{
  uvm_reg_item* rw;
  rw = uvm_reg_item::type_id::create("field_read_item", NULL, get_full_name());
  rw->element      = this;
  rw->element_kind = UVM_FIELD;
  rw->access_kind  = UVM_READ;
  rw->value[0]     = 0;
  rw->path         = path;
  rw->map          = map;
  rw->parent       = parent;
  rw->prior        = prior;
  rw->extension    = extension;
  rw->fname        = fname;
  rw->lineno       = lineno;

  do_read(rw);

  value = rw->value[0];
  status = rw->status;
  
  uvm_reg_item::type_id::destroy(rw);
}


//----------------------------------------------------------------------
// member function: poke
//
//! Deposit the specified value in this field
//!
//! Deposit the value in the DUT field corresponding to this
//! abstraction class instance, as-is, using a back-door access.
//! A peek-modify-poke process is used
//! in a best-effort not to modify the value of the other fields in the
//! register.
//!
//! The mirrored value will be updated using the uvm_reg_field::predict()
//! method.
//----------------------------------------------------------------------

void uvm_reg_field::poke( uvm_status_e& status, // output
                          uvm_reg_data_t value,
                          const std::string& kind,
                          uvm_sequence_base* parent,
                          uvm_object* extension,
                          const std::string& fname,
                          int lineno )
{
  uvm_reg_data_t tmp;

  m_fname = fname;
  m_lineno = lineno;

  if (value >> m_size)
  {
    UVM_WARNING("RegModel",
        "uvm_reg_field::poke(): Value exceeds size of field '" +
         get_name() + "'");
    value &= value & ((1<<m_size)-1);
  }

  m_parent->m_atomic_check_lock(1);
  m_parent->m_is_locked_by_field = true;

  tmp = 0;

  // What is the current values of the other fields???
  m_parent->peek(status, tmp, kind, parent, extension, fname, lineno);

  if (status == UVM_NOT_OK)
  {
    std::ostringstream str;
    str << "uvm_reg_field::poke(): Peek of register '"
        << m_parent->get_full_name()
        << "' returned status "
        << uvm_status_name[status];
    UVM_ERROR("RegModel",str.str());

    m_parent->m_atomic_check_lock(0);
    m_parent->m_is_locked_by_field = false;
      return;
  }

  // Force the value for this field then poke the resulting value
  tmp &= ~(((1<<m_size)-1) << m_lsb);
  tmp |= value << m_lsb;
  m_parent->poke(status, tmp, kind, parent, extension, fname, lineno);

  m_parent->m_atomic_check_lock(0);
  m_parent->m_is_locked_by_field = false;
}

//----------------------------------------------------------------------
// member function: peek
//
//! Read the current value from this field
//!
//! Sample the value in the DUT field corresponding to this
//! abstraction class instance using a back-door access.
//! The field value is sampled, not modified.
//!
//! Uses the HDL path for the design abstraction specified by \p kind.
//!
//! The entire containing register is peeked
//! and the mirrored value of the other fields in the register
//! are updated using the uvm_reg_field::predict() method.
//----------------------------------------------------------------------

void uvm_reg_field::peek( uvm_status_e& status, // output
                          uvm_reg_data_t& value, // output
                          const std::string& kind,
                          uvm_sequence_base* parent,
                          uvm_object* extension,
                          const std::string& fname,
                          int lineno )
{
   uvm_reg_data_t reg_value = 0;

   m_fname = fname;
   m_lineno = lineno;

   m_parent->peek(status, reg_value, kind, parent, extension, fname, lineno);
   value = (reg_value >> m_lsb) & ((1<<m_size)-1); // TODO check, moved parenthesis
}

//----------------------------------------------------------------------
// member function: mirror
//
//! Read the field and update/check its mirror value
//!
//! Read the field and optionally compared the readback value
//! with the current mirrored value if \p check is UVM_CHECK.
//! The mirrored value will be updated using the predict()
//! method based on the readback value.
//!
//! The \p path argument specifies whether to mirror using
//! the UVM_FRONTDOOR> (read) or
//! or UVM_BACKDOOR> (peek()).
//!
//! If \p check is specified as UVM_CHECK,
//! an error message is issued if the current mirrored value
//! does not match the readback value, unless #set_compare was used
//! disable the check.
//!
//! If the containing register is mapped in multiple address maps and physical
//! access is used (front-door access), an address \p map must be specified.
//! For write-only fields, their content is mirrored and optionally
//! checked only if a UVM_BACKDOOR
//! access path is used to read the field.
//----------------------------------------------------------------------

void uvm_reg_field::mirror( uvm_status_e& status, // output
                            uvm_check_e check,
                            uvm_path_e path,
                            uvm_reg_map* map,
                            uvm_sequence_base* parent,
                            int prior,
                            uvm_object* extension,
                            const std::string& fname,
                            int lineno )
{
   m_fname = fname;
   m_lineno = lineno;
   m_parent->mirror( status, check, path, map, parent, prior, extension,
                     fname, lineno);
}

//----------------------------------------------------------------------
// member function: set_compare
//
//! Sets the compare policy during a mirror update.
//! The field value is checked against its mirror only when both the
//! \p check argument in uvm_reg_block::mirror, uvm_reg::mirror,
//! or uvm_reg_field::mirror and the compare policy for the
//! field is UVM_CHECK.
//----------------------------------------------------------------------

void uvm_reg_field::set_compare( uvm_check_e check )
{
  m_check = check;
}

//----------------------------------------------------------------------
// member function: get_compare
//
//! Returns the compare policy for this field.
//----------------------------------------------------------------------

uvm_check_e uvm_reg_field::get_compare() const
{
  return m_check;
}


//----------------------------------------------------------------------
// member function: is_indv_accessible
//
//! Check if this field can be written individually, i.e. without
//! affecting other fields in the containing register.
//----------------------------------------------------------------------

bool uvm_reg_field::is_indv_accessible( uvm_path_e path,
                                        uvm_reg_map* local_map ) const
{
  if (path == UVM_BACKDOOR)
  {
    UVM_WARNING("RegModel",
        "Individual BACKDOOR field access not available for field '" +
        get_full_name() + "'. Accessing complete register instead.");
        return false;
  }

  if (!m_individually_accessible)
  {
    UVM_WARNING("RegModel",
        "Individual field access not available for field '" +
        get_full_name() + "'. Accessing complete register instead.");
        return false;
  }

  // Cannot access individual fields if the container register
  // has a user-defined front-door
  if (m_parent->get_frontdoor(local_map) != NULL)
  {
    UVM_WARNING("RegModel",
        "Individual field access not available for field '" +
        get_name() + "' because register '" +
        m_parent->get_full_name() +
        "' has a user-defined front-door. Accessing complete register instead.");
        return false;
  }

  uvm_reg_map* system_map = local_map->get_root_map();
  uvm_reg_adapter* adapter = system_map->get_adapter();

  if (adapter->supports_byte_enable)
    return true;

  int bus_width = local_map->get_n_bytes();
  std::vector<uvm_reg_field *>fields;

  m_parent->get_fields(fields);

  if (fields.size() != 1)
  {
    unsigned int fld_idx = 0;

    int bus_sz = bus_width*8;

    for( unsigned int i = 0; i < fields.size(); i++ )
    {
      if (fields[i] == this)
      {
        fld_idx = i;
        break;
      }
    }

    int this_lsb = fields[fld_idx]->get_lsb_pos();
    int prev_lsb = this_lsb;
    int next_lsb = this_lsb;
    int this_sz  = fields[fld_idx]->get_n_bits();
    int prev_sz = this_sz;

    if (fld_idx > 0)
    {
      prev_lsb = fields[fld_idx-1]->get_lsb_pos();
      prev_sz  = fields[fld_idx-1]->get_n_bits();
    }

    if (fld_idx < fields.size()-1)
      next_lsb = fields[fld_idx+1]->get_lsb_pos();

    // if first field in register
    if (fld_idx == 0 &&
        ((next_lsb % bus_sz) == 0 ||
            (next_lsb - this_sz) > (next_lsb % bus_sz)))
      return true;

    // if last field in register
    else
      if (fld_idx == (fields.size()-1) &&
          ((this_lsb % bus_sz) == 0 ||
              (this_lsb - (prev_lsb + prev_sz)) >= (this_lsb % bus_sz)))
        return true;

      // if somewhere in between
      else
      {
        if((this_lsb % bus_sz) == 0)
        {
          if((next_lsb % bus_sz) == 0 ||
              (next_lsb - (this_lsb + this_sz)) >= (next_lsb % bus_sz))
            return true;
        }
        else
        {
          if( (next_lsb - (this_lsb + this_sz)) >= (next_lsb % bus_sz) &&
              ((this_lsb - (prev_lsb + prev_sz)) >= (this_lsb % bus_sz)) )
            return true;
        }
      } // else this_lsb % bus_sz
  } // else field-size


  UVM_WARNING("RegModel",
    "Target bus does not support byte enabling, and the field '" +
    get_full_name() + "' is not the only field within the entire bus width. " +
    "Individual field access will not be available. " +
    "Accessing complete register instead.");

  return false;
}

//----------------------------------------------------------------------
// member function: predict
//
//! Update the mirrored value for this field.
//!
//! Predict the mirror value of the field based on the specified
//! observed \p value on a bus using the specified address \p map.
//!
//! If \p kind is specified as UVM_PREDICT_READ, the value
//! was observed in a read transaction on the specified address \p map or
//! backdoor (if \p path is UVM_BACKDOOR).
//! If \p kind is specified as UVM_PREDICT_WRITE, the value
//! was observed in a write transaction on the specified address \p map or
//! backdoor (if \p path is UVM_BACKDOOR).
//! If \p kind is specified as UVM_PREDICT_DIRECT, the value
//! was computed and is updated as-is, without regard to any access policy.
//! For example, the mirrored value of a read-only field is modified
//! by this method if \p kind is specified as UVM_PREDICT_DIRECT.
//!
//! This method does not allow an update of the mirror
//! when the register containing this field is busy executing
//! a transaction because the results are unpredictable and
//! indicative of a race condition in the testbench.
//!
//! Returns TRUE if the prediction was successful.
//----------------------------------------------------------------------

bool uvm_reg_field::predict( uvm_reg_data_t value,
                             uvm_reg_byte_en_t be,
                             uvm_predict_e kind,
                             uvm_path_e path,
                             uvm_reg_map* map,
                             const std::string& fname,
                             int lineno )
{
  uvm_reg_item* rw = new uvm_reg_item();
  rw->value[0] = value;
  rw->path = path;
  rw->map = map;
  rw->fname = fname;
  rw->lineno = lineno;
  rw->status = UVM_NOT_OK;

  do_predict(rw, kind, be);

  return (rw->status != UVM_NOT_OK);
}


//----------------------------------------------------------------------
// Group: Callbacks
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: pre_write
//
//! Called before field write.
//!
//! If the specified data value, access \p path or address \p map are modified,
//! the updated data value, access path or address map will be used
//! to perform the register operation.
//! If the \p status is modified to anything other than UVM_IS_OK,
//! the operation is aborted.
//!
//! The field callback methods are invoked after the callback methods
//! on the containing register.
//! The registered callback methods are invoked after the invocation
//! of this method.
//----------------------------------------------------------------------

void uvm_reg_field::pre_write( uvm_reg_item* rw )
{}

//----------------------------------------------------------------------
// member function: post_write
//
//! Called after field write.
//!
//! If the specified \p status is modified,
//! the updated status will be
//! returned by the register operation.
//!
//! The field callback methods are invoked after the callback methods
//! on the containing register.
//! The registered callback methods are invoked before the invocation
//! of this method.
//----------------------------------------------------------------------

void uvm_reg_field::post_write( uvm_reg_item* rw )
{}


//----------------------------------------------------------------------
// member function: pre_read
//
//! Called before field read.
//!
//! If the access \p path or address \p map in the \p rw argument are modified,
//! the updated access path or address map will be used to perform
//! the register operation.
//! If the \p status is modified to anything other than UVM_IS_OK,
//! the operation is aborted.
//!
//! The field callback methods are invoked after the callback methods
//! on the containing register.
//! The registered callback methods are invoked after the invocation
//! of this method.
//----------------------------------------------------------------------

void uvm_reg_field::pre_read( uvm_reg_item* rw )
{}


//----------------------------------------------------------------------
// member function: post_read
//
//! Called after field read.
//!
//! If the specified readback data or \p status in the \p rw argument is
//! modified, the updated readback data or status will be
//! returned by the register operation.
//!
//! The field callback methods are invoked after the callback methods
//! on the containing register.
//! The registered callback methods are invoked before the invocation
//! of this method.
//----------------------------------------------------------------------

void uvm_reg_field::post_read( uvm_reg_item* rw )
{}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------
// member function: get_register
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg* uvm_reg_field::get_register() const
{
  return m_parent;
}


//----------------------------------------------------------------------
// member function:  m_predefined_policies
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_reg_field::m_predefine_policies()
{
  if (m_predefined)
    return true;

  m_predefined = true;

  define_access("RO");
  define_access("RW");
  define_access("RC");
  define_access("RS");
  define_access("WRC");
  define_access("WRS");
  define_access("WC");
  define_access("WS");
  define_access("WSRC");
  define_access("WCRS");
  define_access("W1C");
  define_access("W1S");
  define_access("W1T");
  define_access("W0C");
  define_access("W0S");
  define_access("W0T");
  define_access("W1SRC");
  define_access("W1CRS");
  define_access("W0SRC");
  define_access("W0CRS");
  define_access("WO");
  define_access("WOC");
  define_access("WOS");
  define_access("W1");
  define_access("WO1");
  return true;
}

//----------------------------------------------------------------------
// member function: m_predict
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_data_t uvm_reg_field::m_predict( uvm_reg_data_t cur_val,
                                         uvm_reg_data_t wr_val,
                                         uvm_reg_map* map )
{
  uvm_reg_data_t mask = (1 << m_size)-1;
  std::string access = get_access(map);

  if( access == "RO" )    return cur_val;
  if( access == "RW" )    return wr_val;
  if( access == "RC" )    return cur_val;
  if( access == "RS" )    return cur_val;
  if( access == "WC" )    return 0;
  if( access == "WS" )    return mask;
  if( access == "WRC" )   return wr_val;
  if( access == "WRS" )   return wr_val;
  if( access == "WSRC" )  return mask;
  if( access == "WCRS" )  return 0;
  if( access == "W1C" )   return cur_val & (~wr_val);
  if( access == "W1S" )   return cur_val | wr_val;
  if( access == "W1T" )   return cur_val ^ wr_val;
  if( access == "W0C" )   return cur_val & wr_val;
  if( access == "W0S" )   return cur_val | (~wr_val & mask);
  if( access == "W0T" )   return cur_val ^ (~wr_val & mask);
  if( access == "W1SRC" ) return cur_val | wr_val;
  if( access == "W1CRS" ) return cur_val & (~wr_val);
  if( access == "W0SRC" ) return cur_val | (~wr_val & mask);
  if( access == "W0CRS" ) return cur_val & wr_val;
  if( access == "WO" )    return wr_val;
  if( access == "WOC" )   return 0;
  if( access == "WOS" )   return mask;
  if( access == "W1" )    return (m_written) ? cur_val : wr_val;
  if( access == "WO1" )   return (m_written) ? cur_val : wr_val;

  //default:
  return wr_val;
}

//----------------------------------------------------------------------
// member function: m_update
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_data_t uvm_reg_field::m_update()
{
  // Figure out which value must be written to get the desired value
  // given what we think is the current value in the hardware

  uvm_reg_data_t update = 0;

  if( m_access == "RO" )    update = m_desired;
  if( m_access == "RW" )    update = m_desired;
  if( m_access == "RC" )    update = m_desired;
  if( m_access == "RS" )    update = m_desired;
  if( m_access == "WRC" )   update = m_desired;
  if( m_access == "WRS" )   update = m_desired;
  if( m_access == "WC" )    update = m_desired;   // Warn if != 0
  if( m_access == "WS" )    update = m_desired;   // Warn if != 1
  if( m_access == "WSRC" )  update = m_desired;   // Warn if != 1
  if( m_access == "WCRS" )  update = m_desired;   // Warn if != 0
  if( m_access == "W1C" )   update = ~m_desired;
  if( m_access == "W1S" )   update = m_desired;
  if( m_access == "W1T" )   update = m_desired ^ m_mirrored;
  if( m_access == "W0C" )   update = m_desired;
  if( m_access == "W0S" )   update = ~m_desired;
  if( m_access == "W0T" )   update = ~(m_desired ^ m_mirrored);
  if( m_access == "W1SRC" ) update = m_desired;
  if( m_access == "W1CRS" ) update = ~m_desired;
  if( m_access == "W0SRC" ) update = ~m_desired;
  if( m_access == "W0CRS" ) update = m_desired;
  if( m_access == "WO" )    update = m_desired;
  if( m_access == "WOC" )   update = m_desired;   // Warn if != 0
  if( m_access == "WOS" )   update = m_desired;   // Warn if != 1
  if( m_access == "W1" )    update = m_desired;
  if( m_access == "WO1" )   update = m_desired;
  // default:
  update = m_desired;

  update &= (1 << m_size) - 1;

  return update;
}

//----------------------------------------------------------------------
// member function: m_check_access
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_reg_field::m_check_access( uvm_reg_item* rw,
                                    uvm_reg_map_info*& map_info, // output
                                    const std::string& caller)
{

  if (rw->path == UVM_DEFAULT_PATH)
  {
    uvm_reg_block* blk = m_parent->get_block();
    rw->path = blk->get_default_path();
  }

  if (rw->path == UVM_BACKDOOR)
  {
    if (m_parent->get_backdoor() == NULL && !m_parent->has_hdl_path())
    {
      UVM_WARNING("RegModel",
          "No backdoor access available for field '" + get_full_name() +
          "' . Using frontdoor instead.");
      rw->path = UVM_FRONTDOOR;
    }
    else
      rw->map = uvm_reg_map::backdoor();
  }

  if (rw->path != UVM_BACKDOOR)
  {
    rw->local_map = m_parent->get_local_map(rw->map, caller);

    if (rw->local_map == NULL)
    {
      UVM_ERROR(get_type_name(),
          "No transactor available to physically access memory from map '" +
           rw->map->get_full_name() + "'");
      rw->status = UVM_NOT_OK;
      return false;
    }

    map_info = rw->local_map->get_reg_map_info(m_parent);

    if (map_info->frontdoor == NULL && map_info->unmapped)
    {
      UVM_ERROR("RegModel",
          "Field '" + get_full_name() +
          "' in register that is unmapped in map '" +
          rw->map->get_full_name() +
          "' and does not have a user-defined frontdoor");
      rw->status = UVM_NOT_OK;
      return false;
    }

    if (rw->map == NULL)
      rw->map = rw->local_map;
  }

  return true;
}


//----------------------------------------------------------------------
// member function: do_write
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_field::do_write( uvm_reg_item* rw )
{
  uvm_reg_data_t value_adjust;
  uvm_reg_map_info* map_info;
  std::vector<uvm_reg_field*> fields;
  bool bad_side_effect = false;

  m_parent->m_atomic_check_lock(true);
  m_fname  = rw->fname;
  m_lineno = rw->lineno;

  if (!m_check_access(rw, map_info, "write()"))
    return;

  m_write_in_progress = true;

  if (rw->value[0] >> m_size)
  {
    UVM_WARNING("RegModel",
      "uvm_reg_field::write(): Value greater than field '" +
      get_full_name() + "'");
    rw->value[0] &= (( 1 << m_size)-1);
  }

  // Get values to write to the other fields in register
  m_parent->get_fields(fields);

  for( unsigned int i = 0; i < fields.size(); i++ )
  {
    if (fields[i] == this)
    {
      value_adjust |= rw->value[0] << m_lsb;
      continue;
    }

    // It depends on what kind of bits they are made of...
    std::string acc = fields[i]->get_access(rw->local_map);

    if( (acc == "RO") || (acc == "RC") || (acc == "RS") ||
        (acc == "W1C") || (acc == "W1S") || (acc == "W1T") ||
        (acc == "W1SRC") || (acc == "W1CRC") )
    {
      // Use all 0's
      value_adjust |= 0;
    }

    if( (acc == "W0C") || (acc == "W0S") || (acc == "W0T") ||
        (acc == "W0SRC") || (acc == "W0CRS") )
    {
      // Use all 1's
      value_adjust |= uvm_mask_size(fields[i]->get_n_bits()) << fields[i]->get_lsb_pos();
    }

    if( (acc == "WC") || (acc == "WS") || (acc == "WCRS") ||
        (acc == "WSRC") || (acc == "WOC") || (acc == "WOS") )
    {
      // These might have side effects! Bad!
      bad_side_effect = true;
    }

    //default:
    value_adjust |= fields[i]->m_mirrored << fields[i]->get_lsb_pos();
  } // for-loop

#ifdef UVM_REG_NO_INDIVIDUAL_FIELD_ACCESS
  rw->element_kind = UVM_REG;
  rw->element = m_parent;
  rw->value[0] = value_adjust;
  m_parent->do_write(rw);
#else

  if (!is_indv_accessible(rw->path, rw->local_map))
  {
    rw->element_kind = UVM_REG;
    rw->element = m_parent;
    rw->value[0] = value_adjust;
    m_parent->do_write(rw);

    if (bad_side_effect)
      UVM_WARNING("RegModel",
        "Writing field '" + this->get_full_name() +
        "' will cause unintended side effects in adjoining Write-to-Clear or Write-to-Set fields in the same register");
  }
  else
  {
    uvm_reg_map* system_map = rw->local_map->get_root_map();
    uvm_reg_field_cb_iter* cbs = new uvm_reg_field_cb_iter(this);

    m_parent->m_set_busy(1);

    rw->status = UVM_IS_OK;

    pre_write(rw);
    for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next() )
      cb->pre_write(rw);

    if (rw->status != UVM_IS_OK)
    {
      m_write_in_progress = false;
      m_parent->m_set_busy(0);
      m_parent->m_atomic_check_lock(0);
      return;
    }

    rw->local_map->do_write(rw);

    if (system_map->get_auto_predict())
      // TODO from UVM-SV: Call parent.m_sample();
      do_predict(rw, UVM_PREDICT_WRITE);

    post_write(rw);

    for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next())
      cb->post_write(rw);

    m_parent->m_set_busy(false);

    delete cbs;
  }

#endif

  m_write_in_progress = false;
  m_parent->m_atomic_check_lock(false);
}


//----------------------------------------------------------------------
// member function: do_read
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_field::do_read( uvm_reg_item* rw )
{
  uvm_reg_map_info* map_info;
  bool bad_side_effect = false;

  m_parent->m_atomic_check_lock(true);
  m_fname  = rw->fname;
  m_lineno = rw->lineno;
  m_read_in_progress = true;

  if (!m_check_access(rw, map_info, "read()"))
    return;

#ifdef UVM_REG_NO_INDIVIDUAL_FIELD_ACCESS
  rw->element_kind = UVM_REG;
  rw->element = m_parent;
  m_parent->do_read(rw);
  rw->value[0] = (rw->value[0] >> m_lsb) & (( 1 << m_size))-1;
  bad_side_effect = true;
#else

  if (!is_indv_accessible(rw->path,rw->local_map))
  {
    rw->element_kind = UVM_REG;
    rw->element = m_parent;
    bad_side_effect = true;
    m_parent->do_read(rw);
    rw->value[0] = (rw->value[0] >> m_lsb) & ((1<<m_size)-1); // TODO check -1 within brackets?
  }
  else
  {
    uvm_reg_map* system_map = rw->local_map->get_root_map();
    uvm_reg_field_cb_iter* cbs = new uvm_reg_field_cb_iter(this);

    m_parent->m_set_busy(true);
    rw->status = UVM_IS_OK;
    pre_read(rw);

    for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next())
      cb->pre_read(rw);

    if (rw->status != UVM_IS_OK)
    {
      m_read_in_progress = false;
      m_parent->m_set_busy(false);
      m_parent->m_atomic_check_lock(false);
      return;
    }

    rw->local_map->do_read(rw);

    if (system_map->get_auto_predict())
      // TODO from UVM-SV: Call parent.m_sample();
      do_predict(rw, UVM_PREDICT_READ);

    post_read(rw);

    for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next() )
      cb->post_read(rw);

    m_parent->m_set_busy(0);

    delete cbs;

  } // else
#endif

  m_read_in_progress = false;
  m_parent->m_atomic_check_lock(false);

  if (bad_side_effect)
  {
    std::vector<uvm_reg_field*> fields;
    m_parent->get_fields(fields);
    for( unsigned int i = 0; i < fields.size(); i++ )
    {
      std::string mode;
      if (fields[i] == this)
        continue;
      mode = fields[i]->get_access();
      if( mode == "RC" ||
          mode == "RS" ||
          mode == "WRC" ||
          mode == "WRS" ||
          mode == "WSRC" ||
          mode == "WCRS" ||
          mode == "W1SRC" ||
          mode == "W1CRS" ||
          mode == "W0SRC" ||
          mode == "W0CRS")
      {
        UVM_WARNING("RegModel", "Reading field '" + get_full_name() +
            "' will cause unintended side effects in adjoining " +
            "Read-to-Clear or Read-to-Set fields in the same register");
      } // if
    } // for
  }
}


//----------------------------------------------------------------------
// member function: do_predict
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_field::do_predict( uvm_reg_item* rw,
                                uvm_predict_e kind,
                                uvm_reg_byte_en_t be )
{
  uvm_reg_data_t field_val = rw->value[0] & ((1 << m_size)-1);

  if (rw->status != UVM_NOT_OK)
    rw->status = UVM_IS_OK;

  // Assume that the entire field is enabled
  if (!be[0].to_bool()) // TODO check was !be[0]
    return;

  m_fname = rw->fname;
  m_lineno = rw->lineno;

  switch (kind)
  {
    case UVM_PREDICT_WRITE:
   {
      uvm_reg_field_cb_iter* cbs = new uvm_reg_field_cb_iter(this);

      if (rw->path == UVM_FRONTDOOR || rw->path == UVM_PREDICT)
        field_val = m_predict(m_mirrored, field_val, rw->map);

      m_written = 1;

      for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next())
        cb->post_predict(this, m_mirrored, field_val,
            UVM_PREDICT_WRITE, rw->path, rw->map);

      field_val &= ( 1 << m_size )-1;

      delete cbs;

      break;
    }

    case UVM_PREDICT_READ:
    {
      uvm_reg_field_cb_iter* cbs = new uvm_reg_field_cb_iter(this);

      if (rw->path == UVM_FRONTDOOR || rw->path == UVM_PREDICT)
      {
        std::string acc = get_access(rw->map);

        if( acc == "RC" ||
            acc == "WRC" ||
            acc == "WSRC" ||
            acc == "W1SRC" ||
            acc == "W0SRC" )
          field_val = 0;  // (clear)

        else if (acc == "RS" ||
            acc == "WRS" ||
            acc == "WCRS" ||
            acc == "W1CRS" ||
            acc == "W0CRS")
          field_val = (1 << m_size)-1; // all 1's (set)

              else if (acc == "WO" ||
                  acc == "WOC" ||
                  acc == "WOS" ||
                  acc == "WO1")
                return;
        } // if

      for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next() )
        cb->post_predict(this, m_mirrored, field_val,
          UVM_PREDICT_READ, rw->path, rw->map);

      field_val &= (1 << m_size)-1;

      delete cbs;

      break;
    } // case

    case UVM_PREDICT_DIRECT:
    {
      if (m_parent->is_busy())
      {
        UVM_WARNING("RegModel", "Trying to predict value of field '" +
            get_name() + "' while register '" + m_parent->get_full_name() +
            "' is being accessed");
        rw->status = UVM_NOT_OK;
      }
      break;
    } // case
  } // switch

  // update the mirror with predicted value
  m_mirrored = field_val;
  m_desired  = field_val;
  m_value    = field_val;
}

//----------------------------------------------------------------------
// member function: pre_randomize
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_field::pre_randomize()
{
  // Update the only publicly known property with the current
  // desired value so it can be used as a state variable should
  // the rand_mode of the field be turned off.
  m_value = m_desired;
}


//----------------------------------------------------------------------
// member function: post_randomize
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_field::post_randomize()
{
  m_desired = m_value;
}

//----------------------------------------------------------------------
// Implementation defined - member functions for UVM object
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: do_print
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_field::do_print( const uvm_printer& printer ) const
{
  printer.print_generic(get_name(), get_type_name(), -1, convert2string());
}

//----------------------------------------------------------------------
// member function: convert2string
//
// Implementation defined
//----------------------------------------------------------------------

std::string uvm_reg_field::convert2string() const
{
  std::string fmt;
  std::ostringstream res_str;
  std::ostringstream cvt_str;
  std::ostringstream m_str;
  std::string t_str;
  //bool with_debug_info;
  std::string prefix;
  uvm_reg* reg_ = get_register();

  if (m_desired != m_mirrored)
    m_str << " (Mirror: 0x" << std::hex << m_mirrored.to_uint64() << ")";
    else
      m_str << "";

  cvt_str << prefix << " " << get_access() << " " << reg_->get_name()
          << "[" << get_lsb_pos() + get_n_bits() - 1
          << ":" << get_lsb_pos()
          << "] = 0x"
          << std::hex << m_desired.to_uint64()
          << " (0b" << m_desired.range(get_n_bits()-1,0).to_string(sc_dt::SC_BIN_US, false)
          << ")"<< m_str.str();

  if(m_read_in_progress)
  {
    if (!m_fname.empty() && m_lineno != 0)
      res_str << " from " << m_fname << ":" << m_lineno;

    cvt_str << std::endl << "currently being read" << res_str.str();
  }

  if(m_write_in_progress)
  {
    if (!m_fname.empty() && m_lineno != 0)
      res_str << " from " << m_fname << ":" << m_lineno;

    cvt_str << std::endl << "currently being written" << res_str.str();
  }

  return cvt_str.str();
}

//----------------------------------------------------------------------
// member function: clone
//
// Implementation defined
//----------------------------------------------------------------------

uvm_object* uvm_reg_field::clone()
{
  UVM_FATAL("RegModel","RegModel field cannot be cloned");
  return NULL;
}

//----------------------------------------------------------------------
// member function: do_copy
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_field::do_copy( const uvm_object& rhs )
{
  UVM_WARNING("RegModel","RegModel field copy not yet implemented");
  // TODO just a set(rhs.get()) ?
}


//----------------------------------------------------------------------
// member function: do_compare
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_reg_field::do_compare( const uvm_object& rhs,
                                const uvm_comparer* comparer ) const
{
  UVM_WARNING("RegModel","RegModel field compare not yet implemented");
  // TODO just a return (get() == rhs.get()) ?
  return false;
}

//----------------------------------------------------------------------
// member function: do_pack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_field::do_pack( uvm_packer& packer ) const
{
  UVM_WARNING("RegModel","RegModel field cannot be packed");
}

//----------------------------------------------------------------------
// member function: do_unpack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_field::do_unpack( uvm_packer& packer )
{
  UVM_WARNING("RegModel","RegModel field cannot be unpacked");
}

//----------------------------------------------------------------------
// member function: do_unpack
//
// Implementation defined
//----------------------------------------------------------------------

std::map<std::string, bool>& uvm_reg_field::m_policy_names()
{
  static std::map<std::string, bool> policy_names;
  return policy_names;
}

} // namespace uvm
