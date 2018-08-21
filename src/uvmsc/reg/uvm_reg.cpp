//----------------------------------------------------------------------
//   Copyright 2013-2016 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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

#include <iostream>
#include <string>

#include "uvmsc/reg/uvm_reg.h"
#include "uvmsc/reg/uvm_reg_block.h"
#include "uvmsc/reg/uvm_hdl_path_concat.h"
#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/reg/uvm_reg_map.h"
#include "uvmsc/reg/uvm_reg_item.h"
#include "uvmsc/reg/uvm_reg_file.h"
#include "uvmsc/reg/uvm_reg_field.h"
#include "uvmsc/reg/uvm_reg_backdoor.h"
#include "uvmsc/reg/uvm_reg_frontdoor.h"
#include "uvmsc/reg/uvm_reg_cbs.h"
#include "uvmsc/conf/uvm_object_string_pool.h"
#include "uvmsc/macros/uvm_message_defines.h"

namespace uvm {

//------------------------------------------------------------------------------
// Initialization of static data members
//------------------------------------------------------------------------------

unsigned int uvm_reg::m_max_size = 0;

//----------------------------------------------------------------------
// Group: Initialization
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Constructor
//
//! Create a new instance and type-specific configuration
//!
//! Creates an instance of a register abstraction class with the specified
//! name.
//!
//! \p n_bits specifies the total number of bits in the register.
//! Not all bits need to be implemented.
//! This value is usually a multiple of 8.
//!
//! \p has_coverage specifies which functional coverage models are present in
//! the extension of the register abstraction class.
//! Multiple functional coverage models may be specified by adding their
//! symbolic names, as defined by the #uvm_coverage_model_e type.
//----------------------------------------------------------------------

uvm_reg::uvm_reg( const std::string& name,
                  unsigned int n_bits,
                  int has_coverage)
: uvm_object(name)
{
  m_locked = false;
  m_parent = NULL;
  m_regfile_parent = NULL;
  m_process_valid = false;

  if (n_bits > m_max_size)
    m_max_size = n_bits;

  if (n_bits == 0)
  {
    UVM_ERROR("RegModel", "Register '" + get_name() + "' cannot have 0 bits");
    n_bits = 1;
  }

  m_n_bits      = n_bits;
  m_n_used_bits = 0;

  m_maps.clear();
  m_fields.clear();

  m_has_cover = has_coverage;
  m_cover_on = 0;

  m_fname = "";
  m_lineno = 0;

  m_read_in_progress = false;
  m_write_in_progress = false;

  m_is_busy     = false;
  m_is_locked_by_field = false;
  m_backdoor = NULL;

  //m_hdl_paths_pool = new uvm_object_string_pool< uvm_queue<uvm_hdl_path_concat*>* >("hdl_paths");
  m_hdl_paths_pool.clear();

  m_update_in_progress = false;
}

//----------------------------------------------------------------------
// member function: configure
//
//! Instance-specific configuration
//!
//! Specify the parent block of this register.
//! May also set a parent register file for this register,
//!
//! If the register is implemented in a single HDL variable,
//! it's name is specified as the \p hdl_path.
//! Otherwise, if the register is implemented as a concatenation
//! of variables (usually one per field), then the HDL path
//! must be specified using the add_hdl_path() or
//! add_hdl_path_slice method.
//----------------------------------------------------------------------

void uvm_reg::configure( uvm_reg_block* blk_parent,
                         uvm_reg_file* regfile_parent,
                         const std::string& hdl_path )
{
  m_parent = blk_parent;
  m_parent->add_reg(this);
  m_regfile_parent = regfile_parent;

  if (!hdl_path.empty())
    add_hdl_path_slice(hdl_path, -1, -1);
}


//----------------------------------------------------------------------
// member function: set_offset
//
//! Modify the offset of the register
//!
//! The offset of a register within an address map is set using the
//! uvm_reg_map::add_reg() method.
//! This method is used to modify that offset dynamically.
//!
//! Modifying the offset of a register will make the register model
//! diverge from the specification that was used to create it.
//----------------------------------------------------------------------

void uvm_reg::set_offset( uvm_reg_map* map,
                          uvm_reg_addr_t offset,
                          bool unmapped )
{
  if( m_maps.size() > 1 && map == NULL)
  {
    UVM_ERROR("RegModel",
      "set_offset requires a non-null map when register '" +
          get_full_name() + "' belongs to more than one map.");
    return;
  }

  map = get_local_map(map, "set_offset()");

  if (map == NULL)
    return;

  map->m_set_reg_offset(this, offset, unmapped);
}


//----------------------------------------------------------------------
// Group: Introspection
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: get_name
//
// Get the simple name
//
// Return the simple object name of this register.
//----------------------------------------------------------------------

// inherited from base class

//----------------------------------------------------------------------
// member function: get_full_name
//
//! Get the hierarchical name
//!
//! Return the hierarchical name of this register.
//! The base of the hierarchical name is the root block.
//----------------------------------------------------------------------

const std::string uvm_reg::get_full_name() const
{
  if (m_regfile_parent != NULL)
    return m_regfile_parent->get_full_name() + "." + get_name();

  if (m_parent != NULL)
    return m_parent->get_full_name() + "." + get_name();

  return get_name();
}

//----------------------------------------------------------------------
// member function: get_parent
//
//! Get the parent block
//----------------------------------------------------------------------

uvm_reg_block* uvm_reg::get_parent() const
{
  return m_parent;
}


//----------------------------------------------------------------------
// member function: get_regfile
//
//! Get the parent register file
//!
//! Returns NULL if this register is instantiated in a block.
//----------------------------------------------------------------------

uvm_reg_file* uvm_reg::get_regfile() const
{
  return m_regfile_parent;
}

//----------------------------------------------------------------------
// member function: get_n_maps
//
//! Returns the number of address maps this register is mapped in
//----------------------------------------------------------------------

int uvm_reg::get_n_maps() const
{
  return m_maps.size();
}

//----------------------------------------------------------------------
// member function: is_in_map
//
//! Returns true if this register is in the specified address \p map
//----------------------------------------------------------------------

bool uvm_reg::is_in_map( uvm_reg_map* map ) const
{
  if ( m_maps.find(map) != m_maps.end() ) // exists
    return true;

  for( m_maps_citt it = m_maps.begin(); it != m_maps.end(); it++)
  {
    uvm_reg_map* local_map = (*it).first;
    uvm_reg_map* parent_map = local_map->get_parent_map();

    while (parent_map != NULL)
    {
      if (parent_map == map)
        return true;
      parent_map = parent_map->get_parent_map();
    }
  }
  return false;
}

//----------------------------------------------------------------------
// member function: get_maps
//
//! Returns all of the address \p maps where this register is mapped
//----------------------------------------------------------------------

void uvm_reg::get_maps( std::vector<uvm_reg_map*>& maps ) const
{
  for( m_maps_citt it = m_maps.begin(); it != m_maps.end(); it++)
     maps.push_back( (*it).first );
}

//----------------------------------------------------------------------
// member function: get_rights
//
//! Returns the accessibility ("RW, "RO", or "WO") of this register in the given \p map.
//!
//! If no address map is specified and the register is mapped in only one
//! address map, that address map is used. If the register is mapped
//! in more than one address map, the default address map of the
//! parent block is used.
//!
//! Whether a register field can be read or written depends on both the field's
//! configured access policy (see uvm_reg_field::configure) and the register's
//! accessibility rights in the map being used to access the field.
//!
//! If an address map is specified and
//! the register is not mapped in the specified
//! address map, an error message is issued
//! and "RW" is returned.
//----------------------------------------------------------------------

std::string uvm_reg::get_rights( uvm_reg_map* map ) const
{
  uvm_reg_map_info* info = NULL;

  map = get_local_map(map, "get_rights()");

  if (map == NULL)
    return "RW";

  info = map->get_reg_map_info(this);
  return info->rights;
}

//----------------------------------------------------------------------
// member function: get_n_bits
//
//! Returns the width, in bits, of this register.
//----------------------------------------------------------------------

unsigned int uvm_reg::get_n_bits() const
{
   return m_n_bits;
}

//----------------------------------------------------------------------
// member function: get_n_bytes
//
//! Returns the width, in bytes, of this register. Rounds up to
//! next whole byte if register is not a multiple of 8.
//----------------------------------------------------------------------

unsigned int uvm_reg::get_n_bytes() const
{
  return ((m_n_bits-1) / 8) + 1;
}

//----------------------------------------------------------------------
// member function: get_max_size (static)
//
//! Returns the maximum width, in bits, of all registers.
//----------------------------------------------------------------------

unsigned int uvm_reg::get_max_size()
{
  return m_max_size;
}

//----------------------------------------------------------------------
// member function: get_fields
//
//! Return the fields in this register
//!
//! Fills the specified array with the abstraction class
//! for all of the fields contained in this register.
//! Fields are ordered from least-significant position to most-significant
//----------------------------------------------------------------------

void uvm_reg::get_fields( std::vector<uvm_reg_field*>& fields ) const
{
  for( unsigned int i = 0; i < m_fields.size(); i++ )
    fields.push_back(m_fields[i]);
}


//----------------------------------------------------------------------
// member function: get_field_by_name
//
//! Return the named field in this register
//!
//! Finds a field with the specified name in this register
//! and returns its abstraction class.
//! If no fields are found, returns NULL.
//----------------------------------------------------------------------

uvm_reg_field* uvm_reg::get_field_by_name( const std::string& name ) const
{
  for( unsigned int i = 0; i < m_fields.size(); i++ )
    if ( m_fields[i]->get_name() == name)
      return m_fields[i];

  UVM_WARNING("RegModel",
    "Unable to locate field '" + name + "' in register '" + get_name() + "'");

  return NULL;
}

//----------------------------------------------------------------------
// member function: get_offset
//
//! Returns the offset of this register
//!
//! Returns the offset of this register in an address \p map.
//!
//! If no address map is specified and the register is mapped in only one
//! address map, that address map is used. If the register is mapped
//! in more than one address map, the default address map of the
//! parent block is used.
//!
//! If an address map is specified and
//! the register is not mapped in the specified
//! address map, an error message is issued.
//----------------------------------------------------------------------

uvm_reg_addr_t uvm_reg::get_offset( uvm_reg_map* map ) const
{
  uvm_reg_map_info* map_info;

  uvm_reg_map* orig_map = map;

  map = get_local_map(map,"get_offset()");

  if (map == NULL)
    return -1;

  map_info = map->get_reg_map_info(this);

  if (map_info->unmapped)
  {
	std::ostringstream str;
    str << "Register '"
        << get_name()
        << "' is unmapped in map '"
        << ((orig_map == NULL) ? map->get_full_name() : orig_map->get_full_name())
        << "'";

    UVM_WARNING("RegModel", str.str() );
    return -1;
  }

  return map_info->offset;
}

//----------------------------------------------------------------------
// member function: get_address
//
//! Returns the base external physical address of this register
//!
//! Returns the base external physical address of this register
//! if accessed through the specified address \p map.
//!
//! If no address map is specified and the register is mapped in only one
//! address map, that address map is used. If the register is mapped
//! in more than one address map, the default address map of the
//! parent block is used.
//!
//! If an address map is specified and
//! the register is not mapped in the specified
//! address map, an error message is issued.
//----------------------------------------------------------------------

uvm_reg_addr_t uvm_reg::get_address( const uvm_reg_map* map ) const
{
   std::vector<uvm_reg_addr_t> addr ;
   uvm_reg_addr_t a;
   get_addresses(addr, map);
   if (addr.size() > 0) { a = addr[0]; }
   else a = 0;
   return a;
}

//----------------------------------------------------------------------
// member function: get_addresses
//
//! Identifies the external physical address(es) of this register
//!
//! Computes all of the external physical addresses that must be accessed
//! to completely read or write this register. The addressed are specified in
//! little endian order.
//! Returns the number of bytes transfered on each access.
//!
//! If no address map is specified and the register is mapped in only one
//! address map, that address map is used. If the register is mapped
//! in more than one address map, the default address map of the
//! parent block is used.
//!
//! If an address map is specified and
//! the register is not mapped in the specified
//! address map, an error message is issued.
//----------------------------------------------------------------------

int uvm_reg::get_addresses( std::vector<uvm_reg_addr_t>& addr, const uvm_reg_map* map ) const
{
  uvm_reg_map_info* map_info;

  uvm_reg_map* lmap = get_local_map(map, "get_addresses()");

  uvm_reg_map* orig_map = lmap;

  if (lmap == NULL)
    return -1;

  map_info = lmap->get_reg_map_info(this);

  if (map_info->unmapped)
  {
    std::ostringstream str;
    str << "Register '"
        << get_name()
        << "' is unmapped in map '"
        << ((orig_map == NULL) ? lmap->get_full_name() : orig_map->get_full_name())
        << "'";

    UVM_WARNING("RegModel", str.str() );
    return -1;
  }

  addr = map_info->addr;
  return lmap->get_n_bytes();
}

//----------------------------------------------------------------------
// Group: Access
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// member function: set
//
//! Set the desired value for this register
//!
//! Sets the desired value of the fields in the register
//! to the specified value. Does not actually
//! set the value of the register in the design,
//! only the desired value in its corresponding
//! abstraction class in the RegModel model.
//! Use the uvm_reg::update() method to update the
//! actual register with the mirrored value or
//! the uvm_reg::write() method to set
//! the actual register and its mirrored value.
//!
//! Unless this method is used, the desired value is equal to
//! the mirrored value.
//!
//! Refer uvm_reg_field::set() for more details on the effect
//! of setting mirror values on fields with different
//! access policies.
//!
//! To modify the mirrored field values to a specific value,
//! and thus use the mirrored as a scoreboard for the register values
//! in the DUT, use the uvm_reg::predict() method.
//----------------------------------------------------------------------

void uvm_reg::set( uvm_reg_data_t value,
                   const std::string& fname,
                   int lineno )
{
  // Split the value into the individual fields
  m_fname = fname;
  m_lineno = lineno;

  for( unsigned int i = 0; i < m_fields.size(); i++ )
  {
    uvm_reg_data_t val = (value >> m_fields[i]->get_lsb_pos()) &
      ((1 << m_fields[i]->get_n_bits()) - 1);
    m_fields[i]->set(val);
  }
}

//----------------------------------------------------------------------
// member function: get
//
//! Return the desired value of the fields in the register.
//!
//! Does not actually read the value
//! of the register in the design, only the desired value
//! in the abstraction class. Unless set to a different value
//! using the uvm_reg::set(), the desired value
//! and the mirrored value are identical.
//!
//! Use the uvm_reg::read() or uvm_reg::peek()
//! method to get the actual register value.
//!
//! If the register contains write-only fields, the desired/mirrored
//! value for those fields are the value last written and assumed
//! to reside in the bits implementing these fields.
//! Although a physical read operation would something different
//! for these fields,
//! the returned value is the actual content.
//----------------------------------------------------------------------

uvm_reg_data_t uvm_reg::get( const std::string& fname, int lineno ) const
{
  // Concatenate the value of the individual fields
  // to form the register value
  m_fname = fname;
  m_lineno = lineno;

  uvm_reg_data_t _get = 0;

  for( unsigned int i = 0; i < m_fields.size(); i++ )
    _get |= m_fields[i]->get() << m_fields[i]->get_lsb_pos();

  return _get;
}

//----------------------------------------------------------------------
// member function: get_mirrored_value
//
//! Return the mirrored value of the fields in the register.
//!
//! Does not actually read the value
//! of the register in the design
//!
//! If the register contains write-only fields, the desired/mirrored
//! value for those fields are the value last written and assumed
//! to reside in the bits implementing these fields.
//! Although a physical read operation would something different
//! for these fields, the returned value is the actual content.
//----------------------------------------------------------------------

uvm_reg_data_t uvm_reg::get_mirrored_value( const std::string& fname,
                                            int lineno ) const
{
  // Concatenate the value of the individual fields
  // to form the register value
  m_fname = fname;
  m_lineno = lineno;

  uvm_reg_data_t _get_mirrored_value = 0;

  for( unsigned int i =0; i < m_fields.size(); i++ )
    _get_mirrored_value |= m_fields[i]->get_mirrored_value() << m_fields[i]->get_lsb_pos();

  return _get_mirrored_value;
}


//----------------------------------------------------------------------
// member function: needs_update
//
//! Returns true if any of the fields need updating
//!
//! See uvm_reg_field::needs_update() for details.
//! Use the uvm_reg::update() to actually update the DUT register.
//----------------------------------------------------------------------

bool uvm_reg::needs_update() const
{
  for( unsigned int i =0; i < m_fields.size(); i++ )
  {
    if ( m_fields[i]->needs_update() )
      return true;
  }
  return false;
}

//----------------------------------------------------------------------
// member function: reset
//
//! Reset the desired/mirrored value for this register.
//!
//! Sets the desired and mirror value of the fields in this register
//! to the reset value for the specified reset \p kind.
//! See uvm_reg_field.reset() for more details.
//!
//! Also resets the semaphore that prevents concurrent access
//! to the register.
//! This semaphore must be explicitly reset if a thread accessing
//! this register array was killed in before the access
//! was completed
//----------------------------------------------------------------------

void uvm_reg::reset( const std::string& kind )
{
  for( unsigned int i = 0; i < m_fields.size(); i++ )
    m_fields[i]->reset(kind);

  // Put back a key in the semaphore if it is checked out
  // in case a thread was killed during an operation

  m_atomic.trylock(); //try_get
  m_atomic.unlock(); // put

  m_process_valid = false;
}

//----------------------------------------------------------------------
// member function: get_reset
//
//! Get the specified reset value for this register
//!
//! Return the reset value for this register
//! for the specified reset \p kind.
//----------------------------------------------------------------------

uvm_reg_data_t uvm_reg::get_reset( const std::string& kind ) const
{
  // Concatenate the value of the individual fields
  // to form the register value
  uvm_reg_data_t _get_reset = 0;

  for( unsigned int i =0; i < m_fields.size(); i++ )
    _get_reset |= m_fields[i]->get_reset(kind) << m_fields[i]->get_lsb_pos();

  return _get_reset;
}

//----------------------------------------------------------------------
// member function: has_reset
//
//! Check if any field in the register has a reset value specified
//! for the specified reset \p kind.
//! If \p delete is true, removes the reset value, if any.
//----------------------------------------------------------------------

bool uvm_reg::has_reset( const std::string& kind,
                         bool do_delete )
{
  bool _has_reset = false;

  for( unsigned int i =0; i < m_fields.size(); i++ )
  {
    _has_reset |= m_fields[i]->has_reset(kind, do_delete);

    if (!do_delete && _has_reset )
      return true;
  }
  return _has_reset;
}

//----------------------------------------------------------------------
// member function: set_reset
//
//! Specify or modify the reset value for this register
//!
//! Specify or modify the reset value for all the fields in the register
//! corresponding to the cause specified by \p kind.
//----------------------------------------------------------------------

void uvm_reg::set_reset( uvm_reg_data_t value,
                         const std::string& kind )
{
  for( unsigned int i =0; i < m_fields.size(); i++ )
    m_fields[i]->set_reset( value >> m_fields[i]->get_lsb_pos(), kind );
}

//----------------------------------------------------------------------
// member function: write
//
//! Write the specified value in this register
//!
//! Write \p value in the DUT register that corresponds to this
//! abstraction class instance using the specified access
//! \p path.
//! If the register is mapped in more than one address map,
//! an address \p map must be
//! specified if a physical access is used (front-door access).
//! If a back-door access path is used, the effect of writing
//! the register through a physical access is mimicked. For
//! example, read-only bits in the registers will not be written.
//!
//! The mirrored value will be updated using the uvm_reg::predict()
//! method.
//----------------------------------------------------------------------

void uvm_reg::write( uvm_status_e& status,
                     uvm_reg_data_t value,
                     uvm_path_e path,
                     uvm_reg_map* map,
                     uvm_sequence_base* parent,
                     int prior,
                     uvm_object* extension,
                     const std::string& fname,
                     int lineno )
{
   // create an abstract transaction for this operation
   uvm_reg_item* rw;

   m_atomic_check_lock(true);

   set(value);

   rw = uvm_reg_item::type_id::create("write_item", NULL, get_full_name());

   // make sure we have reserved space to store an initial value
   if (rw->value.size() == 0)
     rw->value.resize(1);

   rw->element      = this;
   rw->element_kind = UVM_REG;
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

   m_atomic_check_lock(false);
}

//----------------------------------------------------------------------
// member function: read
//
//! Read the current value from this register
//!
//! Read and return \p value from the DUT register that corresponds to this
//! abstraction class instance using the specified access
//! \p path.
//! If the register is mapped in more than one address map,
//! an address \p map must be
//! specified if a physical access is used (front-door access).
//! If a back-door access path is used, the effect of reading
//! the register through a physical access is mimicked. For
//! example, clear-on-read bits in the registers will be set to zero.
//!
//! The mirrored value will be updated using the uvm_reg::predict()
//! method.
//----------------------------------------------------------------------

void uvm_reg::read( uvm_status_e& status,
                    uvm_reg_data_t& value,
                    uvm_path_e path,
                    uvm_reg_map* map,
                    uvm_sequence_base* parent,
                    int prior,
                    uvm_object* extension,
                    const std::string& fname,
                    int lineno )
{
   m_atomic_check_lock(true);
   m_read(status, value, path, map, parent, prior, extension, fname, lineno);
   m_atomic_check_lock(false);
}

//----------------------------------------------------------------------
// member function: poke
//
//! Deposit the specified value in this register
//!
//! Deposit the value in the DUT register corresponding to this
//! abstraction class instance, as-is, using a back-door access.
//!
//! Uses the HDL path for the design abstraction specified by \p kind.
//!
//! The mirrored value will be updated using the uvm_reg::predict()
//! method.
//----------------------------------------------------------------------

void uvm_reg::poke( uvm_status_e& status,
                    uvm_reg_data_t value,
                    const std::string& kind,
                    uvm_sequence_base* parent,
                    uvm_object* extension,
                    const std::string& fname,
                    int lineno )
{
  uvm_reg_backdoor* bkdr = get_backdoor();
  uvm_reg_item* rw = NULL;

  m_fname = fname;
  m_lineno = lineno;

  if (bkdr == NULL && !has_hdl_path(kind))
  {
    UVM_ERROR("RegModel",
        "No backdoor access available to poke register '" + get_full_name() + "'");
        status = UVM_NOT_OK;
    return;
  }

  if (!m_is_locked_by_field)
    m_atomic_check_lock(true);

  // create an abstract transaction for this operation
  rw = uvm_reg_item::type_id::create("reg_poke_item", NULL, get_full_name());
  rw->element      = this;
  rw->path         = UVM_BACKDOOR;
  rw->element_kind = UVM_REG;
  rw->access_kind  = UVM_WRITE;
  rw->bd_kind      = kind;
  rw->value[0]     = value & uvm_mask_size(m_n_bits);
  rw->parent       = parent;
  rw->extension    = extension;
  rw->fname        = fname;
  rw->lineno       = lineno;

  if (bkdr != NULL)
    bkdr->write(rw);
  else
    backdoor_write(rw);

  status = rw->status;

  std::ostringstream str;
  str << "Poked register '"
      << get_full_name()
      << "': 0x"
      << std::hex << value.to_uint64();
  UVM_INFO("RegModel", str.str(), UVM_HIGH);

  do_predict(rw, UVM_PREDICT_WRITE);

  if (!m_is_locked_by_field)
    m_atomic_check_lock(false);
}

//----------------------------------------------------------------------
// member function: peek
//
//! Read the current value from this register
//!
//! Sample the value in the DUT register corresponding to this
//! abstraction class instance using a back-door access.
//! The register value is sampled, not modified.
//!
//! Uses the HDL path for the design abstraction specified by \p kind.
//!
//! The mirrored value will be updated using the uvm_reg::predict()
//! method.
//----------------------------------------------------------------------

void uvm_reg::peek( uvm_status_e& status,
                    uvm_reg_data_t& value,
                    const std::string& kind,
                    uvm_sequence_base* parent,
                    uvm_object* extension,
                    const std::string& fname,
                    int lineno )
{
  uvm_reg_backdoor* bkdr = get_backdoor();
  uvm_reg_item* rw = NULL;

  m_fname = fname;
  m_lineno = lineno;

  if (bkdr == NULL && !has_hdl_path(kind))
  {
    UVM_ERROR("RegModel",
        "No backdoor access available to peek register '" + get_full_name() + "'");
        status = UVM_NOT_OK;
    return;
  }

  if(!m_is_locked_by_field)
    m_atomic_check_lock(true);

  // create an abstract transaction for this operation
  rw = uvm_reg_item::type_id::create("mem_peek_item", NULL, get_full_name());
  rw->element      = this;
  rw->path         = UVM_BACKDOOR;
  rw->element_kind = UVM_REG;
  rw->access_kind  = UVM_READ;
  rw->bd_kind      = kind;
  rw->parent       = parent;
  rw->extension    = extension;
  rw->fname        = fname;
  rw->lineno       = lineno;

  if (bkdr != NULL)
    bkdr->read(rw);
  else
    backdoor_read(rw);

  status = rw->status;
  value = rw->value[0];

  std::ostringstream str;
  str << "Peeked register '"
      << get_full_name()
      << "': 0x"
      << std::hex << value.to_uint64();
  UVM_INFO("RegModel", str.str(), UVM_HIGH);

  do_predict(rw, UVM_PREDICT_READ);

  if (!m_is_locked_by_field)
    m_atomic_check_lock(false);
}

//----------------------------------------------------------------------
// member function: update
//
//! Updates the content of the register in the design to match the
//! desired value
//!
//! This method performs the reverse
//! operation of uvm_reg::mirror().
//! Write this register if the DUT register is out-of-date with the
//! desired/mirrored value in the abstraction class, as determined by
//! the uvm_reg::needs_update() method.
//!
//! The update can be performed using the using the physical interfaces
//! (frontdoor) or uvm_reg::poke() (backdoor) access.
//! If the register is mapped in multiple address maps and physical access
//! is used (front-door), an address \p map must be specified.
//----------------------------------------------------------------------

void uvm_reg::update( uvm_status_e& status,
                      uvm_path_e path,
                      uvm_reg_map* map,
                      uvm_sequence_base* parent,
                      int prior,
                      uvm_object* extension,
                      const std::string& fname,
                      int lineno )
{
  uvm_reg_data_t upd;

  status = UVM_IS_OK;

  if (!needs_update())
    return;

  // Concatenate the write-to-update values from each field
  // Fields are stored in LSB or MSB order
  upd = 0;

  for( unsigned int i = 0; i < m_fields.size(); i++ )
  upd |= m_fields[i]->m_update() << m_fields[i]->get_lsb_pos();

  write(status, upd, path, map, parent, prior, extension, fname, lineno);
}

//----------------------------------------------------------------------
// member function: mirror
//
//! Read the register and update/check its mirror value
//!
//! Read the register and optionally compared the readback value
//! with the current mirrored value if \p check is UVM_CHECK.
//! The mirrored value will be updated using the uvm_reg::predict()
//! method based on the readback value.
//!
//! The mirroring can be performed using the physical interfaces (frontdoor)
//! or uvm_reg::peek() (backdoor).
//!
//! If \p check is specified as UVM_CHECK,
//! an error message is issued if the current mirrored value
//! does not match the readback value. Any field whose check has been
//! disabled with uvm_reg_field::set_compare() will not be considered
//! in the comparison.
//!
//! If the register is mapped in multiple address maps and physical
//! access is used (front-door access), an address \p map must be specified.
//! If the register contains
//! write-only fields, their content is mirrored and optionally
//! checked only if a UVM_BACKDOOR
//! access path is used to read the register.
//----------------------------------------------------------------------

void uvm_reg::mirror( uvm_status_e& status,
                      uvm_check_e check,
                      uvm_path_e path,
                      uvm_reg_map* map,
                      uvm_sequence_base* parent,
                      int prior,
                      uvm_object* extension,
                      const std::string& fname,
                      int  lineno )
{
   uvm_reg_data_t  v;
   uvm_reg_data_t  exp;
   uvm_reg_backdoor* bkdr = get_backdoor();

   m_atomic_check_lock(true);
   m_fname = fname;
   m_lineno = lineno;

   if (path == UVM_DEFAULT_PATH)
     path = m_parent->get_default_path();

   if (path == UVM_BACKDOOR && (bkdr != NULL || has_hdl_path()))
     map = uvm_reg_map::backdoor();
   else
     map = get_local_map(map, "read()");

   if (map == NULL)
     return;

   // Remember what we think the value is before it gets updated
   if (check == UVM_CHECK)
     exp = get_mirrored_value();

   m_read(status, v, path, map, parent, prior, extension, fname, lineno);

   if (status == UVM_NOT_OK)
   {
      m_atomic_check_lock(false);
      return;
   }

   if (check == UVM_CHECK)
     do_check(exp, v, map);

   m_atomic_check_lock(false);
}

//----------------------------------------------------------------------
// member function: predict
//
//! Update the mirrored value for this register.
//!
//! Predict the mirror value of the fields in the register
//! based on the specified observed \p value on a specified address \p map,
//! or based on a calculated value.
//! See uvm_reg_field::predict() for more details.
//!
//! Returns true if the prediction was successful for each field in the
//! register.
//----------------------------------------------------------------------

bool uvm_reg::predict( uvm_reg_data_t value,
                       uvm_reg_byte_en_t be,
                       uvm_predict_e kind,
                       uvm_path_e path,
                       uvm_reg_map* map,
                       const std::string&  fname,
                       int lineno )
{
  uvm_reg_item* rw = new uvm_reg_item();

  // make sure we have reserved space to store an initial value
  if (rw->value.size() == 0)
    rw->value.resize(1);

  rw->value[0] = value;
  rw->path = path;
  rw->map = map;
  rw->fname = fname;
  rw->lineno = lineno;

  do_predict(rw, kind, be);

  return (rw->status == UVM_NOT_OK) ? false : true;
}

//----------------------------------------------------------------------
// member function: is_busy
//
//! Returns true if register is currently being read or written.
//----------------------------------------------------------------------

bool uvm_reg::is_busy() const
{
   return m_is_busy;
}

//----------------------------------------------------------------------
// Group: Frontdoor
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: set_frontdoor
//
//! Set a user-defined frontdoor for this register
//!
//! By default, registers are mapped linearly into the address space
//! of the address maps that instantiate them.
//! If registers are accessed using a different mechanism,
//! a user-defined access
//! mechanism must be defined and associated with
//! the corresponding register abstraction class
//!
//! If the register is mapped in multiple address maps, an address \p map
//! must be specified.
//----------------------------------------------------------------------

void uvm_reg::set_frontdoor( uvm_reg_frontdoor* ftdr,
                             uvm_reg_map* map,
                             const std::string& fname,
                             int lineno )
{
  uvm_reg_map_info* map_info;

  ftdr->m_fname = m_fname;
  ftdr->m_lineno = m_lineno;

  map = get_local_map(map, "set_frontdoor()");

  if (map == NULL)
    return;

  map_info = map->get_reg_map_info(this);

  if (map_info == NULL)
    map->add_reg(this, -1, "RW", 1, ftdr);
  else
    map_info->frontdoor = ftdr;
}

//----------------------------------------------------------------------
// member function: get_frontdoor
//
//! Returns the user-defined frontdoor for this register
//!
//! If NULL, no user-defined frontdoor has been defined.
//! A user-defined frontdoor is defined
//! by using the uvm_reg::set_frontdoor() method.
//!
//! If the register is mapped in multiple address maps, an address \p map
//! must be specified.
//----------------------------------------------------------------------

uvm_reg_frontdoor* uvm_reg::get_frontdoor( uvm_reg_map* map ) const
{
  uvm_reg_map_info* map_info;

  map = get_local_map(map, "get_frontdoor()");

  if (map == NULL)
    return NULL;

  map_info = map->get_reg_map_info(this);

  return map_info->frontdoor;
}

//----------------------------------------------------------------------
// Group: Backdoor
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: set_backdoor
//
//! Set a user-defined backdoor for this register
//!
//! By default, registers are accessed via the built-in string-based
//! DPI routines if an HDL path has been specified using the
//! uvm_reg::configure() or uvm_reg::add_hdl_path() method.
//!
//! If this default mechanism is not suitable (e.g. because
//! the register is not implemented in pure SystemVerilog)
//! a user-defined access
//! mechanism must be defined and associated with
//! the corresponding register abstraction class
//!
//! A user-defined backdoor is required if active update of the
//! mirror of this register abstraction class, based on observed
//! changes of the corresponding DUT register, is used.
//----------------------------------------------------------------------

void uvm_reg::set_backdoor( uvm_reg_backdoor* bkdr,
                            const std::string& fname,
                            int lineno )
{
   bkdr->m_fname = fname;
   bkdr->m_lineno = lineno;

   if (m_backdoor != NULL && m_backdoor->has_update_threads() )
      UVM_WARNING("RegModel", "Previous register backdoor still has update threads running. Backdoors with active mirroring should only be set before simulation starts.");

   m_backdoor = bkdr;
}

//----------------------------------------------------------------------
// member function: get_backdoor
//
// Returns the user-defined backdoor for this register
//
//! If NULL, no user-defined backdoor has been defined.
//! A user-defined backdoor is defined
//! by using the uvm_reg::set_backdoor() method.
//!
//! If \p inherited is true, returns the backdoor of the parent block
//! if none have been specified for this register.
//----------------------------------------------------------------------

uvm_reg_backdoor* uvm_reg::get_backdoor( bool inherited ) const
{
  if (m_backdoor == NULL && inherited)
  {
    uvm_reg_block* blk = get_parent();
    uvm_reg_backdoor* bkdr;

    while (blk != NULL)
    {
      bkdr = blk->get_backdoor();
      if (bkdr != NULL)
      {
        m_backdoor = bkdr;
        break;
      }
      blk = blk->get_parent();
    }
  }

  return m_backdoor;
}

//----------------------------------------------------------------------
// member function: clear_hdl_path
//
// Delete HDL paths
//
//! Remove any previously specified HDL path to the register instance
//! for the specified design abstraction.
//----------------------------------------------------------------------

void uvm_reg::clear_hdl_path( const std::string& kind )
{
  std::string lkind = kind;

  if (lkind == "ALL")
  {
    m_hdl_paths_pool.clear();
    return;
  }

  if (lkind.empty()) {
    if (m_regfile_parent != NULL)
      lkind = m_regfile_parent->get_default_hdl_path();
    else
      lkind = m_parent->get_default_hdl_path();
  }

  if (m_hdl_paths_pool.find(lkind) == m_hdl_paths_pool.end())
  {
    UVM_WARNING("RegModel", "Unknown HDL Abstraction '" + lkind + "'");
    return;
  }

  m_hdl_paths_pool.erase(lkind);
}

//----------------------------------------------------------------------
// member function: add_hdl_path
//
//! Add an HDL path
//!
//! Add the specified HDL path to the register instance for the specified
//! design abstraction. This method may be called more than once for the
//! same design abstraction if the register is physically duplicated
//! in the design abstraction
//!
//! For example, the following register
//!
//!|        1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//!| Bits:  5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//!|       +-+---+-------------+---+-------+
//!|       |A|xxx|      B      |xxx|   C   |
//!|       +-+---+-------------+---+-------+
//!
//! would be specified using the following literal value:
//!
//!| add_hdl_path('{ '{"A_reg", 15, 1},
//!|                 '{"B_reg",  6, 7},
//!|                 '{'C_reg",  0, 4} } );
//!
//! If the register is implemented using a single HDL variable,
//! The array should specify a single slice with its offset and size
//! specified as -1.
//----------------------------------------------------------------------

void uvm_reg::add_hdl_path( std::vector<uvm_hdl_path_slice> slices,
                            const std::string& kind )
{
  std::vector<uvm_hdl_path_concat> paths;

  if (m_hdl_paths_pool.find(kind) != m_hdl_paths_pool.end())
    paths = m_hdl_paths_pool.find(kind)->second;

  uvm_hdl_path_concat concat;
  concat.set(slices);
  paths.push_back(concat);

  m_hdl_paths_pool[kind] = paths;
}

//----------------------------------------------------------------------
// member function: add_hdl_path_slice
//
//! Append the specified HDL slice to the HDL path of the register instance
//! for the specified design abstraction.
//! If \p first is true, starts the specification of a duplicate
//! HDL implementation of the register.
//----------------------------------------------------------------------

void uvm_reg::add_hdl_path_slice( const std::string& name,
                                  int offset,
                                  int size,
                                  bool first,
                                  const std::string& kind )
{
  std::vector<uvm_hdl_path_concat> paths;

  if (m_hdl_paths_pool.find(kind) != m_hdl_paths_pool.end())
    paths = m_hdl_paths_pool.find(kind)->second;

  uvm_hdl_path_concat concat;

  if (first || paths.size() == 0)
  {
    concat.add_path(name, offset, size);
    paths.push_back(concat);
  }
  else
  {
    concat = paths[paths.size()-1];
    concat.add_path(name, offset, size);
    paths[paths.size()-1] = concat;
  }
  m_hdl_paths_pool[kind] = paths;
}

//----------------------------------------------------------------------
// member function: has_hdl_path
//
//! Check if a HDL path is specified
//!
//! Returns true if the register instance has a HDL path defined for the
//! specified design abstraction. If no design abstraction is specified,
//! uses the default design abstraction specified for the parent block.
//----------------------------------------------------------------------

bool uvm_reg::has_hdl_path( const std::string& kind ) const
{
  std::string kindl = kind;

  if (kind.empty())
  {
    if (m_regfile_parent != NULL)
      kindl = m_regfile_parent->get_default_hdl_path();
    else
      kindl = m_parent->get_default_hdl_path();
  }

  return (m_hdl_paths_pool.find(kindl) != m_hdl_paths_pool.end() ); // exists
}

//----------------------------------------------------------------------
// member function:  get_hdl_path
//
//! Get the incremental HDL path(s)
//!
//! Returns the HDL path(s) defined for the specified design abstraction
//! in the register instance.
//! Returns only the component of the HDL paths that corresponds to
//! the register, not a full hierarchical path
//!
//! If no design abstraction is specified, the default design abstraction
//! for the parent block is used.
//----------------------------------------------------------------------

void uvm_reg::get_hdl_path( std::vector<uvm_hdl_path_concat>& paths,
                            const std::string& kind ) const
{
  std::vector<uvm_hdl_path_concat> hdl_paths;

  std::string lkind = kind;

  if (kind.empty())
  {
     if (m_regfile_parent != NULL)
        lkind = m_regfile_parent->get_default_hdl_path();
     else
        lkind = m_parent->get_default_hdl_path();
  }

  if (!has_hdl_path(lkind))
  {
    UVM_ERROR("RegModel",
       "Register does not have HDL path defined for abstraction '" + lkind + "'");
    return;
  }

  if (m_hdl_paths_pool.find(lkind) != m_hdl_paths_pool.end())
    paths = m_hdl_paths_pool.find(lkind)->second;
}

//----------------------------------------------------------------------
// member function:  get_hdl_path_kinds
//
//! Get design abstractions for which HDL paths have been defined
//----------------------------------------------------------------------

void uvm_reg::get_hdl_path_kinds( std::vector<std::string>& kinds ) const
{
  std::string kind;
  kinds.clear(); // delete all elements

  if (m_hdl_paths_pool.size() == 0)
    return;

  for ( m_hdl_paths_pool_itT it = m_hdl_paths_pool.begin(); it != m_hdl_paths_pool.end(); it++)
  {
    kind = it->first;
    kinds.push_back(kind);
  }
}

//----------------------------------------------------------------------
// member function:  get_full_hdl_path
//
//! Get the full hierarchical HDL path(s)
//!
//! Returns the full hierarchical HDL path(s) defined for the specified
//! design abstraction in the register instance.
//! There may be more than one path returned even
//! if only one path was defined for the register instance, if any of the
//! parent components have more than one path defined for the same design
//! abstraction
//!
//! If no design abstraction is specified, the default design abstraction
//! for each ancestor block is used to get each incremental path.
//----------------------------------------------------------------------

void uvm_reg::get_full_hdl_path( std::vector<uvm_hdl_path_concat>& paths,
                                 const std::string& kind,
                                 const std::string& separator ) const
{
  std::string lkind = kind;

  if (kind.empty())
  {
    if (m_regfile_parent != NULL)
      lkind = m_regfile_parent->get_default_hdl_path();
    else
      lkind = m_parent->get_default_hdl_path();
  }

  if (!has_hdl_path(lkind))
  {
    UVM_ERROR("RegModel",
       "Register " + get_full_name() +
       " does not have HDL path defined for abstraction '"  + lkind + "'");
    return;
  }

  std::vector<uvm_hdl_path_concat> hdl_paths;

  if (m_hdl_paths_pool.find(lkind) != m_hdl_paths_pool.end())
    hdl_paths = m_hdl_paths_pool.find(lkind)->second;

  std::vector<std::string> parent_paths;

  if (m_regfile_parent != NULL)
    m_regfile_parent->get_full_hdl_path(parent_paths, lkind, separator);
  else
    m_parent->get_full_hdl_path(parent_paths, lkind, separator);

  for(unsigned int i = 0; i < hdl_paths.size(); i++)
  {
    uvm_hdl_path_concat hdl_concat = hdl_paths[i];

    for( unsigned int j = 0; j < parent_paths.size(); j++ )
    {
      uvm_hdl_path_concat t;

      for( unsigned int k = 0; k < hdl_concat.slices.size(); k++)
      {
        if (hdl_concat.slices[k].path.empty())
          t.add_path(parent_paths[j]);
        else
          t.add_path( parent_paths[j] + separator + hdl_concat.slices[k].path,
              hdl_concat.slices[k].offset,
              hdl_concat.slices[k].size);
      }
      paths.push_back(t);
    }
  }
}

//----------------------------------------------------------------------
// member function: backdoor_read
//
//! User-define backdoor read access
//!
//! Override the default string-based DPI backdoor access read
//! for this register type.
//! By default calls uvm_reg::backdoor_read_func().
//----------------------------------------------------------------------

void uvm_reg::backdoor_read( uvm_reg_item* rw )
{
  rw->status = backdoor_read_func(rw);
}

//----------------------------------------------------------------------
// member function: backdoor_write
//
//! User-defined backdoor read access
//!
//! Override the default string-based DPI backdoor access write
//! for this register type.
//----------------------------------------------------------------------

void uvm_reg::backdoor_write( uvm_reg_item* rw )
{
  /* TODO backdoor write
  std::vector<uvm_hdl_path_concat> paths;
  std::vector<uvm_hdl_path_concat>::iterator paths_itT;

  bool ok = true;

  get_full_hdl_path(paths, rw->bd_kind);

  for( paths_itT it = paths.begin(); it != paths.end(); it++ )
  {
    uvm_hdl_path_concat hdl_concat = (*it);

    for( int j = 0; j < hdl_concat.slices.size(); j++)
    {
      UVM_INFO("RegMem", "backdoor_write to " +
          hdl_concat.slices[j].path ,UVM_DEBUG)

          if (hdl_concat.slices[j].offset < 0)
          {
            ok &= uvm_hdl_deposit(hdl_concat.slices[j].path,rw.value[0]);
            continue;
          }

      uvm_reg_data_t slice;
      slice = rw.value[0] >> hdl_concat.slices[j].offset;
      slice &= (1 << hdl_concat.slices[j].size)-1;
      ok &= uvm_hdl_deposit(hdl_concat.slices[j].path, slice);

    }
  }
  rw->status = (ok ? UVM_IS_OK : UVM_NOT_OK);
  */
}

//----------------------------------------------------------------------
// member function: backdoor_read_func
//
//! User-defined backdoor read access
//!
//! Override the default string-based DPI backdoor access read
//! for this register type.
//----------------------------------------------------------------------

uvm_status_e uvm_reg::backdoor_read_func( uvm_reg_item* rw )
{
  /* TODO backdoor read func

  std::vector<uvm_hdl_path_concat> paths;
  uvm_reg_data_t val;

  bool ok = true;

  get_full_hdl_path(paths, rw->bd_kind);

  for( int i = 0; i < paths.size(); i++ )
  {
    uvm_hdl_path_concat hdl_concat = paths[i];

    val = 0;

    foreach (hdl_concat.slices[j])
    {
      std::ostringstream str;
      str << "backdoor_read from "
          << hdl_concat.slices[j].path;
      UVM_INFO("RegMem", str.str(), UVM_DEBUG);

      if (hdl_concat.slices[j].offset < 0)
      {
        ok &= uvm_hdl_read(hdl_concat.slices[j].path,val);
        continue;
      }

      uvm_reg_data_t slice;
      int k = hdl_concat.slices[j].offset;

      ok &= uvm_hdl_read(hdl_concat.slices[j].path, slice);

      for (int s = 0; s < hdl_concat.slices[j].size; s++)
      {
        val[k++] = slice[0];
        slice >>= 1;
      }
    }

    val &= (1 << m_n_bits)-1;

    if (i == 0)
      rw->value[0] = val;

    if (val != rw->value[0])
    {
      std::ostringstream str;
      str << "Backdoor read of register "
          << get_full_name()
          << " with multiple HDL copies: values are not the same: "
          << rw->value[0]
                       << " at path '"
                       << uvm_hdl_concat2string(paths[0])
                       << "', and "
                       << val
                       << " at path '"
                       << uvm_hdl_concat2string(paths[i])
                       << "'. Returning first value.";
      UVM_ERROR("RegModel", str.str());
      return UVM_NOT_OK;
    }
    std::ostringstream str;
    str << "returned backdoor value 0x"
        << std::hex << rw->value[0].to_uint64();
    UVM_INFO("RegMem", str.str(), UVM_DEBUG);
  }

  rw->status = (ok) ? UVM_IS_OK : UVM_NOT_OK;
  return rw->status;
  */

  return UVM_NOT_OK; // dummy only
}

//----------------------------------------------------------------------
// member function: backdoor_watch
//
//! User-defined DUT register change monitor
//!
//! Watch the DUT register corresponding to this abstraction class
//! instance for any change in value and return when a value-change occurs.
//! This may be implemented a string-based DPI access if the simulation
//! tool provide a value-change callback facility. Such a facility does
//! not exists in the standard SystemVerilog DPI and thus no
//! default implementation for this method can be provided.
//----------------------------------------------------------------------

void uvm_reg::backdoor_watch()
{}


//----------------------------------------------------------------------
// Group: Coverage
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: include_coverage
//
//! Specify which coverage model that must be included in
//! various block, register or memory abstraction class instances.
//!
//! The coverage models are specified by or'ing or adding the
//! #uvm_coverage_model_e coverage model identifiers corresponding to the
//! coverage model to be included.
//!
//! The scope specifies a hierarchical name or pattern identifying
//! a block, memory or register abstraction class instances.
//! Any block, memory or register whose full hierarchical name
//! matches the specified scope will have the specified functional
//! coverage models included in them.
//!
//! The scope can be specified as a POSIX regular expression
//! or simple pattern.
//! See #uvm_resource_base Scope Interface for more details.
//!
//!| uvm_reg::include_coverage("*", UVM_CVR_ALL);
//!
//! The specification of which coverage model to include in
//! which abstraction class is stored in a #uvm_reg_cvr_t resource in the
//! #uvm_resource_db resource database,
//! in the "uvm_reg::" scope namespace.
//----------------------------------------------------------------------

void uvm_reg::include_coverage( const std::string& scope,
                                uvm_reg_cvr_t models,
                                uvm_object* accessor )
{
  /* TODO support of regular data in resource database
  uvm_reg_cvr_rsrc_db::set( "uvm_reg::"+ scope,
                            "include_coverage",
                            models, accessor);
                            */
}

//----------------------------------------------------------------------
// member function: build_coverage
//
//! Check if all of the specified coverage models must be built.
//!
//! Check which of the specified coverage model must be built
//! in this instance of the register abstraction class,
//! as specified by calls to uvm_reg::include_coverage().
//!
//! Models are specified by adding the symbolic value of individual
//! coverage model as defined in #uvm_coverage_model_e.
//! Returns the sum of all coverage models to be built in the
//! register model.
//----------------------------------------------------------------------

uvm_reg_cvr_t uvm_reg::build_coverage( uvm_reg_cvr_t models )
{
  uvm_reg_cvr_t coverage = UVM_NO_COVERAGE;

  uvm_reg_cvr_rsrc_db::read_by_name("uvm_reg::" + get_full_name(),
                                    "include_coverage",
                                    coverage, this);
  return coverage & models;
}


//----------------------------------------------------------------------
// member function: add_coverage
//
//! Specify that additional coverage models are available.
//!
//! Add the specified coverage model to the coverage models
//! available in this class.
//! Models are specified by adding the symbolic value of individual
//! coverage model as defined in #uvm_coverage_model_e.
//!
//! This method shall be called only in the constructor of
//! subsequently derived classes.
//----------------------------------------------------------------------

void uvm_reg::add_coverage( uvm_reg_cvr_t models )
{
   m_has_cover |= models;
}

//----------------------------------------------------------------------
// member function: has_coverage
//
//! Check if register has coverage model(s)
//!
//! Returns TRUE if the register abstraction class contains a coverage model
//! for all of the models specified.
//! Models are specified by adding the symbolic value of individual
//! coverage model as defined in #uvm_coverage_model_e.
//----------------------------------------------------------------------

bool uvm_reg::has_coverage( uvm_reg_cvr_t models ) const
{
   return ((m_has_cover & models) == models);
}

//----------------------------------------------------------------------
// member function: set_coverage
//
//! Turns on coverage measurement.
//!
//! Turns the collection of functional coverage measurements on or off
//! for this register.
//! The functional coverage measurement is turned on for every
//! coverage model specified using #uvm_coverage_model_e symbolic
//! identifiers.
//! Multiple functional coverage models can be specified by adding
//! the functional coverage model identifiers.
//! All other functional coverage models are turned off.
//! Returns the sum of all functional
//! coverage models whose measurements were previously on.
//!
//! This method can only control the measurement of functional
//! coverage models that are present in the register abstraction classes,
//! then enabled during construction.
//! See the uvm_reg::has_coverage() method to identify
//! the available functional coverage models.
//----------------------------------------------------------------------

uvm_reg_cvr_t uvm_reg::set_coverage( uvm_reg_cvr_t is_on )
{
  if (is_on == UVM_NO_COVERAGE)
  {
    m_cover_on = is_on;
    return m_cover_on;
  }

  m_cover_on = m_has_cover & is_on;

  return m_cover_on;
}

//----------------------------------------------------------------------
// member function: get_coverage
//
//! Check if coverage measurement is on.
//!
//! Returns true if measurement for all of the specified functional
//! coverage models are currently on.
//! Multiple functional coverage models can be specified by adding the
//! functional coverage model identifiers.
//!
//! See uvm_reg::set_coverage() for more details.
//----------------------------------------------------------------------

bool uvm_reg::get_coverage( uvm_reg_cvr_t is_on ) const
{
  if (has_coverage(is_on) == 0)
    return false;

  return ((m_cover_on & is_on) == is_on);
}


//----------------------------------------------------------------------
// member function: sample
//
//! Functional coverage measurement method
//!
//! This method is invoked by the register abstraction class
//! whenever it is read or written with the specified ~data~
//! via the specified address \p map.
//! It is invoked after the read or write operation has completed
//! but before the mirror has been updated.
//!
//! Empty by default, this method may be extended by the
//! abstraction class generator to perform the required sampling
//! in any provided functional coverage model.
//----------------------------------------------------------------------

void uvm_reg::sample( uvm_reg_data_t data,
                      uvm_reg_data_t byte_en,
                      bool is_read,
                      uvm_reg_map* map )
{}

//----------------------------------------------------------------------
// member function: sample_values
//
//! Functional coverage measurement method for field values
//!
//! This method is invoked by the user
//! or by the uvm_reg_block::sample_values() method of the parent block
//! to trigger the sampling
//! of the current field values in the
//! register-level functional coverage model.
//!
//! This method may be extended by the
//! abstraction class generator to perform the required sampling
//! in any provided field-value functional coverage model.
//----------------------------------------------------------------------

void uvm_reg::sample_values()
{}

//----------------------------------------------------------------------
// Group: Callbacks
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: pre_write
//
//! Called before register write.
//!
//! If the specified data value, access \p path or address \p map are modified,
//! the updated data value, access path or address map will be used
//! to perform the register operation.
//! If the \p status is modified to anything other than UVM_IS_OK,
//! the operation is aborted.
//!
//! The registered callback methods are invoked after the invocation
//! of this method.
//! All register callbacks are executed before the corresponding
//! field callbacks
//----------------------------------------------------------------------

void uvm_reg::pre_write( uvm_reg_item* rw )
{}


//----------------------------------------------------------------------
// member function: post_write
//
//! Called after register write.
//!
//! If the specified \p status is modified,
//! the updated status will be
//! returned by the register operation.
//!
//! The registered callback methods are invoked before the invocation
//! of this method.
//! All register callbacks are executed before the corresponding
//! field callbacks
//----------------------------------------------------------------------

void uvm_reg::post_write( uvm_reg_item* rw )
{}


//----------------------------------------------------------------------
// member function: pre_read
//
//! Called before register read.
//!
//! If the specified access \p path or address \p map are modified,
//! the updated access path or address map will be used to perform
//! the register operation.
//! If the \p status is modified to anything other than UVM_IS_OK,
//! the operation is aborted.
//!
//! The registered callback methods are invoked after the invocation
//! of this method.
//! All register callbacks are executed before the corresponding
//! field callbacks
//----------------------------------------------------------------------

void uvm_reg::pre_read( uvm_reg_item* rw )
{}


//----------------------------------------------------------------------
// member function: post_read
//
//! Called after register read.
//!
//! If the specified readback data or ~status~ is modified,
//! the updated readback data or status will be
//! returned by the register operation.
//!
//! The registered callback methods are invoked before the invocation
//! of this method.
//! All register callbacks are executed before the corresponding
//! field callbacks
//----------------------------------------------------------------------

void uvm_reg::post_read( uvm_reg_item* rw )
{}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------
// Implementation defined - Initialization
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: set_parent
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::set_parent( uvm_reg_block* blk_parent,
                          uvm_reg_file* regfile_parent )
{
  if (m_parent != NULL)
    // UVM-SV TODO: remove register from previous parent

  m_parent = blk_parent;
  m_regfile_parent = regfile_parent;
}

//----------------------------------------------------------------------
// member function: get_block
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_block* uvm_reg::get_block() const
{
  return m_parent;
}

//----------------------------------------------------------------------
// member function: add_field
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::add_field( uvm_reg_field* field )
{
  unsigned int offset;
  int idx;
  unsigned int i;

  if (m_locked)
  {
    UVM_ERROR("RegModel", "Cannot add field to locked register model");
    return;
  }

  if (field == NULL)
    UVM_FATAL("RegModel", "Attempting to register NULL field");

  // Store fields in LSB to MSB order
  offset = field->get_lsb_pos();

  idx = -1;
  i = 0;

  for( m_fields_itt it = m_fields.begin();
       it != m_fields.end();
       it++)
  {
    if (offset < (*it)->get_lsb_pos())
    {
      m_fields.insert(it, field);
      idx = (int)i;
      break;
    }
    i++;
  }

  if (idx < 0)
  {
    m_fields.push_back(field);
    idx = m_fields.size()-1;
  }

  m_n_used_bits += field->get_n_bits();

  // Check if there are too many fields in the register
  if (m_n_used_bits > m_n_bits)
  {
    std::ostringstream str;
    str << "Fields use more bits ("
        << m_n_used_bits
        << ") than available in register '"
        << get_name()
        << "' ("
        << m_n_bits
        << ")";
    UVM_ERROR("RegModel", str.str() );
  }

  // Check if there are overlapping fields
  if (idx > 0)
  {
    if (m_fields[idx-1]->get_lsb_pos() + m_fields[idx-1]->get_n_bits() > offset)
    {
      std::ostringstream str;
      str << "Field "
          << m_fields[idx-1]->get_name()
          << " overlaps field "
          << field->get_name()
          << "in register '"
          << get_name()
          << "'.";

      UVM_ERROR("RegModel", str.str() );
    }
  }

  if (idx < (int)m_fields.size()-1)
  {
    if (offset + field->get_n_bits() > m_fields[idx+1]->get_lsb_pos())
    {
      std::ostringstream str;
      str << "Field "
          << field->get_name()
          << " overlaps field "
          << m_fields[idx+1]->get_name()
          << "in register '"
          << get_name()
          << "'.";

      UVM_ERROR("RegModel", str.str() );
    }
  }
}

//----------------------------------------------------------------------
// member function: add_map
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::add_map( uvm_reg_map* map )
{
  m_maps[map] = true;
}

//----------------------------------------------------------------------
// member function: m_lock_model
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::m_lock_model()
{
  if (m_locked)
    return;
  m_locked = true;
}

//----------------------------------------------------------------------
// Implementation defined - Introspection
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function:  get_local_map
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_map* uvm_reg::get_local_map( const uvm_reg_map* map, const std::string& caller ) const
{

  if (map == NULL)
    return get_default_map();

  uvm_reg_map* lmap = const_cast<uvm_reg_map*>(map);

  if (m_maps.find(lmap) != m_maps.end()) // exists
    return lmap;

  for(m_maps_citt it = m_maps.begin(); it != m_maps.end(); it++)
  {
    uvm_reg_map* local_map = (*it).first;
    uvm_reg_map* parent_map = local_map->get_parent_map();

    while (parent_map != NULL)
    {
      if (parent_map == lmap)
        return local_map;
      parent_map = parent_map->get_parent_map();
    }
  }

  std::ostringstream str;
  str << "Register '"
      << get_full_name()
      << "' is not contained within map '"
      << lmap->get_full_name()
      << "'"
      << ( ( caller.empty() ) ? "" : " (called from " + caller + ")" );

  UVM_WARNING("RegModel", str.str() );

  return NULL;
}

//----------------------------------------------------------------------
// member function:  get_default_map
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_map* uvm_reg::get_default_map( const std::string& caller ) const
{
  // if reg is not associated with any map, return null
  if (m_maps.size() == 0)
  {
    std::ostringstream str;
    str << "Register '"
        << get_full_name()
        << "' is not registered with any map"
        << ( (caller.empty()) ? "": " (called from " + caller + ")" );
    UVM_WARNING("RegModel", str.str() );
    return NULL;
  }

  // if only one map, choose that
  if (m_maps.size() == 1)
  {
    uvm_reg_map* map;
    map = (*m_maps.begin()).first;
    return map;
  }

  // try to choose one based on default_map in parent blocks.
  for( m_maps_citt it = m_maps.begin(); it != m_maps.end(); it++ )
  {
    uvm_reg_map* map = (*it).first;
    uvm_reg_block* blk = map->get_parent();
    uvm_reg_map* default_map = blk->get_default_map();
    if (default_map != NULL)
    {
      uvm_reg_map* local_map = get_local_map(default_map, "get_default_map()");
      if (local_map != NULL)
        return local_map;
    }
  }

  // if that fails, choose the first in this reg's maps
  uvm_reg_map* map;
  map = (*m_maps.begin()).first;
  return map;
}


//----------------------------------------------------------------------
// member function: m_get_fields_access
//
// Implementation defined
//
// Returns "WO" if all of the fields in the registers are write-only
// Returns "RO" if all of the fields in the registers are read-only
// Returns "RW" otherwise.
//----------------------------------------------------------------------

std::string uvm_reg::m_get_fields_access( uvm_reg_map* map ) const
{
  bool is_R = false;
  bool is_W = false;

  for( unsigned int i = 0; i < m_fields.size(); i++ )
  {
    std::string row = m_fields[i]->get_access(map);

    if (row == "RO") is_R = true;
    else
      if (row == "RC") is_R = true;
      else
        if (row == "RS") is_R = true;
        else
          if (row == "WO") is_W = true;
          else
            if (row == "WOC") is_W = true;
            else
              if (row == "WOS") is_W = true;
              else
                if (row == "WO1") is_W = true;
                else
                  return "RW"; //default:

    if (is_R && is_W) return "RW";
  }

  if (is_R == false && is_W == true) return "WO";
  if (is_R == true && is_W == false) return "RO";

  return "RW";
}

//----------------------------------------------------------------------
// Implementation defined - Access
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: m_set_busy
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::m_set_busy( bool busy )
{
  m_is_busy = busy;
}


//----------------------------------------------------------------------
// member function: m_read
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::m_read( uvm_status_e& status,
                      uvm_reg_data_t& value,
                      uvm_path_e path,
                      uvm_reg_map* map,
                      uvm_sequence_base* parent,
                      int prior,
                      uvm_object* extension,
                      const std::string& fname,
                      int lineno )
{
   // create an abstract transaction for this operation
   uvm_reg_item* rw;
   rw = uvm_reg_item::type_id::create("read_item", NULL, get_full_name());

   // make sure we have reserved space to store an initial value
   if (rw->value.size() == 0)
     rw->value.resize(1);

   rw->element      = dynamic_cast<uvm_object*>(this);
   rw->element_kind = UVM_REG;
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
   status = rw->status;
   value = rw->value[0];
}


//----------------------------------------------------------------------
// member function: m_atomic_check_lock
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::m_atomic_check_lock( bool on )
{
  sc_core::sc_process_handle m_reg_process = sc_core::sc_get_current_process_handle();
  m_process_valid = m_reg_process.valid();

  if (on)
  {
    if (!m_process_valid) return; // handle (still) invalid, so return
    if (m_reg_process == m_process) return; // same process, also return
    m_atomic.lock(); // get
    m_process = m_reg_process;
  }
  else
  {
    // Maybe a key was put back in by a spurious call to reset()
    m_atomic.trylock(); // try_get
    m_atomic.unlock(); // put
    m_process_valid = false;
  }
}


//----------------------------------------------------------------------
// member function: m_check_access
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_reg::m_check_access( uvm_reg_item* rw,
                              uvm_reg_map_info*& map_info,
                              const std::string& caller )
{
  if (rw->path == UVM_DEFAULT_PATH)
    rw->path = m_parent->get_default_path();

  if (rw->path == UVM_BACKDOOR)
  {
    if (get_backdoor() == NULL && !has_hdl_path())
    {
      UVM_WARNING("RegModel",
          "No backdoor access available for register '" + get_full_name() +
          "' . Using frontdoor instead.");
          rw->path = UVM_FRONTDOOR;
    }
    else
      rw->map = uvm_reg_map::backdoor();
  }

  if (rw->path != UVM_BACKDOOR)
  {
    rw->local_map = get_local_map(rw->map, caller);

    if (rw->local_map == NULL)
    {
      UVM_ERROR(get_type_name(),
          "No transactor available to physically access register on map '" +
          rw->map->get_full_name() + "'");
          rw->status = UVM_NOT_OK;
      return false;
    }

    map_info = rw->local_map->get_reg_map_info(this);

    if (map_info->frontdoor == NULL && map_info->unmapped)
    {
      std::ostringstream str;
      str << "Register '"
          << get_full_name()
          << "' unmapped in map '"
          << ((rw->map==NULL) ? rw->local_map->get_full_name() : rw->map->get_full_name())
          << "' and does not have a user-defined frontdoor";
      UVM_ERROR("RegModel", str.str() );
      rw->status = UVM_NOT_OK;
      return false;
    }

    if (rw->map == NULL)
      rw->map = rw->local_map;
  }
  return true;
}

//----------------------------------------------------------------------
// member function: m_get_is_locked_by_field
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_reg::m_get_is_locked_by_field() const
{
  return m_is_locked_by_field;
}

//----------------------------------------------------------------------
// member function: do_check
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_reg::do_check( uvm_reg_data_t expected,
                        uvm_reg_data_t actual,
                        uvm_reg_map* map )
{
  uvm_reg_data_t dc = 0;

  for( unsigned int i = 0; i < m_fields.size(); i++ )
  {
    std::string acc = m_fields[i]->get_access(map);
    acc = acc.substr(0, 1);
    if (m_fields[i]->get_compare() == UVM_NO_CHECK ||
        acc == "WO")
    {
      dc |= ((1 << m_fields[i]->get_n_bits())-1)
                << m_fields[i]->get_lsb_pos();
    }
  }

  // TODO check below used  '===' so we need to find the SystemC equivalent
  if ((actual|dc) == (expected|dc)) return true;

  std::ostringstream str;
  str << "Register '"
      << get_full_name()
      << "' value read from DUT (0x"
      << std::hex << actual.to_uint64()
      << ") does not match mirrored value (0x"
      << std::hex << expected.to_uint64()
      << ")";
  UVM_ERROR("RegModel", str.str() );

  for( unsigned int i = 0; i < m_fields.size(); i++ )
  {
    std::string acc = m_fields[i]->get_access(map);
    acc = acc.substr(0, 1);
    if (!(m_fields[i]->get_compare() == UVM_NO_CHECK || acc == "WO"))
    {
      uvm_reg_data_t mask = ((1 << m_fields[i]->get_n_bits())-1);
      uvm_reg_data_t val  = actual   >> m_fields[i]->get_lsb_pos() & mask;
      uvm_reg_data_t exp  = expected >> m_fields[i]->get_lsb_pos() & mask;

      if (val != exp) // TODO check was '!==' replaced by !=
      {
        std::ostringstream str;
        str << "Field "
            << m_fields[i]->get_name()
            << " ("
            << get_full_name()
            << "["
            << m_fields[i]->get_lsb_pos() + m_fields[i]->get_n_bits() - 1
            << ":"
            << m_fields[i]->get_lsb_pos()
            << "]) mismatch read = 0x"
            << std::hex << val.to_uint64()
            << "(size: "
            << std::dec << m_fields[i]->get_n_bits()
            << ") mirrored = 0x"
            << std::hex << exp.to_uint64()
            << "(size: "
            << std::dec << m_fields[i]->get_n_bits()
            << ")";
        UVM_INFO("RegModel", str.str(), UVM_NONE);
      }
    }
  }

  return false;
}


//----------------------------------------------------------------------
// member function: do_write
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::do_write( uvm_reg_item* rw )
{
  uvm_reg_cb_iter* cbs = new uvm_reg_cb_iter(this);
  uvm_reg_map_info* map_info = NULL;
  uvm_reg_data_t value;

  m_fname  = rw->fname;
  m_lineno = rw->lineno;

  if (!m_check_access(rw, map_info, "write()") )
    return;

  m_atomic_check_lock(true);

  m_write_in_progress = true;

  rw->value[0] &= uvm_mask_size(m_n_bits);
  value = rw->value[0];

  rw->status = UVM_IS_OK;

  // pre-write cbs - fields
  {
    uvm_reg_data_t msk, msk_init;
    int lsb;

    for( unsigned int i = 0; i < m_fields.size(); i++ )
    {
      uvm_reg_field_cb_iter* cbsf = new uvm_reg_field_cb_iter(m_fields[i]);
      uvm_reg_field* f = m_fields[i];
      lsb = f->get_lsb_pos();

      msk_init = (1<<f->get_n_bits())-1;
      msk = msk_init << lsb;

      rw->value[0] = (value & msk) >> lsb;
      f->pre_write(rw);

      for( uvm_reg_cbs* cb = cbsf->first(); cb != NULL; cb = cbsf->next() )
      {
        rw->element = f;
        rw->element_kind = UVM_FIELD;
        cb->pre_write(rw);
      }
      value = (value & ~msk) | (rw->value[0] << lsb);
      delete cbsf;
    }
  }
  // end pre-write cbs

  rw->element = this;
  rw->element_kind = UVM_REG;
  rw->value[0] = value;

  // pre-write cbs - reg
  pre_write(rw);

  for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next() )
    cb->pre_write(rw);

