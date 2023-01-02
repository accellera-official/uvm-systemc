//----------------------------------------------------------------------
//   Copyright 2013-2021 NXP B.V.
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

#include "uvmsc/reg/uvm_mem.h"
#include "uvmsc/reg/uvm_mem_mam.h"
#include "uvmsc/reg/uvm_mem_mam_cfg.h"
#include "uvmsc/reg/uvm_reg_item.h"
#include "uvmsc/reg/uvm_reg_block.h"
#include "uvmsc/reg/uvm_reg_map.h"
#include "uvmsc/reg/uvm_reg_frontdoor.h"
#include "uvmsc/reg/uvm_reg_backdoor.h"
#include "uvmsc/reg/uvm_vreg.h"
#include "uvmsc/reg/uvm_vreg_field.h"
#include "uvmsc/conf/uvm_object_string_pool.h"
#include "uvmsc/dpi/uvm_hdl.h" // for backdoor access

namespace uvm {

//------------------------------------------------------------------------------
// static data member initialization
//------------------------------------------------------------------------------

int unsigned uvm_mem::m_max_size = 0;

//----------------------------------------------------------------------
// Constructor
//
//! Create a new instance and type-specific configuration
//!
//! Creates an instance of a memory abstraction class with the specified
//! name.
//!
//! \p size specifies the total number of memory locations.
//! \p n_bits specifies the total number of bits in each memory location.
//! \p access specifies the access policy of this memory and may be
//! one of "RW for RAMs and "RO" for ROMs.
//!
//! \p has_coverage specifies which functional coverage models are present in
//! the extension of the register abstraction class.
//! Multiple functional coverage models may be specified by adding their
//! symbolic names, as defined by the uvm_coverage_model_e type.
//----------------------------------------------------------------------

uvm_mem::uvm_mem( const std::string& name,
                  unsigned long size,
                  unsigned int n_bits,
                  const std::string& access,
                  int has_coverage ) : uvm_object(name)
{
  m_locked = false;
  m_read_in_progress = false;
  m_write_in_progress = false;

  m_access    = uvm_toupper(access);
  m_size      = size;
  m_maps.clear();

  if (n_bits > m_max_size)
      m_max_size = n_bits;

  m_n_bits    = n_bits;

  if (m_n_bits == 0)
  {
    UVM_ERROR("RegModel", "Memory '" + get_full_name() + "' cannot have 0 bits. Size will be set to one bit." );
    n_bits = 1;
  }

  m_backdoor  = NULL;
  m_parent = NULL;

  m_is_powered_down = false;
  m_has_cover = has_coverage;

  m_fname = "";
  m_lineno = 0;
  m_vregs.clear();

  m_hdl_paths_pool.clear();
}

//----------------------------------------------------------------------
// Member function: configure
//
//! Instance-specific configuration
//!
//! Specify the parent block of this memory.
//!
//! If this memory is implemented in a single HDL variable,
//! it's name is specified as the \p hdl_path.
//! Otherwise, if the memory is implemented as a concatenation
//! of variables (usually one per bank), then the HDL path
//! must be specified using the add_hdl_path() or
//! add_hdl_path_slice() method.
//----------------------------------------------------------------------

void uvm_mem::configure( uvm_reg_block* parent,
                         const std::string& hdl_path )
{
  if (parent == NULL)
    UVM_FATAL("REG/NULL_PARENT", "configure: parent argument is null");

  m_parent = parent;

  if (m_access != "RW" && m_access != "RO")
  {
    UVM_ERROR("RegModel", "Memory '" + get_full_name() + "' can only be RW or RO");
      m_access = "RW";
  }

  uvm_mem_mam_cfg* cfg = new uvm_mem_mam_cfg();

  cfg->n_bytes      = ((m_n_bits-1) / 8) + 1;
  cfg->start_offset = 0;
  cfg->end_offset   = m_size-1;

  cfg->mode     = uvm_mem_mam::GREEDY;
  cfg->locality = uvm_mem_mam::BROAD;

  mam = new uvm_mem_mam(get_full_name(), cfg, this);

  m_parent->add_mem(this);

  if (hdl_path != "")
    add_hdl_path_slice(hdl_path, -1, -1);
}

//----------------------------------------------------------------------
// Member function: set_offset
//
//! Modify the offset of the memory
//!
//! The offset of a memory within an address map is set using the
//! uvm_reg_map::add_mem() method.
//! This method is used to modify that offset dynamically.
//!
//! Note: Modifying the offset of a memory will make the abstract model
//! diverge from the specification that was used to create it.
//----------------------------------------------------------------------

void uvm_mem::set_offset( uvm_reg_map* map,
                          uvm_reg_addr_t offset,
                          bool unmapped )
{
  if (m_maps.size() > 1 && map == NULL)
  {
    UVM_ERROR("RegModel", "set_offset requires a non-null map when memory '" +
      get_full_name() +"' belongs to more than one map.");
    return;
  }

  map = get_local_map(map,"set_offset()");

  if (map == NULL)
    return;

  map->m_set_mem_offset(this, offset, unmapped);
}

//----------------------------------------------------------------------
// Group: Introspection
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: get_name
//
// Get the simple name
//
//! Return the simple object name of this memory.
//----------------------------------------------------------------------

// inherited from base class

//----------------------------------------------------------------------
// Member function: get_full_name
//
//! Get the hierarchical name
//!
//! Return the hierarchical name of this memory.
//! The base of the hierarchical name is the root block.
//----------------------------------------------------------------------

const std::string uvm_mem::get_full_name() const
{
  if (m_parent == NULL)
    return get_name();

  return m_parent->get_full_name() + "." + get_name();
}

//----------------------------------------------------------------------
// Member function: get_parent
//
//! Get the parent block
//----------------------------------------------------------------------

uvm_reg_block* uvm_mem::get_parent() const
{
  return m_parent;
}
//----------------------------------------------------------------------
// Member function: get_n_maps
//
//! Returns the number of address maps this memory is mapped in
//----------------------------------------------------------------------

int uvm_mem::get_n_maps() const
{
   return (int)m_maps.size();
}

//----------------------------------------------------------------------
// Member function: is_in_map
//
//! Return TRUE if this memory is in the specified address /p map
//----------------------------------------------------------------------

bool uvm_mem::is_in_map( uvm_reg_map* map ) const
{
   if (m_maps.find(map) != m_maps.end() ) // exists
     return true;

   for( m_maps_itt it = m_maps.begin();
        it != m_maps.end();
        it++ )
   {
     uvm_reg_map* local_map= (*it).first;
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
// Member function: get_maps
//
//! Returns all of the address \p maps where this memory is mapped
//----------------------------------------------------------------------

void uvm_mem::get_maps( std::vector<uvm_reg_map*>& maps ) const
{
  for( m_maps_itt it = m_maps.begin(); it != m_maps.end(); it++)
     maps.push_back( (*it).first );
}

//----------------------------------------------------------------------
// Member function: get_rights
//
//! Returns the access rights of this memory.
//!
//! Returns "RW", "RO" or "WO".
//! The access rights of a memory is always "RW",
//! unless it is a shared memory
//! with access restriction in a particular address map.
//!
//! If no address map is specified and the memory is mapped in only one
//! address map, that address map is used. If the memory is mapped
//! in more than one address map, the default address map of the
//! parent block is used.
//!
//! If an address map is specified and
//! the memory is not mapped in the specified
//! address map, an error message is issued
//! and "RW" is returned.
//----------------------------------------------------------------------

std::string uvm_mem::get_rights( const uvm_reg_map* map ) const
{
   const uvm_reg_map_info* info;

   // No right restrictions if not shared
   if (m_maps.size() <= 1)
      return "RW";

   map = get_local_map(map, "get_rights()");

   if (map == NULL)
     return "RW";

   info = map->get_mem_map_info(this);
   return info->rights;
}

//----------------------------------------------------------------------
// Member function: get_access
//
//! Returns the access policy of the memory when written and read
//! via an address map.
//!
//! If the memory is mapped in more than one address map,
//! an address \p map must be specified.
//! If access restrictions are present when accessing a memory
//! through the specified address map, the access mode returned
//! takes the access restrictions into account.
//! For example, a read-write memory accessed
//! through a domain with read-only restrictions would return "RO".
//----------------------------------------------------------------------

std::string uvm_mem::get_access( const uvm_reg_map* map ) const
{
  std::string _access = m_access;
  std::string rights;

  if (get_n_maps() == 1)
    return _access;

  map = get_local_map(map, "get_access()");

  if (map == NULL)
    return _access;

  rights = get_rights(map);

  // Is the memory restricted in this map?
  if (rights == "RW")
    return _access; // No restrictions

  if (rights == "RO")
  {
    if( (_access == "RW") || (_access == "RO") )
      _access = "RO";
    else
    {
      if( _access == "WO" )
      {
        UVM_ERROR("RegModel", "WO memory '" + get_full_name() +
          "' restricted to RO in map '" + map->get_full_name() + "'");
      }
      else { // default
        UVM_ERROR("RegModel", "Memory '" + get_full_name() +
          "' has invalid access mode, '" + _access + "'");
      }
    }
  } // if "RO"
  else
  {
    if (rights == "WO")
    {
		if( (_access == "RW") || (_access == "WO") )
        _access = "WO";
      else
      {
        if( _access == "RO" )
        {
          UVM_ERROR("RegModel", "RO memory '" + get_full_name() +
            "' restricted to WO in map '" + map->get_full_name() + "'");
        }
        else
        {  // default
           UVM_ERROR("RegModel", "Memory '" + get_full_name() +
            "' has invalid access mode, '" + _access + "'");
        }
      }
    }
    else //  default other rights
      UVM_ERROR("RegModel", "Shared memory '" + get_full_name() +
         "' is not shared in map '" + map->get_full_name() + "'");
  }

  return _access;
}

//----------------------------------------------------------------------
// Member function: get_size
//
//! Returns the number of unique memory locations in this memory.
//----------------------------------------------------------------------

unsigned long uvm_mem::get_size() const
{
  return m_size;
}


//----------------------------------------------------------------------
// Member function: get_n_bytes
//
//! Return the width, in number of bytes, of each memory location
//----------------------------------------------------------------------

unsigned int uvm_mem::get_n_bytes() const
{
  return (m_n_bits - 1) / 8 + 1;
}


//----------------------------------------------------------------------
// Member function: get_n_bits
//
//! Returns the width, in number of bits, of each memory location
//----------------------------------------------------------------------

unsigned int uvm_mem::get_n_bits() const
{
  return m_n_bits;
}
//----------------------------------------------------------------------
// Member function: get_max_size
//
//! Returns the maximum width, in number of bits, of all memories
//----------------------------------------------------------------------

unsigned int uvm_mem::get_max_size()
{
  return m_max_size;
}

//----------------------------------------------------------------------
// Member function: get_virtual_registers
//
//! Return the virtual registers in this memory
//!
//! Fills the specified array with the abstraction class
//! for all of the virtual registers implemented in this memory.
//! The order in which the virtual registers are located in the array
//! is not specified.
//----------------------------------------------------------------------

void uvm_mem::get_virtual_registers( std::vector<uvm_vreg*>& regs ) const
{
  for( m_vregs_itt it = m_vregs.begin(); it != m_vregs.end(); it++ )
    regs.push_back( (*it).first );
}

//----------------------------------------------------------------------
// Member function: get_virtual_fields
//
//! Return the virtual fields in the memory
//!
//! Fills the specified dynamic array with the abstraction class
//! for all of the virtual fields implemented in this memory.
//! The order in which the virtual fields are located in the array is
//! not specified.
//----------------------------------------------------------------------

void uvm_mem::get_virtual_fields( std::vector<uvm_vreg_field*>& fields ) const
{
  for( m_vregs_itt it = m_vregs.begin(); it != m_vregs.end(); it++ )
  {
    uvm_vreg* vreg = (*it).first;
    vreg->get_fields(fields);
  }
}

//----------------------------------------------------------------------
// Member function: get_vreg_by_name
//
//! Find the named virtual register
//!
//! Finds a virtual register with the specified name
//! implemented in this memory and returns
//! its abstraction class instance.
//! If no virtual register with the specified name is found, returns NULL.
//----------------------------------------------------------------------

uvm_vreg* uvm_mem::get_vreg_by_name( const std::string& name ) const
{
  for( m_vregs_itt it = m_vregs.begin(); it != m_vregs.end(); it++ )
  {
    uvm_vreg* vreg = (*it).first;
    if (vreg->get_name() == name)
      return vreg;
  }

  UVM_WARNING("RegModel", "Unable to find virtual register '" + name +
    "' in memory '" + get_full_name() + "'");
  return NULL;
}

//----------------------------------------------------------------------
// Member function: get_vfield_by_name
//
//! Find the named virtual field
//!
//! Finds a virtual field with the specified name
//! implemented in this memory and returns
//! its abstraction class instance.
//! If no virtual field with the specified name is found, returns NULL.
//----------------------------------------------------------------------

uvm_vreg_field* uvm_mem::get_vfield_by_name( const std::string& name ) const
{
  // Return first occurrence of vfield matching name
  std::vector<uvm_vreg_field*> vfields;

  get_virtual_fields(vfields);

  for( unsigned int i = 0; i < vfields.size(); i++ )
    if (vfields[i]->get_name() == name)
      return vfields[i];

  UVM_WARNING("RegModel", "Unable to find virtual field '" + name +
    "' in memory '" + get_full_name() + "'");

  return NULL;
}

//----------------------------------------------------------------------
// Member function: get_vreg_by_offset
//
//! Find the virtual register implemented at the specified offset
//!
//! Finds the virtual register implemented in this memory
//! at the specified \p offset in the specified address \p map
//! and returns its abstraction class instance.
//! If no virtual register at the offset is found, returns NULL.
//----------------------------------------------------------------------

uvm_vreg* uvm_mem::get_vreg_by_offset( uvm_reg_addr_t offset,
                                       const uvm_reg_map* map ) const
{
  UVM_ERROR("RegModel", "uvm_mem::get_vreg_by_offset() not yet implemented");
  return NULL;
}

//----------------------------------------------------------------------
// Member function: get_offset
//
//! Returns the base offset of a memory location
//!
//! Returns the base offset of the specified location in this memory
//! in an address \p map.
//!
//! If no address map is specified and the memory is mapped in only one
//! address map, that address map is used. If the memory is mapped
//! in more than one address map, the default address map of the
//! parent block is used.
//!
//! If an address map is specified and
//! the memory is not mapped in the specified
//! address map, an error message is issued.
//----------------------------------------------------------------------

uvm_reg_addr_t uvm_mem::get_offset( uvm_reg_addr_t offset,
                                    const uvm_reg_map* map ) const
{
  uvm_reg_map_info* map_info;
  uvm_reg_map* lmap = get_local_map(map, "get_offset()");
  uvm_reg_map* orig_map = lmap;

  if (lmap == NULL)
    return -1; // FIXME: not good to return -1 for sc_uint!

  map_info = lmap->get_mem_map_info(this);

  if (map_info->unmapped)
  {
    std::ostringstream str;
    str << "Memory '"
        << get_name()
        << "' is unmapped in map '"
        << ((orig_map == NULL) ? lmap->get_full_name() : orig_map->get_full_name())
        << "'";
    UVM_WARNING("RegModel", str.str() );
    return -1; // TODO: not smart to return -1 for sc_uint!
  }

  return map_info->offset;
}

//----------------------------------------------------------------------
// Member function: get_address
//
//! Returns the base external physical address of a memory location
//!
//! Returns the base external physical address of the specified location
//! in this memory if accessed through the specified address \p map.
//!
//! If no address map is specified and the memory is mapped in only one
//! address map, that address map is used. If the memory is mapped
//! in more than one address map, the default address map of the
//! parent block is used.
//!
//! If an address map is specified and
//! the memory is not mapped in the specified
//! address map, an error message is issued.
//----------------------------------------------------------------------

uvm_reg_addr_t uvm_mem::get_address( uvm_reg_addr_t offset,
                                     const uvm_reg_map* map ) const
{
  std::vector<uvm_reg_addr_t> addr;
  get_addresses(addr, map, offset);
  return addr[0];
}

//----------------------------------------------------------------------
// Member function: get_addresses
//
//! Identifies the external physical address(es) of a memory location
//!
//! Computes all of the external physical addresses that must be accessed
//! to completely read or write the specified location in this memory.
//! The addressed are specified in little endian order.
//! Returns the number of bytes transfered on each access.
//!
//! If no address map is specified and the memory is mapped in only one
//! address map, that address map is used. If the memory is mapped
//! in more than one address map, the default address map of the
//! parent block is used.
//!
//! If an address map is specified and
//! the memory is not mapped in the specified
//! address map, an error message is issued.
//----------------------------------------------------------------------

int uvm_mem::get_addresses( std::vector<uvm_reg_addr_t>& addr,
                            const uvm_reg_map* map,
                            uvm_reg_addr_t offset ) const
{
  uvm_reg_map_info* map_info = NULL;
  uvm_reg_map* lmap = get_local_map(map,"get_addresses()");
  uvm_reg_map* orig_map = lmap;

  if (lmap == NULL)
    return 0;

  map_info = lmap->get_mem_map_info(this);

  if (map_info->unmapped)
  {
    std::ostringstream str;
    str << "Memory '"
        << get_name()
        << "' is unmapped in map '"
        << ((orig_map == NULL) ? lmap->get_full_name() : orig_map->get_full_name())
        << "'";
    UVM_WARNING("RegModel", str.str() );
    return 0;
  }

  addr = map_info->addr;

  for( unsigned int i = 0; i < addr.size(); i++ )
    addr[i] = addr[i] + map_info->mem_range.stride * offset;

  return lmap->get_n_bytes();
}

//----------------------------------------------------------------------
// Group: HDL Access
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: write
//
//! Write the specified value in a memory location
//!
//! Write \p value in the memory location that corresponds to this
//! abstraction class instance at the specified \p offset
//! using the specified access path.
//! If the memory is mapped in more than one address map,
//! an address \p map must be
//! specified if a physical access is used (front-door access).
//! If a back-door access path is used, the effect of writing
//! the register through a physical access is mimicked. For
//! example, a read-only memory will not be written.
//----------------------------------------------------------------------

void uvm_mem::write( uvm_status_e& status, // output
                     uvm_reg_addr_t offset,
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
  uvm_reg_item* rw = uvm_reg_item::type_id::create("mem_write", NULL, get_full_name() );

  rw->element      = this;
  rw->element_kind = UVM_MEM;
  rw->access_kind  = UVM_WRITE;
  rw->offset       = offset;
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
// Member function: read
//
//! Read the current value from a memory location
//!
//! Read and return \p value from the memory location that corresponds to this
//! abstraction class instance at the specified \p offset
//! using the specified access \p path.
//! If the register is mapped in more than one address map,
//! an address \p map must be
//! specified if a physical access is used (front-door access).
//----------------------------------------------------------------------

void uvm_mem::read( uvm_status_e& status, // output
                    uvm_reg_addr_t offset,
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
  rw = uvm_reg_item::type_id::create("mem_read", NULL, get_full_name());
  rw->element      = this;
  rw->element_kind = UVM_MEM;
  rw->access_kind  = UVM_READ;
  rw->value[0]     = false; // only first bit set
  rw->offset       = offset;
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

  uvm_reg_item::type_id::destroy(rw);
}

//----------------------------------------------------------------------
// Member function: burst_write
//
//! Write the specified values in memory locations
//!
//! Burst-write the specified \p values in the memory locations
//! beginning at the specified \p offset.
//! If the memory is mapped in more than one address map,
//! an address \p map must be specified if not using the backdoor.
//! If a back-door access path is used, the effect of writing
//! the register through a physical access is mimicked. For
//! example, a read-only memory will not be written.
//----------------------------------------------------------------------

void uvm_mem::burst_write( uvm_status_e& status,
                           uvm_reg_addr_t offset,
                           std::vector<uvm_reg_data_t> value,
                           uvm_path_e path,
                           uvm_reg_map* map,
                           uvm_sequence_base* parent,
                           int prior,
                           uvm_object* extension,
                           const std::string& fname,
                           int lineno )
{
  uvm_reg_item* rw;
  rw = uvm_reg_item::type_id::create("mem_burst_write", NULL, get_full_name());
  rw->element      = this;
  rw->element_kind = UVM_MEM;
  rw->access_kind  = UVM_BURST_WRITE;
  rw->offset       = offset;
  rw->value        = value; // write all bits
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
// Member function: burst_read
//
//! Read values from memory locations
//!
//! Burst-read into \p values the data the memory locations
//! beginning at the specified \p offset.
//! If the memory is mapped in more than one address map,
//! an address \p map must be specified if not using the backdoor.
//! If a back-door access path is used, the effect of writing
//! the register through a physical access is mimicked. For
//! example, a read-only memory will not be written.
//----------------------------------------------------------------------

void uvm_mem::burst_read( uvm_status_e& status, // output
                          uvm_reg_addr_t offset,
                          std::vector<uvm_reg_data_t>& value, // output
                          uvm_path_e path,
                          uvm_reg_map* map,
                          uvm_sequence_base* parent,
                          int prior,
                          uvm_object* extension,
                          const std::string& fname,
                          int lineno )
{
  uvm_reg_item* rw;
  rw = uvm_reg_item::type_id::create("mem_burst_read", NULL, get_full_name());
  rw->element      = this;
  rw->element_kind = UVM_MEM;
  rw->access_kind  = UVM_BURST_READ;
  rw->offset       = offset;
  rw->value        = value;  // read all bits
  rw->path         = path;
  rw->map          = map;
  rw->parent       = parent;
  rw->prior        = prior;
  rw->extension    = extension;
  rw->fname        = fname;
  rw->lineno       = lineno;

  do_read(rw);

  status = rw->status;
  value  = rw->value;

  uvm_reg_item::type_id::destroy(rw);
}

//----------------------------------------------------------------------
// Member function: poke
//
//! Deposit the specified value in a memory location
//!
//! Deposit the value in the DUT memory location corresponding to this
//! abstraction class instance at the specified \p offset, as-is,
//! using a back-door access.
//!
//! Uses the HDL path for the design abstraction specified by \p kind.
//----------------------------------------------------------------------

void uvm_mem::poke( uvm_status_e& status, // output
                    uvm_reg_addr_t offset,
                    uvm_reg_data_t value,
                    const std::string& kind,
                    uvm_sequence_base* parent,
                    uvm_object* extension,
                    const std::string& fname,
                    int lineno )
{
  uvm_reg_item* rw;
  uvm_reg_backdoor* bkdr = get_backdoor();

  m_fname = fname;
  m_lineno = lineno;

  if (bkdr == NULL && !has_hdl_path(kind))
  {
    UVM_ERROR("RegModel", "No backdoor access available in memory '" +
      get_full_name() + "'");
    status = UVM_NOT_OK;
    return;
  }

  // create an abstract transaction for this operation
  rw = uvm_reg_item::type_id::create("mem_poke_item", NULL, get_full_name());
  rw->element      = this;
  rw->path         = UVM_BACKDOOR;
  rw->element_kind = UVM_MEM;
  rw->access_kind  = UVM_WRITE;
  rw->offset       = offset;
  rw->value[0]     = value & uvm_mask_size(m_n_bits);
  rw->bd_kind      = kind;
  rw->parent       = parent;
  rw->extension    = extension;
  rw->fname        = fname;
  rw->lineno       = lineno;

  if (bkdr != NULL)
    bkdr->write(rw);
  else
    backdoor_write(rw);

  status = rw->status;

  std::ostringstream inf;
  inf << "Poked memory '"
      << get_full_name()
      << "[0x"
      << std::hex << offset.to_uint64()
      << "]' with value 0x"
      << std::hex << value.to_uint64()
      << ".";
  UVM_INFO("RegModel", inf.str(), UVM_HIGH);
  
  uvm_reg_item::type_id::destroy(rw);
}

//----------------------------------------------------------------------
// Member function: peek
//
//! Read the current value from a memory location
//!
//! Sample the value in the DUT memory location corresponding to this
//! abstraction class instance at the specified \p offset
//! using a back-door access.
//! The memory location value is sampled, not modified.
//!
//! Uses the HDL path for the design abstraction specified by \p kind.
//----------------------------------------------------------------------

void uvm_mem::peek( uvm_status_e& status, // output
                    uvm_reg_addr_t offset,
                    uvm_reg_data_t& value, // output
                    const std::string& kind,
                    uvm_sequence_base* parent,
                    uvm_object* extension,
                    const std::string& fname,
                    int lineno )
{
  uvm_reg_backdoor* bkdr = get_backdoor();
  uvm_reg_item* rw;

  m_fname = fname;
  m_lineno = lineno;

  if (bkdr == NULL && !has_hdl_path(kind))
  {
    UVM_ERROR("RegModel", "No backdoor access available in memory '" +
      get_full_name() + "'");
    status = UVM_NOT_OK;
    return;
  }

  // create an abstract transaction for this operation
  rw = uvm_reg_item::type_id::create("mem_peek_item", NULL, get_full_name());
  rw->element      = this;
  rw->path         = UVM_BACKDOOR;
  rw->element_kind = UVM_MEM;
  rw->access_kind  = UVM_READ;
  rw->offset       = offset;
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
  value  = rw->value[0];

  std::ostringstream inf;
  inf << "Peeked memory '"
      << get_full_name()
      << "[Ox"
      << offset
      << "]' has value 0x"
      << std::hex << value.to_uint64()
      << ".";
  UVM_INFO("RegModel", inf.str(), UVM_HIGH);
  
  uvm_reg_item::type_id::destroy(rw);
}

//----------------------------------------------------------------------
// Group: Frontdoor
//----------------------------------------------------------------------

// Member function: set_frontdoor
//
//! Set a user-defined frontdoor for this memory
//!
//! By default, memorys are mapped linearly into the address space
//! of the address maps that instantiate them.
//! If memorys are accessed using a different mechanism,
//! a user-defined access
//! mechanism must be defined and associated with
//! the corresponding memory abstraction class
//!
//! If the memory is mapped in multiple address maps, an address \p map
//! must be specified.
//----------------------------------------------------------------------

void uvm_mem::set_frontdoor( uvm_reg_frontdoor* ftdr,
                             uvm_reg_map* map,
                             const std::string& fname,
                             int lineno )
{
  uvm_reg_map_info* map_info;
  uvm_reg_map* local_map;

  m_fname = fname;
  m_lineno = lineno;

  local_map = get_local_map(map, "set_frontdoor()");

  if (local_map == NULL)
  {
    UVM_ERROR("RegModel", "Memory '" + get_full_name() +
      "' not found in map '" + map->get_full_name() + "'");
    return;
  }

  map_info = local_map->get_mem_map_info(this);
  map_info->frontdoor = ftdr;
}

//----------------------------------------------------------------------
// Member function: get_frontdoor
//
//! Returns the user-defined frontdoor for this memory
//!
//! If NULL, no user-defined frontdoor has been defined.
//! A user-defined frontdoor is defined
//! by using the uvm_mem::set_frontdoor() method.
//!
//! If the memory is mapped in multiple address maps, an address \p map
//! must be specified.
//----------------------------------------------------------------------

uvm_reg_frontdoor* uvm_mem::get_frontdoor( const uvm_reg_map* map ) const
{
  uvm_reg_map_info* map_info;
  uvm_reg_map* local_map;

  local_map = get_local_map(map, "set_frontdoor()");

  if (local_map == NULL)
  {
    UVM_ERROR("RegModel", "Memory '" + get_full_name() +
      "' not found in map '" + map->get_full_name() + "'");
    return NULL;
  }

  map_info = local_map->get_mem_map_info(this);
  return map_info->frontdoor;
}

//----------------------------------------------------------------------
// Group: Backdoor
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: set_backdoor
//
//! Set a user-defined backdoor for this memory
//!
//! By default, memories are accessed via the built-in string-based
//! DPI routines if an HDL path has been specified using the
//! uvm_mem::configure() or uvm_mem::add_hdl_path() method.
//! If this default mechanism is not suitable (e.g. because
//! the memory is not implemented in pure SystemVerilog)
//! a user-defined access
//! mechanism must be defined and associated with
//! the corresponding memory abstraction class
//----------------------------------------------------------------------

void uvm_mem::set_backdoor( uvm_reg_backdoor* bkdr,
                            const std::string& fname,
                            int lineno )
{
   m_fname = fname;
   m_lineno = lineno;
   m_backdoor = bkdr;
}

//----------------------------------------------------------------------
// Member function: get_backdoor
//
//! Returns the user-defined backdoor for this memory
//!
//! If NULL, no user-defined backdoor has been defined.
//! A user-defined backdoor is defined
//! by using the uvm_reg::set_backdoor() method.
//!
//! If argument \p inherit is true, returns the backdoor of the parent block
//! if none have been specified for this memory.
//----------------------------------------------------------------------

uvm_reg_backdoor* uvm_mem::get_backdoor( bool inherited )
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
// Member function: clear_hdl_path
//
//! Delete HDL paths
//!
//! Remove any previously specified HDL path to the memory instance
//! for the specified design abstraction.
//----------------------------------------------------------------------

void uvm_mem::clear_hdl_path( const std::string& kind )
{
  std::string lkind;

  if (lkind == "ALL")
  {
    m_hdl_paths_pool.clear();
    return;
  }

  if (lkind.empty())
    lkind = m_parent->get_default_hdl_path();

  if (m_hdl_paths_pool.find(lkind) == m_hdl_paths_pool.end())
  {
    UVM_WARNING("RegModel", "Unknown HDL Abstraction '" + lkind + "'");
    return;
  }

  m_hdl_paths_pool.erase(lkind);
}

//----------------------------------------------------------------------
// Member function: add_hdl_path
//
//! Add an HDL path
//!
//! Add the specified HDL path to the memory instance for the specified
//! design abstraction. This method may be called more than once for the
//! same design abstraction if the memory is physically duplicated
//! in the design abstraction
//----------------------------------------------------------------------

void uvm_mem::add_hdl_path( std::vector<uvm_hdl_path_slice> slices, const std::string& kind )
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
// Member function: add_hdl_path_slice
//
//! Add the specified HDL slice to the HDL path for the specified
//! design abstraction.
//! If \p first is true, starts the specification of a duplicate
//! HDL implementation of the memory.
//----------------------------------------------------------------------

void uvm_mem::add_hdl_path_slice( const std::string& name,
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
// Member function: has_hdl_path
//
//! Check if a HDL path is specified
//!
//! Returns TRUE if the memory instance has a HDL path defined for the
//! specified design abstraction. If no design abstraction is specified,
//! uses the default design abstraction specified for the parent block.
//----------------------------------------------------------------------

bool uvm_mem::has_hdl_path( const std::string& kind ) const
{
  std::string kindl = kind;

  if (kindl.empty())
    kindl = m_parent->get_default_hdl_path();

  return (m_hdl_paths_pool.find(kindl) != m_hdl_paths_pool.end() ); // exists
}

//----------------------------------------------------------------------
// Member function: get_hdl_path
//
//! Get the incremental HDL path(s)
//!
//! Returns the HDL path(s) defined for the specified design abstraction
//! in the memory instance.
//! Returns only the component of the HDL paths that corresponds to
//! the memory, not a full hierarchical path
//!
//! If no design abstraction is specified, the default design abstraction
//! for the parent block is used.
//----------------------------------------------------------------------

void uvm_mem::get_hdl_path( std::vector<uvm_hdl_path_concat>& paths,
                            const std::string& kind ) const
{
  std::vector<uvm_hdl_path_concat> hdl_paths;

  std::string lkind = kind;

  if (lkind.empty())
    lkind = m_parent->get_default_hdl_path();

  if (!has_hdl_path(lkind))
  {
    UVM_ERROR("RegModel",
       "Memory does not have HDL path defined for abstraction '" + lkind + "'");
    return;
  }

  if (m_hdl_paths_pool.find(lkind) != m_hdl_paths_pool.end())
    paths = m_hdl_paths_pool.find(lkind)->second;
}

//----------------------------------------------------------------------
// Member function: get_full_hdl_path
//
//! Get the full hierarchical HDL path(s)
//!
//! Returns the full hierarchical HDL path(s) defined for the specified
//! design abstraction in the memory instance.
//! There may be more than one path returned even
//! if only one path was defined for the memory instance, if any of the
//! parent components have more than one path defined for the same design
//! abstraction
//!
//! If no design abstraction is specified, the default design abstraction
//! for each ancestor block is used to get each incremental path.
//----------------------------------------------------------------------

void uvm_mem::get_full_hdl_path( std::vector<uvm_hdl_path_concat>& paths,
                                 const std::string& kind,
                                 const std::string& separator ) const
{
  std::string lkind = kind;

  if (lkind.empty())
    lkind = m_parent->get_default_hdl_path();

  if (!has_hdl_path(lkind))
  {
    UVM_ERROR("RegModel",
       "Memory " + get_full_name() +
       " does not have HDL path defined for abstraction '"  + lkind + "'");
    return;
  }

  std::vector<uvm_hdl_path_concat> hdl_paths;

  if (m_hdl_paths_pool.find(lkind) != m_hdl_paths_pool.end())
    hdl_paths = m_hdl_paths_pool.find(lkind)->second;

  std::vector<std::string> parent_paths;

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
// Member function: get_hdl_path_kinds
//
//! Get design abstractions for which HDL paths have been defined
//----------------------------------------------------------------------

void uvm_mem::get_hdl_path_kinds( std::vector<std::string>& kinds ) const
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
// Member function: backdoor_read
//
//! User-define backdoor read access
//!
//! Override the default string-based DPI backdoor access read
//! for this memory type.
//! By default calls uvm_mem::backdoor_read_func().
//----------------------------------------------------------------------

void uvm_mem::backdoor_read( uvm_reg_item* rw )
{
  rw->status = backdoor_read_func(rw);
}


//----------------------------------------------------------------------
// Member function: backdoor_write
//
//! User-defined backdoor read access
//!
//! Override the default string-based DPI backdoor access write
//! for this memory type.
//----------------------------------------------------------------------

void uvm_mem::backdoor_write( uvm_reg_item* rw )
{
  std::vector<uvm_hdl_path_concat> paths;
  bool ok = true;

  get_full_hdl_path(paths, rw->bd_kind);

  for( unsigned int mem_idx = 0; mem_idx < rw->value.size(); mem_idx++ )
  {
    std::ostringstream idx;
    idx << (rw->offset + mem_idx);

    for( unsigned int i = 0; i < paths.size(); i++ )
    {
      uvm_hdl_path_concat hdl_concat = paths[i];

      for ( unsigned int j = 0; j < hdl_concat.slices.size(); j++)
      {
        std::ostringstream str;
        str << "backdoor_write to "
            << hdl_concat.slices[j].path
            << ".";
        UVM_INFO("RegModel", str.str(), UVM_DEBUG);

        if (hdl_concat.slices[j].offset < 0)
        {
          ok &= uvm_hdl_deposit(hdl_concat.slices[j].path + "[" + idx.str() + "]",rw->value[mem_idx]);
          continue;
        }

        uvm_reg_data_t slice;
        slice = rw->value[mem_idx] >> hdl_concat.slices[j].offset;
        slice &= (1 << hdl_concat.slices[j].size)-1; // TODO check
        ok &= uvm_hdl_deposit( hdl_concat.slices[j].path + "[" + idx.str() + "]", slice);
      }
    }
  }

  rw->status = (ok ? UVM_IS_OK : UVM_NOT_OK);
}

//----------------------------------------------------------------------
// Member function: backdoor_read_func
//
//! User-defined backdoor read access
//!
//! Override the default string-based DPI backdoor access read
//! for this memory type.
//----------------------------------------------------------------------

uvm_status_e uvm_mem::backdoor_read_func( uvm_reg_item* rw )
{
  std::vector<uvm_hdl_path_concat> paths;
  uvm_hdl_data_t val;
  bool ok = true;

  get_full_hdl_path(paths, rw->bd_kind);

  for( unsigned int mem_idx = 0; mem_idx < rw->value.size(); mem_idx++ )
  {
    std::ostringstream idx;
    idx << (rw->offset + mem_idx);

    for( unsigned int i = 0; i < paths.size(); i++ )
    {
      uvm_hdl_path_concat hdl_concat = paths[i];
      val = 0;
      for ( unsigned int j = 0; j < hdl_concat.slices.size(); j++)
      {
        std::string hdl_path = hdl_concat.slices[j].path + "[" + idx.str() + "]";

        UVM_INFO("RegModel", "backdoor_read from " + hdl_path + ".", UVM_DEBUG);

        if (hdl_concat.slices[j].offset < 0)
        {
          ok &= uvm_hdl_read(hdl_path, val);
          continue;
        }

        uvm_reg_data_t slice;
        int k = hdl_concat.slices[j].offset;
        ok &= uvm_hdl_read(hdl_path, slice);
        for ( int r = 0; r < hdl_concat.slices[j].size; r++)
        {
          val[k++] = slice[0];
          slice >>= 1;
        }

      } // for

      val &= (1 << m_n_bits)-1;

      if (i == 0) rw->value[mem_idx] = val;

      if (val != rw->value[mem_idx])
      {
        std::ostringstream str;
        str << "Backdoor read of register "
            << get_full_name()
            << " with multiple HDL copies: values are not the same: 0x"
            << std::hex << rw->value[mem_idx].to_uint64()
            << " at path '"
            << uvm_hdl_concat2string(paths[0])
            << "', and 0x"
            << std::hex << val.to_uint64()
            << " at path '"
            << uvm_hdl_concat2string(paths[i])
            << "'. Returning first value.";
        UVM_ERROR("RegModel", str.str() );
        return UVM_NOT_OK;
      }
    }
  }

  rw->status = (ok ? UVM_IS_OK : UVM_NOT_OK);

  return rw->status;
}

//----------------------------------------------------------------------
// Group: Callbacks
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: pre_write
//
//! Called before memory write.
//!
//! If the \p offset, \p value, access \p path,
//! or address \p map are modified, the updated offset, data value,
//! access path or address map will be used to perform the memory operation.
//! If the \p status is modified to anything other than UVM_IS_OK,
//! the operation is aborted.
//!
//! The registered callback methods are invoked after the invocation
//! of this method.
//----------------------------------------------------------------------

void uvm_mem::pre_write( uvm_reg_item* rw )
{}

//----------------------------------------------------------------------
// Member function: post_write
//
// Called after memory write.
//
//!! If the \p status is modified, the updated status will be
//!! returned by the memory operation.
//!!
//!! The registered callback methods are invoked before the invocation
//!! of this method.
//----------------------------------------------------------------------

void uvm_mem::post_write( uvm_reg_item* rw )
{}

//----------------------------------------------------------------------
// Member function: pre_read
//
//! Called before memory read.
//!
//! If the \p offset, access \p path or address \p map are modified,
//! the updated offset, access path or address map will be used to perform
//! the memory operation.
//! If the \p status is modified to anything other than UVM_IS_OK,
//! the operation is aborted.
//!
//! The registered callback methods are invoked after the invocation
//! of this method.
//----------------------------------------------------------------------

void uvm_mem::pre_read( uvm_reg_item* rw )
{}

//----------------------------------------------------------------------
// Member function: post_read
//
//! Called after memory read.
//!
//! If the readback data or status is modified,
//! the updated readback data or status will be
//! returned by the memory operation.
//!
//! The registered callback methods are invoked before the invocation
//! of this method.
//----------------------------------------------------------------------

void uvm_mem::post_read( uvm_reg_item* rw )
{}


//----------------------------------------------------------------------
// Group: Coverage
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: build_coverage
//
//! Check if all of the specified coverage model must be built.
//!
//! Check which of the specified coverage model must be built
//! in this instance of the memory abstraction class,
//! as specified by calls to uvm_reg::include_coverage().
//!
//! Models are specified by adding the symbolic value of individual
//! coverage model as defined in #uvm_coverage_model_e.
//! Returns the sum of all coverage models to be built in the
//! memory model.
//----------------------------------------------------------------------

uvm_reg_cvr_t uvm_mem::build_coverage( uvm_reg_cvr_t models )
{
  uvm_reg_cvr_t coverage = UVM_NO_COVERAGE;

  uvm_reg_cvr_rsrc_db::read_by_name("uvm_reg::" + get_full_name(),
                                    "include_coverage",
                                    coverage, this);
  return coverage & models;
}

//----------------------------------------------------------------------
// Member function: add_coverage
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

void uvm_mem::add_coverage( uvm_reg_cvr_t models )
{
  m_has_cover |= models;
}

//----------------------------------------------------------------------
// Member function: has_coverage
//
// Check if memory has coverage model(s)
//
// Returns TRUE if the memory abstraction class contains a coverage model
// for all of the models specified.
// Models are specified by adding the symbolic value of individual
// coverage model as defined in <uvm_coverage_model_e>.
//----------------------------------------------------------------------

bool uvm_mem::has_coverage( uvm_reg_cvr_t models ) const
{
  return ((m_has_cover & models) == models);
}

//----------------------------------------------------------------------
// Member function: set_coverage
//
//! Turns on coverage measurement.
//!
//! Turns the collection of functional coverage measurements on or off
//! for this memory.
//! The functional coverage measurement is turned on for every
//! coverage model specified using #uvm_coverage_model_e symbolic
//! identifers.
//! Multiple functional coverage models can be specified by adding
//! the functional coverage model identifiers.
//! All other functional coverage models are turned off.
//! Returns the sum of all functional
//! coverage models whose measurements were previously on.
//!
//! This method can only control the measurement of functional
//! coverage models that are present in the memory abstraction classes,
//! then enabled during construction.
//! See the uvm_mem::has_coverage() method to identify
//! the available functional coverage models.
//----------------------------------------------------------------------

uvm_reg_cvr_t uvm_mem::set_coverage( uvm_reg_cvr_t is_on )
{
  if (is_on == (uvm_reg_cvr_t)UVM_NO_COVERAGE)
  {
    m_cover_on = is_on;
    return m_cover_on;
  }

  m_cover_on = m_has_cover & is_on;

  return m_cover_on;
}

//----------------------------------------------------------------------
// Member function: get_coverage
//
// Check if coverage measurement is on.
//
//! Returns TRUE if measurement for all of the specified functional
//! coverage models are currently on.
//! Multiple functional coverage models can be specified by adding the
//! functional coverage model identifiers.
//!
//! See uvm_mem::set_coverage() for more details.
//----------------------------------------------------------------------

bool uvm_mem::get_coverage( uvm_reg_cvr_t is_on )
{
   if (has_coverage(is_on) == false)
     return false;

   return ((m_cover_on & is_on) == is_on);
}

//----------------------------------------------------------------------
// Member function: sample
//
//! Functional coverage measurement method
//!
//! This method is invoked by the memory abstraction class
//! whenever an address within one of its address map
//! is successfully read or written.
//! The specified offset is the offset within the memory,
//! not an absolute address.
//!
//! Empty by default, this method may be extended by the
//! abstraction class generator to perform the required sampling
//! in any provided functional coverage model.
//----------------------------------------------------------------------

void uvm_mem::sample( uvm_reg_addr_t offset,
                      bool is_read,
                      uvm_reg_map* map )
{}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------
// Member function: set_parent
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::set_parent( uvm_reg_block* parent )
{
  m_parent = parent;
}

//----------------------------------------------------------------------
// Member function: add_map
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::add_map( uvm_reg_map* map )
{
  m_maps[map] = true;
}

//----------------------------------------------------------------------
// Member function: m_lock_model
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::m_lock_model()
{
  m_locked = true;
}

//----------------------------------------------------------------------
// Member function: m_add_vreg
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::m_add_vreg( uvm_vreg* vreg )
{
  m_vregs[vreg] = true;
}

//----------------------------------------------------------------------
// Member function: m_delete_vreg
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::m_delete_vreg( uvm_vreg* vreg )
{
  if (m_vregs.find(vreg) != m_vregs.end()) // exists
     m_vregs.erase(vreg);
}

//----------------------------------------------------------------------
// Member function: get_block
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_block* uvm_mem::get_block()
{
  return m_parent;
}


//----------------------------------------------------------------------
// Member function: get_local_map
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_map* uvm_mem::get_local_map( const uvm_reg_map* map,
                                     const std::string& caller ) const
{
  uvm_reg_map* lmap = const_cast<uvm_reg_map*>(map);

  if (lmap == NULL)
    return get_default_map();

  if (m_maps.find(lmap) != m_maps.end() ) // exists
    return lmap;

  for( m_maps_itt it = m_maps.begin();
      it != m_maps.end();
      it++)
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
  str << "Memory '"
      << get_full_name()
      << "' is not contained within map '"
      << lmap->get_full_name()
      << "'"
      << ((caller.empty()) ? "" : (" (called from " + caller + ")" ) )
      << ".";
  UVM_WARNING("RegModel", str.str() );

  return NULL;
}

//----------------------------------------------------------------------
// Member function: get_default_map
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_map* uvm_mem::get_default_map( const std::string& caller ) const
{
  uvm_reg_map* _map;

  // if mem is not associated with any may, return null
  if (m_maps.size() == 0)
  {
    std::ostringstream str;
    str << "Memory '"
        << get_full_name()
        << "' is not registered with any map"
        << ((caller.empty()) ? "" : (" (called from " + caller + ")"))
        << ".";
    UVM_WARNING("RegModel", str.str() );
    return NULL;
  }

  // if only one map, choose that
  if (m_maps.size() == 1)
    _map = (*m_maps.begin()).first;


  // try to choose one based on default_map in parent blocks.
  for( m_maps_itt it = m_maps.begin();
      it != m_maps.end();
      it++)
  {
    uvm_reg_map* map = (*it).first;
    uvm_reg_block* blk = map->get_parent();
    uvm_reg_map* default_map = blk->get_default_map();

    if (default_map != NULL)
    {
      uvm_reg_map* local_map = get_local_map(default_map);
      if (local_map != NULL)
        return local_map;
    }
  }

  // if that fails, choose the first in this mem's maps
  _map = (*m_maps.begin()).first;

  return _map;
}


//----------------------------------------------------------------------
// Member function: m_check_access
//
// Implementation defined
//----------------------------------------------------------------------
// TODO correct argument types (*&?)

bool uvm_mem::m_check_access( uvm_reg_item* rw,
                              uvm_reg_map_info*& map_info, // output
                              const std::string& caller )
{
  if (rw->offset >= m_size)
  {
    std::ostringstream str;
    str << "Offset 0x"
        << std::hex << rw->offset.to_uint64()
        << " exceeds size of memory, 0x"
        << std::hex << m_size
        << ".";
    UVM_ERROR(get_type_name(), str.str() );
    rw->status = UVM_NOT_OK;
    return false;
  }

  if (rw->path == UVM_DEFAULT_PATH)
    rw->path = m_parent->get_default_path();

  if (rw->path == UVM_BACKDOOR)
  {
    if (get_backdoor() == NULL && !has_hdl_path())
    {
      UVM_WARNING("RegModel",
        "No backdoor access available for memory '" + get_full_name() +
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
        "No transactor available to physically access memory from map '" +
         rw->map->get_full_name() + "'");
      rw->status = UVM_NOT_OK;
      return false;
    }

    map_info = rw->local_map->get_mem_map_info(this);

    if (map_info->frontdoor == NULL)
    {
      if (map_info->unmapped)
      {
        UVM_ERROR("RegModel", "Memory '" + get_full_name() +
            "' unmapped in map '" + rw->map->get_full_name() +
            "' and does not have a user-defined frontdoor");
        rw->status = UVM_NOT_OK;
        return false;
      }

      if ((rw->value.size() > 1))
      {
        if (get_n_bits() > rw->local_map->get_n_bytes()*8)
        {
          std::ostringstream str;
          str << "Cannot burst a "
              << get_n_bits()
              << "-bit memory through a narrower data path ("
              << rw->local_map->get_n_bytes()*8
              << " bytes).";
          UVM_ERROR("RegModel", str.str() );
          rw->status = UVM_NOT_OK;
          return false;
        }
        if (rw->offset + rw->value.size() > m_size)
        {
          std::ostringstream str;
          str << "Burst of size "
              << rw->value.size()
              << " starting at offset 0x"
              << std::hex << rw->offset.to_uint64()
              << " exceeds size of memory, 0x"
              << std::hex << m_size
              << ".";
          UVM_ERROR("RegModel", str.str() );
          return false;
        }
      }
    }

    if (rw->map == NULL)
      rw->map = rw->local_map;
  }
  return true;
}


//----------------------------------------------------------------------
// Member function: do_write
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::do_write( uvm_reg_item* rw )
{
  uvm_mem_cb_iter* cbs = new uvm_mem_cb_iter(this);
  uvm_reg_map_info* map_info;

  m_fname  = rw->fname;
  m_lineno = rw->lineno;

  if (!m_check_access(rw, map_info, "burst_write()"))
    return;

  m_write_in_progress = true;

  rw->status = UVM_IS_OK;

  // PRE-WRITE CBS
  pre_write(rw);

  for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next())
    cb->pre_write(rw);

  if (rw->status != UVM_IS_OK)
  {
    m_write_in_progress = false;
    return;
  }

  rw->status = UVM_NOT_OK;

  // FRONTDOOR
  if (rw->path == UVM_FRONTDOOR)
  {
    uvm_reg_map* system_map = rw->local_map->get_root_map();

    if (map_info->frontdoor != NULL)
    {
      uvm_reg_frontdoor* fd = map_info->frontdoor;
      fd->rw_info = rw;
      if (fd->sequencer == NULL)
        fd->sequencer = system_map->get_sequencer();
      fd->start(fd->sequencer, rw->parent);
    }
    else
      rw->local_map->do_write(rw);

    if (rw->status != UVM_NOT_OK)
      for ( uvm_reg_addr_t idx = rw->offset;
          idx <= rw->offset + rw->value.size();
          idx++ )
      {
        m_sample(map_info->mem_range.stride * idx, 0, rw->map);
        m_parent->m_sample(map_info->offset +
          (map_info->mem_range.stride * idx), 0, rw->map);
      }
  }
  // BACKDOOR
  else
  {
    // Mimick front door access, i.e. do not write read-only memories
    if (get_access(rw->map) == "RW")
    {
      uvm_reg_backdoor* bkdr = get_backdoor();
      if (bkdr != NULL)
        bkdr->write(rw);
      else
        backdoor_write(rw);
    }
    else
      rw->status = UVM_IS_OK;
  }

  // POST-WRITE CBS
  post_write(rw);

  for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next())
    cb->post_write(rw);

  // REPORT
  if (uvm_report_enabled(UVM_HIGH, UVM_INFO, "RegModel"))
  {
    std::ostringstream path_s, value_s, pre_s, range_s;
    if (rw->path == UVM_FRONTDOOR)
      path_s << ((map_info->frontdoor != NULL) ? "user frontdoor" :
          ("map " + rw->map->get_full_name()));
    else
      path_s << ((get_backdoor() != NULL) ? "user backdoor" : "DPI backdoor");

    if (rw->value.size() > 1)
    {
      value_s << "='{";
      pre_s << "Burst ";
      for( unsigned int i = 0; i < rw->value.size(); i++ )
        value_s << "0x" << std::hex << rw->value[i].to_uint64();

      value_s << "}";
      range_s << "[" << rw->offset << ":" << rw->offset + rw->value.size() << "]";
    }
    else
    {
      value_s << " = 0x" << std::hex << rw->value[0].to_uint64();
      range_s << "[%" << rw->offset << "]";
    }

    uvm_report_info("RegModel", pre_s.str() + "wrote memory via " +
        path_s.str() + ": " + get_full_name() + range_s.str() + value_s.str(), UVM_HIGH);
  }

  m_write_in_progress = false;
  delete cbs;
}

//----------------------------------------------------------------------
// Member function: do_read
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::do_read( uvm_reg_item* rw )
{
  uvm_mem_cb_iter* cbs = new uvm_mem_cb_iter(this);
  uvm_reg_map_info* map_info;

  m_fname = rw->fname;
  m_lineno = rw->lineno;

  if (!m_check_access(rw, map_info, "burst_read()"))
    return;

  m_read_in_progress = true;

  rw->status = UVM_IS_OK;

  // PRE-READ CBS
  pre_read(rw);

  for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next() )
    cb->pre_read(rw);

  if (rw->status != UVM_IS_OK)
  {
    m_read_in_progress = false;
    return;
  }

  rw->status = UVM_NOT_OK;

  // FRONTDOOR
  if (rw->path == UVM_FRONTDOOR)
  {
    uvm_reg_map* system_map = rw->local_map->get_root_map();

    if (map_info->frontdoor != NULL)
    {
      uvm_reg_frontdoor* fd = map_info->frontdoor;
      fd->rw_info = rw;
      if (fd->sequencer == NULL)
        fd->sequencer = system_map->get_sequencer();
      fd->start(fd->sequencer, rw->parent);
    }
    else
      rw->local_map->do_read(rw);

    if (rw->status != UVM_NOT_OK)
      for (uvm_reg_addr_t idx = rw->offset;
          idx <= rw->offset + rw->value.size();
          idx++ )
      {
        m_sample(map_info->mem_range.stride * idx, 1, rw->map);
        m_parent->m_sample( map_info->offset +
            (map_info->mem_range.stride * idx),
            1, rw->map);
      }
  }

  // BACKDOOR
  else
  {
    uvm_reg_backdoor* bkdr = get_backdoor();
    if (bkdr != NULL)
      bkdr->read(rw);
    else
      backdoor_read(rw);
  }

  // POST-READ CBS
  post_read(rw);

  for( uvm_reg_cbs* cb = cbs->first(); cb != NULL; cb = cbs->next())
    cb->post_read(rw);

  // REPORT
  if (uvm_report_enabled(UVM_HIGH, UVM_INFO, "RegModel"))
  {
    std::ostringstream path_s, value_s, pre_s, range_s;
    if (rw->path == UVM_FRONTDOOR)
      path_s << ((map_info->frontdoor != NULL) ? "user frontdoor" :
          ("map " + rw->map->get_full_name() ));
    else
      path_s << ((get_backdoor() != NULL) ? "user backdoor" : "DPI backdoor");

    if (rw->value.size() > 1)
    {
      value_s << "='{";
      pre_s << "Burst ";

      for( unsigned int i = 0; i < rw->value.size(); i++)
        value_s << "0x" << std::hex << rw->value[i].to_uint64();

      value_s << "}";
      range_s << "[" << rw->offset << ":" << (rw->offset+rw->value.size()) << "]";
    }
    else
    {
      value_s << " = 0x" << std::hex << rw->value[0].to_uint64();
      range_s << "[" << rw->offset << "]";
    }

    uvm_report_info("RegModel", pre_s.str() + "read memory via " + path_s.str() + ": " +
        get_full_name() + range_s.str() + value_s.str(), UVM_HIGH);
  }

  m_read_in_progress = false;
  delete cbs;
}

//----------------------------------------------------------------------
// Member function: m_sample
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::m_sample( uvm_reg_addr_t addr,
                        bool is_read,
                        uvm_reg_map* map )
{
  sample(addr, is_read, map);
}


//----------------------------------------------------------------------
// Member function: convert2string
//
// Implementation defined
//----------------------------------------------------------------------

std::string uvm_mem::convert2string() const
{
  std::ostringstream res_str;
  std::string prefix;
  std::string cvt_str;
  std::ostringstream str1;
  std::ostringstream str2;

  str1 << prefix
       << "Memory "
       << get_full_name()
       << " -- "
       << get_size()
       << "x"
       << get_n_bits()
       << " bits";


  if (m_maps.size() == 0)
    str1 << "  (unmapped)\n";
  else
    str1 << "\n";

  cvt_str = str1.str();

  for( m_maps_itt it = m_maps.begin(); it != m_maps.end(); it++ )
  {
    uvm_reg_map* parent_map = (*it).first;
    unsigned int offset;

    while (parent_map != NULL)
    {
      uvm_reg_map* this_map = parent_map;
      uvm_endianness_e endian_name;
      parent_map = this_map->get_parent_map();
      endian_name = this_map->get_endian();

      offset = (parent_map == NULL) ? this_map->get_base_addr(UVM_NO_HIER) :
          parent_map->get_submap_offset(this_map);
      prefix = prefix + "  ";
      str2 << prefix
          << "Mapped in '"
          << this_map->get_full_name()
          << "' -- buswidth "
          << this_map->get_n_bytes()
          << " bytes, "
          << uvm_endianness_name[endian_name]
          << " offset 0x"
          << std::hex << offset
          << " size 0x"
          << std::hex << get_size()
          << ", "
          << get_access(this_map)
          << "\n";
          cvt_str = str2.str();
    } // while
  }

  prefix = "  ";

  if (m_read_in_progress)
  {
    if (m_fname != "" && m_lineno != 0)
      res_str << m_fname << ":" << m_lineno;
    cvt_str = cvt_str + "  " + res_str.str() +
        "currently executing read method";
  }

  if (m_write_in_progress)
  {
    if (m_fname != "" && m_lineno != 0)
      res_str << m_fname << ":" << m_lineno;
    cvt_str = cvt_str + "  " + res_str.str() +
        "currently executing write method";
  }
  return cvt_str;
}

//----------------------------------------------------------------------
// Member function: do_print
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::do_print( const uvm_printer& printer ) const
{
  uvm_object::do_print(printer);
  //printer.print_generic(" ", " ", -1, convert2string());
  printer.print_field_int("n_bits", get_n_bits(), 32, UVM_UNSIGNED);
  printer.print_field_int("size", get_size(), 32, UVM_UNSIGNED);
}


//----------------------------------------------------------------------
// Member function: clone
//
// Implementation defined
//----------------------------------------------------------------------

uvm_object* uvm_mem::clone()
{
  UVM_FATAL("RegModel","RegModel memories cannot be cloned");
  return NULL;
}

//----------------------------------------------------------------------
// Member function: do_copy
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::do_copy( const uvm_object& rhs )
{
  UVM_FATAL("RegModel","RegModel memories cannot be copied");
}

//----------------------------------------------------------------------
// Member function: do_compare
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_mem::do_compare( const uvm_object& rhs,
                          const uvm_comparer* comparer ) const
{
  UVM_WARNING("RegModel","RegModel memories cannot be compared");
  return false;
}

//----------------------------------------------------------------------
// Member function: do_pack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::do_pack( uvm_packer& packer ) const
{
  UVM_WARNING("RegModel","RegModel memories cannot be packed");
}

//----------------------------------------------------------------------
// Member function: do_unpack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_mem::do_unpack( uvm_packer& packer )
{
  UVM_WARNING("RegModel","RegModel memories cannot be unpacked");
}

//////////

} // namespace uvm