  if (rw->status != UVM_IS_OK)
  {
    m_write_in_progress = false;
    m_atomic_check_lock(false);
    return;
  }

  // execute write...
  switch (rw->path)
  {
    case UVM_BACKDOOR:  // via user backdoor
    {
      uvm_reg_data_t final_val;
      uvm_reg_backdoor* bkdr = get_backdoor();

      value = rw->value[0];

      // Mimick the final value after a physical read
     rw->access_kind = UVM_READ;
      if (bkdr != NULL)
        bkdr->read(rw);
      else
        backdoor_read(rw);

      if (rw->status == UVM_NOT_OK)
      {
        m_write_in_progress = false;
        return;
      }

      for( unsigned int i = 0; i < m_fields.size(); i++)
      {
        uvm_reg_data_t field_val;
        int lsb = m_fields[i]->get_lsb_pos();
        int sz  = m_fields[i]->get_n_bits();
        field_val = m_fields[i]->m_predict( (rw->value[0] >> lsb) & uvm_mask_size(sz),
            (value >> lsb) & uvm_mask_size(sz), rw->local_map);
        final_val |= field_val << lsb;
      }

      rw->access_kind = UVM_WRITE;
      rw->value[0] = final_val;

      if (bkdr != NULL)
        bkdr->write(rw);
      else
        backdoor_write(rw);

      do_predict(rw, UVM_PREDICT_WRITE);
      break;
    } // case UVM_BACKDOOR

    case UVM_FRONTDOOR:
    {
      uvm_reg_map* system_map = rw->local_map->get_root_map();

      m_is_busy = true;

      // via user frontdoor
      if (map_info->frontdoor != NULL)
      {
        uvm_reg_frontdoor* fd = map_info->frontdoor;
        fd->rw_info = rw;
        if (fd->sequencer == NULL)
          fd->sequencer = system_map->get_sequencer();
        fd->start(fd->sequencer, rw->parent);
      }

      // via built-in frontdoor
      else
        rw->local_map->do_write(rw);

      m_is_busy = false;

      if (system_map->get_auto_predict())
      {
        uvm_status_e status;
        if (rw->status != UVM_NOT_OK)
        {
          sample(value, -1, 0, rw->map);
          m_parent->m_sample(map_info->offset, 0, rw->map);
        }

        status = rw->status; // do_predict will override rw.status, so we save it here
        do_predict(rw, UVM_PREDICT_WRITE);
        rw->status = status;
      }
      break;
    } // case UVM_FRONTDOOR
    default: break; // no default?
  } //switch

  value = rw->value[0];

  // post-write cbs - reg

  for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next() )
    cb->post_write(rw);

  post_write(rw);

  // post-write cbs - fields
  for( unsigned int i = 0; i < m_fields.size(); i++)
  {
    uvm_reg_field_cb_iter* cbsf = new uvm_reg_field_cb_iter(m_fields[i]);
    uvm_reg_field* f = m_fields[i];

    rw->element = f;
    rw->element_kind = UVM_FIELD;
    rw->value[0] = (value >> f->get_lsb_pos()) & ((1<<f->get_n_bits())-1);

    for( uvm_reg_cbs* cb = cbsf->first(); cb != NULL; cb = cbsf->next() )
      cb->post_write(rw);

    f->post_write(rw);

    delete cbsf;
  }

  rw->value[0] = value;
  rw->element = this;
  rw->element_kind = UVM_REG;

  // REPORT
  if(uvm_report_enabled(UVM_HIGH, UVM_INFO, "RegModel"))
  {
    std::string path_s;
    std::ostringstream value_s;
    std::ostringstream str;

    if (rw->path == UVM_FRONTDOOR)
      path_s = (map_info->frontdoor != NULL) ? "user frontdoor" :
          "map " + rw->map->get_full_name();
    else
      path_s = (get_backdoor() != NULL) ? "user backdoor" : "DPI backdoor";

    value_s << " = 0x" << std::hex << rw->value[0].to_uint64();

    str << "Wrote register via "
        << path_s
        << ": "
        << get_full_name()
        << value_s.str();

    uvm_report_info("RegModel", str.str(), UVM_HIGH);
  }

  m_write_in_progress = false;
  m_atomic_check_lock(false);

  delete cbs;
}


//----------------------------------------------------------------------
// member function: do_read
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::do_read( uvm_reg_item* rw )
{
  uvm_reg_cb_iter*  cbs = new uvm_reg_cb_iter(this);
  uvm_reg_map_info* map_info = NULL;
  uvm_reg_data_t    value;
  uvm_reg_data_t    exp;

  m_fname   = rw->fname;
  m_lineno  = rw->lineno;

  if (!m_check_access(rw, map_info, "read()"))
    return;

  m_read_in_progress = true;

  rw->status = UVM_IS_OK;

  // pre-read cbs - fields
  for( unsigned int i = 0; i < m_fields.size(); i++)
  {
    uvm_reg_field_cb_iter* cbsf = new uvm_reg_field_cb_iter(m_fields[i]);
    uvm_reg_field* f = m_fields[i];
    rw->element = f;
    rw->element_kind = UVM_FIELD;
    m_fields[i]->pre_read(rw);

    for( uvm_reg_cbs* cb = cbsf->first(); cb != NULL; cb = cbsf->next() )
      cb->pre_read(rw);

    delete cbsf;
  }

  rw->element = this;
  rw->element_kind = UVM_REG;

  // pre-read cbs - reg
  pre_read(rw);

  for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next() )
    cb->pre_read(rw);

  if (rw->status != UVM_IS_OK)
  {
    m_read_in_progress = false;
    return;
  }

  // execute read...
  switch(rw->path)
  {
    case UVM_BACKDOOR: // ...VIA USER BACKDOOR
    {
      uvm_reg_backdoor* bkdr = get_backdoor();

      uvm_reg_map* map = uvm_reg_map::backdoor();
      if (map->get_check_on_read())
        exp = get();

      if (bkdr != NULL)
        bkdr->read(rw);
      else
        backdoor_read(rw);

      value = rw->value[0];

      // Need to clear RC fields, set RS fields and mask WO fields
      if (rw->status != UVM_NOT_OK)
      {
        uvm_reg_data_t wo_mask;

        for( unsigned int i = 0; i < m_fields.size(); i++)
        {
          std::string acc = m_fields[i]->get_access(uvm_reg_map::backdoor());
          if (acc == "RC" ||
              acc == "WRC" ||
              acc == "WSRC" ||
              acc == "W1SRC" ||
              acc == "W0SRC")
          {
            value &= ~(((1<<m_fields[i]->get_n_bits())-1)
                << m_fields[i]->get_lsb_pos());
          }
          else
            if (acc == "RS" ||
                acc == "WRS" ||
                acc == "WCRS" ||
                acc == "W1CRS" ||
                acc == "W0CRS")
          {
            value |= (((1<<m_fields[i]->get_n_bits())-1)
                << m_fields[i]->get_lsb_pos());
          }
          else
            if (acc == "WO" ||
                acc == "WOC" ||
                acc == "WOS" ||
                acc == "WO1")
            {
              wo_mask |= ((1<<m_fields[i]->get_n_bits())-1)
                << m_fields[i]->get_lsb_pos();
            }
        }

        if (value != rw->value[0])
        {
          uvm_reg_data_t saved;
          saved = rw->value[0];
          rw->value[0] = value;
          if (bkdr != NULL)
            bkdr->write(rw);
          else
            backdoor_write(rw);
          rw->value[0] = saved;
        }

        rw->value[0] &= ~wo_mask;

        if (map->get_check_on_read() &&
            rw->status != UVM_NOT_OK)
        {
          do_check(exp, rw->value[0], map);
        }

        do_predict(rw, UVM_PREDICT_READ);
      }
      break;
    } // case UVM_BACKDOOR

    case UVM_FRONTDOOR:
    {
      uvm_reg_map* system_map = rw->local_map->get_root_map();

      m_is_busy = true;

      if (rw->local_map->get_check_on_read())
        exp = get();

      // ...VIA USER FRONTDOOR
      if (map_info->frontdoor != NULL)
      {
        uvm_reg_frontdoor* fd = map_info->frontdoor;
        fd->rw_info = rw;
        if (fd->sequencer == NULL)
          fd->sequencer = system_map->get_sequencer();
        fd->start(fd->sequencer, rw->parent);

      }
      // ...VIA BUILT-IN FRONTDOOR
      else
        rw->local_map->do_read(rw);

      m_is_busy = false;

      if (system_map->get_auto_predict())
      {
        uvm_status_e status;
        if (rw->local_map->get_check_on_read() &&
            rw->status != UVM_NOT_OK)
        {
          do_check(exp, rw->value[0], system_map);
        }

        if (rw->status != UVM_NOT_OK)
        {
          sample(rw->value[0], -1, 1, rw->map);
          m_parent->m_sample(map_info->offset, 1, rw->map);
        }

        status = rw->status; // do_predict will override rw.status, so we save it here
        do_predict(rw, UVM_PREDICT_READ);
        rw->status = status;
      }
      break;
    } // case UVM_FRONTDOOR

    default: break; // TODO no default?

  } // switch

  value = rw->value[0]; // preserve

  // POST-READ CBS - REG

  for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next())
    cb->post_read(rw);

  post_read(rw);

  // POST-READ CBS - FIELDS
  for( unsigned int i = 0; i < m_fields.size(); i++)
  {
    uvm_reg_field_cb_iter* cbsf = new uvm_reg_field_cb_iter(m_fields[i]);
    uvm_reg_field* f = m_fields[i];

    rw->element = f;
    rw->element_kind = UVM_FIELD;
    rw->value[0] = (value >> f->get_lsb_pos()) & ((1<<f->get_n_bits())-1);

    for( uvm_reg_cbs* cb = cbsf->first(); cb != NULL; cb = cbsf->next() )
      cb->post_read(rw);

    f->post_read(rw);

    delete cbsf;
  }

  rw->value[0] = value; // restore
  rw->element = this;
  rw->element_kind = UVM_REG;

  // REPORT
  if (uvm_report_enabled( UVM_HIGH, UVM_INFO, "RegModel" ) )
  {
    std::string path_s;
    std::ostringstream value_s;
    std::ostringstream str;

    if (rw->path == UVM_FRONTDOOR)
      path_s = (map_info->frontdoor != NULL) ? "user frontdoor" :
        "map " + rw->map->get_full_name();
    else
      path_s = (get_backdoor() != NULL) ? "user backdoor" : "DPI backdoor";

    value_s << " = 0x" << std::hex << rw->value[0].to_uint64();

    str << "Read register via "
        << path_s
        << ": "
        << get_full_name()
        << value_s.str();
    uvm_report_info("RegModel", str.str(), UVM_HIGH);
  }

  m_read_in_progress = false;

  delete cbs;
}


//----------------------------------------------------------------------
// member function: do_predict
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::do_predict( uvm_reg_item* rw,
                          uvm_predict_e kind,
                          uvm_reg_byte_en_t be)
{
   uvm_reg_data_t reg_value = rw->value[0];
   m_fname = rw->fname;
   m_lineno = rw->lineno;

   rw->status = UVM_IS_OK;

   if (m_is_busy && kind == UVM_PREDICT_DIRECT)
   {
      UVM_WARNING("RegModel", "Trying to predict value of register '" +
                  get_full_name() + "' while it is being accessed");
      rw->status = UVM_NOT_OK;
      return;
   }

   for( unsigned int i = 0; i < m_fields.size(); i++ )
   {
      rw->value[0] = (reg_value >> m_fields[i]->get_lsb_pos()) &
                                 ((1 << m_fields[i]->get_n_bits())-1);
      m_fields[i]->do_predict(rw, kind, be>>(m_fields[i]->get_lsb_pos()/8));
   }

   rw->value[0] = reg_value;
}

//----------------------------------------------------------------------
// Implementation defined - Backdoor
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: m_sample
//
// Implementation defined
//----------------------------------------------------------------------

void  uvm_reg::m_sample( uvm_reg_data_t  data,
                         uvm_reg_data_t  byte_en,
                         bool            is_read,
                         uvm_reg_map*    map )
{
  sample(data, byte_en, is_read, map);
}

//----------------------------------------------------------------------
// Implementation defined - member functions for UVM object
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: do_print
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::do_print( const uvm_printer& printer ) const
{
  std::vector<uvm_reg_field*> f;
  uvm_object::do_print(printer);
  get_fields(f);
  for( unsigned int i = 0; i < f.size(); i++)
    printer.print_generic(f[i]->get_name(), f[i]->get_type_name(), -2, f[i]->convert2string());
}


//----------------------------------------------------------------------
// member function: convert2string
//
// Implementation defined
//----------------------------------------------------------------------

std::string uvm_reg::convert2string() const
{
   std::ostringstream res_str;
   std::ostringstream str;

   std::string prefix;

   str << "Register "
       << get_full_name()
       << " -- "
       << get_n_bytes()
       << " bytes, mirror value: 0x"
       << std::hex << get().to_uint64();

   if (m_maps.size() == 0)
     str << " (unmapped)" << std::endl;
   else
     str << std::endl;

   for( m_maps_citt it = m_maps.begin(); it != m_maps.end(); it++ )
   {
     uvm_reg_map* parent_map = (*it).first;
     unsigned int offset;

     while (parent_map != NULL)
     {
       uvm_reg_map* this_map = parent_map;
       parent_map = this_map->get_parent_map();
       offset = (parent_map == NULL) ? this_map->get_base_addr(UVM_NO_HIER) :
                                       parent_map->get_submap_offset(this_map);
       prefix += "  ";

            uvm_endianness_e e = this_map->get_endian();

            str << prefix
                << "Mapped in '"
                << this_map->get_full_name()
                << "' -- "
                << this_map->get_n_bytes()
                << " bytes, "
                << uvm_endianness_name[e]
                << ", offset 0x"
                << std::hex << offset
                << std::endl;
      }
   }
   prefix = "  ";

   for( unsigned int i = 0; i < m_fields.size(); i++ )
      str << std::endl <<  m_fields[i]->convert2string();

   if (m_read_in_progress)
   {
      if(!m_fname.empty() && m_lineno != 0)
         res_str << m_fname << ":" << m_lineno;

      str << std::endl << res_str.str() << "currently executing read method";
   }

   if (m_write_in_progress)
   {
      if (!m_fname.empty() && m_lineno != 0)
         res_str << m_fname << ":" << m_lineno;

      str << std::endl << res_str.str() << "currently executing write method";
   }

   return str.str();
}


//----------------------------------------------------------------------
// member function: clone
//
// Implementation defined
//----------------------------------------------------------------------

uvm_object* uvm_reg::clone()
{
  UVM_FATAL("RegModel","RegModel registers cannot be cloned");
  return NULL;
}

//----------------------------------------------------------------------
// member function: do_copy
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::do_copy( const uvm_object& rhs )
{
  UVM_FATAL("RegModel","RegModel registers cannot be copied");
  // TODO - implement private copy constructor
}


//----------------------------------------------------------------------
// member function: do_compare
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_reg::do_compare( const uvm_object& rhs,
                          const uvm_comparer* comparer) const
{
  UVM_WARNING("RegModel","RegModel registers cannot be compared");
  return false;
}

//----------------------------------------------------------------------
// member function: do_pack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::do_pack( uvm_packer& packer ) const
{
  UVM_WARNING("RegModel","RegModel registers cannot be packed");
}

//----------------------------------------------------------------------
// member function: do_unpack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg::do_unpack( uvm_packer& packer )
{
  UVM_WARNING("RegModel","RegModel registers cannot be unpacked");
}

} // namespace uvm
