//----------------------------------------------------------------------
//   Copyright 2026 COSEDA Technologies GmbH
//   Copyright 2013-2015 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010 Mentor Graphics Corporation
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

#include "uvmsc/reg/uvm_vreg.h"
#include "uvmsc/reg/uvm_vreg_field.h"
#include "uvmsc/reg/uvm_reg_block.h"
#include "uvmsc/reg/uvm_reg_map.h"
#include "uvmsc/reg/uvm_mem.h"
#include "uvmsc/reg/uvm_mem_mam.h"
#include "uvmsc/reg/uvm_mem_region.h"
#include "uvmsc/reg/uvm_reg_cbs.h"
#include "uvmsc/reg/uvm_vreg_cbs.h"
#include "uvmsc/reg/uvm_vreg_field_cbs.h"

namespace uvm {

//----------------------------------------------------------------------
// Initialization of static data members
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Constructor
//
//! Create a new instance and type-specific configuration
//!
//! Creates an instance of a virtual register abstraction class
//! with the specified name.
//!
//! \p n_bools specifies the total number of bools in a virtual register.
//! Not all bools need to be mapped to a virtual field.
//! This value is usually a multiple of 8.
//----------------------------------------------------------------------

uvm_vreg::uvm_vreg( const std::string& name, unsigned int n_bits ) : uvm_object(name)
{
  m_locked = false;
  m_parent = nullptr;

  m_n_bits = n_bits;

  if (n_bits == 0)
  {
    UVM_ERROR("RegModel",
      "Virtual register '" + get_full_name() + "' cannot have 0 bits. It will be set to one bit.");
    m_n_bits = 1;
  }

  if (n_bits > UVM_REG_DATA_WIDTH)
  {
    std::ostringstream str;
    str << "Virtual register '"
        << get_full_name()
        << "' cannot have more than "
        << UVM_REG_DATA_WIDTH
        << " bits ("
        << n_bits
        << ").";
    UVM_ERROR("RegModel", str.str() );
    n_bits = UVM_REG_DATA_WIDTH;
  }

  m_fields.clear();
  m_mem = nullptr;

  m_offset = 0;
  m_incr = 0;
  m_size = 0;
  m_is_static = false;
  m_region = nullptr;
  m_fname = "";
  m_lineno = 0;
  m_read_in_progress = false;
  m_write_in_progress = false;
}


//----------------------------------------------------------------------
// Member function: configure
//
//! Instance-specific configuration
//!
//! Specify the \p parent block of this virtual register array.
//! If one of the other parameters are specified, the virtual register
//! is assumed to be dynamic and can be later (re-)implemented using
//! the uvm_vreg::implement() method.
//!
//! If \p mem is specified, then the virtual register array is assumed
//! to be statically implemented in the memory corresponding to the specified
//! memory abstraction class and \p size, \p offset and \p incr
//! must also be specified.
//! Static virtual register arrays cannot be re-implemented.
//----------------------------------------------------------------------

void uvm_vreg::configure( uvm_reg_block* parent,
                          uvm_mem* mem,
                          unsigned long size,
                          uvm_reg_addr_t offset,
                          unsigned int incr )
{
  m_parent = parent;

  m_n_used_bits = 0;

  if (mem != nullptr)
  {
    implement(size, mem, offset, incr);
    m_is_static = true;
  }
  else
  {
    m_mem = nullptr;
    m_is_static = false;
  }

  m_parent->add_vreg(this);
}

//----------------------------------------------------------------------
// Member function: implement
//
//! Dynamically implement, resize or relocate a virtual register array
//!
//! Implement an array of virtual registers of the specified
//! \p size, in the specified memory and \p offset.
//! If an offset increment is specified, each
//! virtual register is implemented at the specified offset increment
//! from the previous one.
//! If an offset increment of 0 is specified,
//! virtual registers are packed as closely as possible
//! in the memory.
//!
//! If no memory is specified, the virtual register array is
//! in the same memory, at the same base offset using the same
//! offset increment as originally implemented.
//! Only the number of virtual registers in the virtual register array
//! is modified.
//!
//! The initial value of the newly-implemented or
//! relocated set of virtual registers is whatever values
//! are currently stored in the memory now implementing them.
//!
//! Returns true if the memory
//! can implement the number of virtual registers
//! at the specified base offset and offset increment.
//! Returns false otherwise.
//!
//! The memory region used to implement a virtual register array
//! is reserved in the memory allocation manager associated with
//! the memory to prevent it from being allocated for another purpose.
//----------------------------------------------------------------------

bool uvm_vreg::implement( unsigned long n,
                          uvm_mem* mem,
                          uvm_reg_addr_t offset,
                          unsigned int incr )
{
  uvm_mem_region* region = nullptr;

  if(n < 1)
  {
    UVM_ERROR("RegModel",
        "Attempting to implement virtual register '" + get_full_name() +
        "' with a subscript less than one doesn't make sense");
    return false;
  }

  if(mem == nullptr)
  {
    UVM_ERROR("RegModel",
        "Attempting to implement virtual register '" + get_full_name() +
        "' using a nullptr uvm_mem reference");
    return false;
  }

  if (m_is_static)
  {
    UVM_ERROR("RegModel",
        "Virtual register '" + get_full_name() +
        "' is static and cannot be dynamically implemented");
    return false;
  }

  if( mem->get_block() != m_parent )
  {
    UVM_ERROR("RegModel",
        "Attempting to implement virtual register '" + get_full_name() +
        "' on memory '" + mem->get_full_name() +
        "' in a different block.");
    return false;
  }

  unsigned int min_incr = (get_n_bytes()-1) / mem->get_n_bytes() + 1;
  if (incr == 0) incr = min_incr;
  if (min_incr > incr)
  {
    std::ostringstream str;
    str << "Virtual register '"
        << get_full_name()
        << "' increment is too small ("
        << incr
        << "): Each virtual register requires at least "
        << min_incr
        << " locations in memory '"
        << mem->get_full_name()
        << "'.";
    UVM_ERROR("RegModel", str.str() );
    return false;
  }

  // Check if the memory is big enough
  if (offset + (n * incr) > mem->get_size())
  {
    std::ostringstream str;
    str << "Given offset for virtual register '"
        << get_full_name()
        << "["
        << n
        << "]' is too big for memory "
        <<  mem->get_full_name()
        << "@ 0x"
        << std::hex << offset.to_uint64();

    UVM_ERROR("RegModel", str.str() );
    return false;
  }

  region = mem->mam->reserve_region(offset, n*incr*mem->get_n_bytes());

  if (m_region == nullptr)
  {
    UVM_ERROR("RegModel",
        "Could not allocate a memory region for virtual register '" + get_full_name() +
        "'." );
    return false;
  }

  if (m_mem != nullptr)
  {
    std::ostringstream str;
    str << "Virtual register '"
        << get_full_name()
        << "' is being moved re-implemented from "
        << mem->get_full_name()
        << "@ 0x"
        << std::hex <<  m_offset.to_uint64()
        << " to "
        << mem->get_full_name()
        << "@ 0x"
        << std::hex << offset.to_uint64()
        << ".";
    UVM_INFO("RegModel", str.str(), UVM_MEDIUM);
    release_region();
  }

  m_region = region;
  m_mem    = mem;
  m_size   = n;
  m_offset = offset;
  m_incr   = incr;
  m_mem->m_add_vreg(this);

  return true;
}

//----------------------------------------------------------------------
// Member function: allocate
//
//! Randomly implement, resize or relocate a virtual register array
//!
//! Implement a virtual register array of the specified
//! size in a randomly allocated region of the appropriate size
//! in the address space managed by the specified memory allocation manager.
//!
//! The initial value of the newly-implemented
//! or relocated set of virtual registers is whatever values are
//! currently stored in the
//! memory region now implementing them.
//!
//! Returns a reference to a #uvm_mem_region memory region descriptor
//! if the memory allocation manager was able to allocate a region
//! that can implement the virtual register array.
//! Returns nullptr otherwise.
//!
//! A region implementing a virtual register array
//! must not be released using the uvm_mem_mam::release_region() method.
//! It must be released using the uvm_vreg::release_region() method.
//----------------------------------------------------------------------

uvm_mem_region* uvm_vreg::allocate( unsigned long n,
                                    uvm_mem_mam* mam )
{
  uvm_mem* l_mem = nullptr;
  uvm_mem_region* region = nullptr;

  if(n < 1)
  {
    UVM_ERROR("RegModel",
        "Attempting to implement virtual register '" + get_full_name() +
        "' with a subscript less than one doesn't make sense" );
    return nullptr;
  }

  if(mam == nullptr)
  {
    UVM_ERROR("RegModel",
        "Attempting to implement virtual register '" + get_full_name() +
        "' using a nullptr uvm_mem_mam reference" );
    return nullptr;
  }

  if(m_is_static)
  {
    UVM_ERROR("RegModel",
        "Virtual register '" + get_full_name() +
        "' is static and cannot be dynamically allocated" );
    return nullptr;
  }

  l_mem = mam->get_memory();

  if (l_mem->get_block() != m_parent)
  {
    UVM_ERROR("RegModel",
        "Attempting to allocate virtual register '" + get_full_name() +
        "' on memory '" + l_mem->get_full_name() +
        "' in a different block." );
    return nullptr;
  }

  unsigned int min_incr = (get_n_bytes()-1) / l_mem->get_n_bytes() + 1;
  if (m_incr == 0) m_incr = min_incr;
  if (min_incr < m_incr)
  {
    std::ostringstream str;
    str << "Virtual register '"
        << get_full_name()
        << "' increment is too small ("
        << m_incr
        << "): Each virtual register requires at least "
        << min_incr
        << " locations in memory '"
        << l_mem->get_full_name()
        << "'.";
    UVM_ERROR("RegModel", str.str() );
    return nullptr;
  }

  // Need memory at least of size num_vregs*sizeof(vreg) in bytes.
  region = mam->request_region(n * m_incr * l_mem->get_n_bytes());

  if (region == nullptr)
  {
    UVM_ERROR("RegModel",
      "Could not allocate a memory region for virtual register '" + get_full_name() + "'" );
    return nullptr;
  }

  if (l_mem != nullptr)
  {

    std::ostringstream str;
    str << "Virtual register '"
        << get_full_name()
        << "' is being moved re-allocated from "
        << l_mem->get_full_name()
        << "@ 0x"
        << std::hex << m_offset.to_uint64()
        << " to "
        << l_mem->get_full_name()
        << "@ 0x"
        << std::hex << region->get_start_offset()
        << ".";
    UVM_INFO("RegModel", str.str(), UVM_MEDIUM);
    release_region();
  }

  m_region = region;

  m_mem    = mam->get_memory();
  m_offset = region->get_start_offset();
  m_size   = n;
  //m_incr   = incr; // TODO can be removed as we already updated m_incr?

  m_mem->m_add_vreg(this);

  return region;
}

//----------------------------------------------------------------------
// Member function: get_region
//
//! Get the region where the virtual register array is implemented
//!
//! Returns a reference to the #uvm_mem_region memory region descriptor
//! that implements the virtual register array.
//!
//! Returns nullptr if the virtual registers array
//! is not currently implemented.
//! A region implementing a virtual register array
//! must not be released using the uvm_mem_mam::release_region() method.
//! It must be released using the uvm_vreg::release_region() method.
//----------------------------------------------------------------------

uvm_mem_region* uvm_vreg::get_region() const
{
   return m_region;
}

//----------------------------------------------------------------------
// Member function: release_region
//
//! Dynamically un-implement a virtual register array
//!
//! Release the memory region used to implement a virtual register array
//! and return it to the pool of available memory
//! that can be allocated by the memory's default allocation manager.
//! The virtual register array is subsequently considered as unimplemented
//! and can no longer be accessed.
//!
//! Statically-implemented virtual registers cannot be released.
//----------------------------------------------------------------------

void uvm_vreg::release_region()
{
  if(m_is_static)
  {
    UVM_ERROR("RegModel",
      "Virtual register '" + get_full_name() +
      "' is static and cannot be dynamically released");
    return;
  }

  if(m_mem != nullptr)
    m_mem->m_delete_vreg(this);

  if(m_region != nullptr)
    m_region->release_region();

  m_region = nullptr;
  m_mem    = nullptr;
  m_size   = 0;
  m_offset = 0;

  reset();
}

//----------------------------------------------------------------------
// Group: Introspection
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: get_name
//
// Get the simple name
//
// Return the simple object name of this register.
//----------------------------------------------------------------------

// inherted from base class

//----------------------------------------------------------------------
// Member function: get_full_name
//
//! Get the hierarchical name
//!
//! Return the hierarchical name of this register.
//! The base of the hierarchical name is the root block.
//----------------------------------------------------------------------

const std::string uvm_vreg::get_full_name() const
{
  uvm_reg_block* blk = nullptr;
  std::string name;

  name = get_name();

  // Do not include top-level name in full name
  blk = get_block();
  if (blk == nullptr)
    return name;

  if (blk->get_parent() == nullptr)
    return name;

  return m_parent->get_full_name() + "." + name;
}

//----------------------------------------------------------------------
// Member function: get_parent
//
//! Get the parent block
//----------------------------------------------------------------------

uvm_reg_block* uvm_vreg::get_parent() const
{
  return m_parent;
}

//----------------------------------------------------------------------
// Member function: get_memory
//
//! Get the memory where the virtual register array is implemented
//----------------------------------------------------------------------

uvm_mem* uvm_vreg::get_memory() const
{
  return m_mem;
}

//----------------------------------------------------------------------
// Member function: get_n_maps
//
//! Returns the number of address maps this virtual register array is mapped in
//----------------------------------------------------------------------

int uvm_vreg::get_n_maps() const
{
  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel",
        "Cannot call uvm_vreg::get_n_maps() on unimplemented virtual register '" +
        get_full_name() + "'");
    return 0;
  }

  return m_mem->get_n_maps();
}

//----------------------------------------------------------------------
// Member function: is_in_map
//
//! Return TRUE if this virtual register array is in the specified address \p map
//----------------------------------------------------------------------

bool uvm_vreg::is_in_map( uvm_reg_map* map ) const
{
  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel",
        "Cannot call uvm_vreg::is_in_map() on unimplemented virtual register '" +
        get_full_name() + "'." );
  return false;
  }

  return m_mem->is_in_map(map);
}

//----------------------------------------------------------------------
// Member function: get_maps
//
//! Returns all of the address \p maps where this virtual register array is mapped
//----------------------------------------------------------------------

void uvm_vreg::get_maps( std::vector<uvm_reg_map*>& maps ) const
{
  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel",
      "Cannot call uvm_vreg::get_maps() on unimplemented virtual register '" +
      get_full_name() + "'." );
    return;
  }

  m_mem->get_maps(maps);
}

//----------------------------------------------------------------------
// Member function: get_rights
//
//! Returns the access rights of this virtual register array
//!
//! Returns "RW", "RO" or "WO".
//! The access rights of a virtual register array is always "RW",
//! unless it is implemented in a shared memory
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

std::string uvm_vreg::get_rights( uvm_reg_map* map ) const
{
  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel",
      "Cannot call uvm_vreg::get_rights() on unimplemented virtual register '" +
      get_full_name() + "'." );
    return "RW";
  }

  return m_mem->get_rights(map);
}

//----------------------------------------------------------------------
// Member function: get_access
//
//!! Returns the access policy of the virtual register array
//!! when written and read via an address map.
//!!
//!! If the memory implementing the virtual register array
//!! is mapped in more than one address map,
//!! an address \p map must be specified.
//!! If access restrictions are present when accessing a memory
//!! through the specified address map, the access mode returned
//!! takes the access restrictions into account.
//!! For example, a read-write memory accessed
//!! through an address map with read-only restrictions would return "RO".
//----------------------------------------------------------------------


std::string uvm_vreg::get_access( uvm_reg_map* map ) const
{
  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel", "Cannot call uvm_vreg::get_rights() on unimplemented virtual register '" +
       get_full_name() + "'." );
    return "RW";
  }

  return m_mem->get_access(map);
}

//----------------------------------------------------------------------
// Member function: get_size
//
//! Returns the size of the virtual register array.
//----------------------------------------------------------------------

unsigned int uvm_vreg::get_size() const
{
   if (m_size == 0)
   {
      UVM_ERROR("RegModel",
        "Cannot call uvm_vreg::get_size() on unimplemented virtual register '" +
        get_full_name() + "'." );
      return 0;
   }

   return m_size;
}

//----------------------------------------------------------------------
// Member function: get_n_bytes
//
//! Returns the width, in bytes, of a virtual register.
//!
//! The width of a virtual register is always a multiple of the width
//! of the memory locations used to implement it.
//! For example, a virtual register containing two 1-byte fields
//! implemented in a memory with 4-bytes memory locations is 4-byte wide.
//----------------------------------------------------------------------

unsigned int uvm_vreg::get_n_bytes() const
{
  return ((m_n_bits-1) / 8) + 1;
}

//----------------------------------------------------------------------
// Member function: get_n_memlocs
//
// Returns the number of memory locations used
// by a single virtual register.
//----------------------------------------------------------------------


unsigned int uvm_vreg::get_n_memlocs() const
{
  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel",
      "Cannot call uvm_vreg::get_n_memlocs() on unimplemented virtual register '" +
      get_full_name() + "'." );
    return 0;
  }

  return (get_n_bytes()-1) / m_mem->get_n_bytes() + 1;
}

//----------------------------------------------------------------------
// Member function: get_incr
//
//! Returns the number of memory locations
//! between two individual virtual registers in the same array.
//----------------------------------------------------------------------

unsigned int uvm_vreg::get_incr() const
{
  if (m_incr == 0)
  {
    UVM_ERROR("RegModel",
       "Cannot call uvm_vreg::get_incr() on unimplemented virtual register '" +
       get_full_name() + "'." );
    return 0;
  }

  return m_incr;
}

//----------------------------------------------------------------------
// Member function: get_fields
//
//! Return the virtual fields in this virtual register
//!
//! Fills the specified array with the abstraction class
//! for all of the virtual fields contained in this virtual register.
//! Fields are ordered from least-significant position to most-significant
//! position within the register.
//----------------------------------------------------------------------

void uvm_vreg::get_fields( std::vector<uvm_vreg_field*>& fields ) const
{
  for( unsigned int i = 0; i < m_fields.size(); i++)
      fields.push_back(m_fields[i]);
}
//----------------------------------------------------------------------
// Member function: get_field_by_name
//
//! Return the named virtual field in this virtual register
//!
//! Finds a virtual field with the specified name in this virtual register
//! and returns its abstraction class.
//! If no fields are found, returns nullptr.
//----------------------------------------------------------------------

uvm_vreg_field* uvm_vreg::get_field_by_name( const std::string& name ) const
{
   for ( unsigned int i = 0; i < m_fields.size(); i++ )
      if (m_fields[i]->get_name() == name)
         return m_fields[i];

   UVM_WARNING("RegModel",
     "Unable to locate field '" + name + "' in virtual register '" +
     get_full_name() + "'." );
   return nullptr;
}
//----------------------------------------------------------------------
// Member function: get_offset_in_memory
//
//! Returns the offset of a virtual register
//!
//! Returns the base offset of the specified virtual register,
//! in the overall address space of the memory
// !that implements the virtual register array.
//----------------------------------------------------------------------


uvm_reg_addr_t uvm_vreg::get_offset_in_memory( unsigned long idx ) const
{
  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel",
        "Cannot call uvm_vreg::get_offset_in_memory() on unimplemented virtual register '" +
        get_full_name() + "'." );
    return 0;
  }

  return m_offset + idx * m_incr;
}

//----------------------------------------------------------------------
// Member function: get_address
//
//! Returns the base external physical address of a virtual register
//!
//! Returns the base external physical address of the specified
//! virtual register if accessed through the specified address \p map.
//!
//! If no address map is specified and the memory implementing
//! the virtual register array is mapped in only one
//! address map, that address map is used. If the memory is mapped
//! in more than one address map, the default address map of the
//! parent block is used.
//!
//! If an address map is specified and
//! the memory is not mapped in the specified
//! address map, an error message is issued.
//----------------------------------------------------------------------

uvm_reg_addr_t uvm_vreg::get_address( unsigned long idx,
                                      const uvm_reg_map* map ) const
{
  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel",
      "Cannot get address of of unimplemented virtual register '" + get_full_name() + "'." );
    return 0;
  }

  return m_mem->get_address( get_offset_in_memory(idx), map);
}
//----------------------------------------------------------------------
// Group: HDL Access
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: write
//
//! Write the specified value in a virtual register
//!
//! Write \p value in the DUT memory location(s) that implements
//! the virtual register array that corresponds to this
//! abstraction class instance using the specified access
//! \p path.
//!
//! If the memory implementing the virtual register array
//! is mapped in more than one address map,
//! an address \p map must be
//! specified if a physical access is used (front-door access).
//!
//! The operation is eventually mapped into set of
//! memory-write operations at the location where the virtual register
//! specified by \p idx in the virtual register array is implemented.
//----------------------------------------------------------------------

void uvm_vreg::write( unsigned long idx,
                      uvm_status_e& status, // output
                      uvm_reg_data_t value,
                      uvm_path_e path,
                      uvm_reg_map* map,
                      uvm_sequence_base* parent,
                      uvm_object* extension,
                      const std::string& fname,
                      int lineno )
{
  uvm_vreg_cb_iter* cbs = new uvm_vreg_cb_iter(this);

  uvm_reg_addr_t addr;
  uvm_reg_data_t tmp;
  uvm_reg_data_t msk;
  int lsb;

  m_write_in_progress = true;
  m_fname = fname;
  m_lineno = lineno;

  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel",
        "Cannot write to unimplemented virtual register '" +
        get_full_name() + "'." );
    status = UVM_NOT_OK;
    return;
  }

  if (path == UVM_DEFAULT_PATH)
    path = m_parent->get_default_path();

  for( unsigned int i = 0; i < m_fields.size(); i++ )
  {
    uvm_vreg_field_cb_iter* cbsf = new uvm_vreg_field_cb_iter(m_fields[i]);
    uvm_vreg_field* f = m_fields[i];

    lsb = f->get_lsb_pos_in_register();
    msk = uvm_mask_size(f->get_n_bits()) << lsb;
    tmp = (value & msk) >> lsb;

    f->pre_write(idx, tmp, path, map);
    for (uvm_vreg_field_cbs* cb = cbsf->first(); cb != nullptr;
      cb = cbsf->next())
    {
      cb->m_fname = m_fname;
      cb->m_lineno = m_lineno;
      cb->pre_write(f, idx, tmp, path, map);
    }

    value = (value & ~msk) | (tmp << lsb);

    delete cbsf;
  }

  pre_write(idx, value, path, map);

  for( uvm_vreg_cbs* cb = cbs->first(); cb != nullptr;
    cb = cbs->next())
  {
    cb->m_fname = m_fname;
    cb->m_lineno = m_lineno;
    cb->pre_write(this, idx, value, path, map);
  }

  addr = m_offset + (idx * m_incr);

  lsb = 0;
  status = UVM_IS_OK;

  for( unsigned int i = 0; i < get_n_memlocs(); i++)
  {
    uvm_status_e s;
    msk = uvm_mask_size(m_mem->get_n_bytes()*8) << lsb;
    tmp = (value & msk) >> lsb;
    int prior = -1;
    m_mem->write(s, addr + i, tmp, path, map ,parent, prior, extension, fname, lineno);
    if (s != UVM_IS_OK && s != UVM_HAS_X)
      status = s;
    lsb += m_mem->get_n_bytes() * 8;
  }

  for( uvm_vreg_cbs* cb = cbs->first(); cb != nullptr;
    cb = cbs->next())
  {
    cb->m_fname = m_fname;
    cb->m_lineno = m_lineno;
    cb->post_write(this, idx, value, path, map, status);
  }

  post_write(idx, value, path, map, status);

  for ( unsigned int i = 0; i < m_fields.size(); i++ )
  {
    uvm_vreg_field_cb_iter* cbsf = new uvm_vreg_field_cb_iter(m_fields[i]);
    uvm_vreg_field* f = m_fields[i];

    lsb = f->get_lsb_pos_in_register();
    msk = uvm_mask_size(f->get_n_bits()) << lsb;
    tmp = (value & msk) >> lsb;

    for (uvm_vreg_field_cbs* cb = cbsf->first(); cb != nullptr;
        cb = cbsf->next())
    {
      cb->m_fname = m_fname;
      cb->m_lineno = m_lineno;
      cb->post_write(f, idx, tmp, path, map, status);
    }

    f->post_write(idx, tmp, path, map, status);

    value = (value & ~msk) | (tmp << lsb);

    delete cbsf;
  }

  std::ostringstream inf;
  inf << "Wrote virtual register '"
      << get_full_name()
      << "'["
      << idx
      << "] via "
      << ((path == UVM_FRONTDOOR) ? "frontdoor" : "backdoor")
      << " with: 0x"
      << std::hex << value.to_uint64()
      << ".";
  UVM_INFO("RegModel", inf.str(), UVM_MEDIUM);

  m_write_in_progress = false;
  m_fname = "";
  m_lineno = 0;

  delete cbs;
}

//----------------------------------------------------------------------
// Member function: read
//
//! Read the current value from a virtual register
//!
//! Read from the DUT memory location(s) that implements
//! the virtual register array that corresponds to this
//! abstraction class instance using the specified access
//! \p path and return the readback \p value.
//!
//! If the memory implementing the virtual register array
//! is mapped in more than one address map,
//! an address \p map must be
//! specified if a physical access is used (front-door access).
//!
//! The operation is eventually mapped into set of
//! memory-read operations at the location where the virtual register
//! specified by \p idx in the virtual register array is implemented.
//----------------------------------------------------------------------


void uvm_vreg::read( unsigned long idx,
                     uvm_status_e& status, // output
                     uvm_reg_data_t& value, // output
                     uvm_path_e path,
                     uvm_reg_map* map,
                     uvm_sequence_base* parent,
                     uvm_object* extension,
                     const std::string& fname,
                     int lineno )
{
  uvm_vreg_cb_iter* cbs = new uvm_vreg_cb_iter(this);

  uvm_reg_addr_t addr;
  uvm_reg_data_t tmp;
  uvm_reg_data_t msk;

  int lsb;

  m_read_in_progress = true;

  m_fname = fname;
  m_lineno = lineno;

  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel",
        "Cannot read from unimplemented virtual register '" +
        get_full_name() + "'." );
    status = UVM_NOT_OK;
    return;
  }

  if (path == UVM_DEFAULT_PATH)
    path = m_parent->get_default_path();

  for( unsigned int i = 0; i < m_fields.size(); i++)
  {
    uvm_vreg_field_cb_iter* cbsf = new uvm_vreg_field_cb_iter(m_fields[i]);
    uvm_vreg_field* f = m_fields[i];

    f->pre_read(idx, path, map);

    for( uvm_vreg_field_cbs* cb = cbsf->first(); cb != nullptr;
      cb = cbsf->next() )
    {
      cb->m_fname = m_fname;
      cb->m_lineno = m_lineno;
      cb->pre_read(f, idx, path, map);
    }

    delete cbsf;
  }

  pre_read(idx, path, map);

  for( uvm_vreg_cbs* cb = cbs->first(); cb != nullptr;
    cb = cbs->next() )
  {
    cb->m_fname = m_fname;
    cb->m_lineno = m_lineno;
    cb->pre_read(this, idx, path, map);
  }

  addr = m_offset + (idx * m_incr);

  lsb = 0;
  value = 0;
  status = UVM_IS_OK;

  for( unsigned int i = 0; i < get_n_memlocs(); i++ )
  {
    uvm_status_e s;
    int prior = -1;
    m_mem->read(s, addr + i, tmp, path, map, parent, prior, extension, fname, lineno);
    if (s != UVM_IS_OK && s != UVM_HAS_X)
      status = s;

    value |= tmp << lsb;
    lsb += m_mem->get_n_bytes() * 8;
  }

  for (uvm_vreg_cbs* cb = cbs->first(); cb != nullptr;
    cb = cbs->next())
  {
    cb->m_fname = m_fname;
    cb->m_lineno = m_lineno;
    cb->post_read(this, idx, value, path, map, status);
  }

  post_read(idx, value, path, map, status);

  for( unsigned int i = 0; i < m_fields.size(); i++ )
  {
    uvm_vreg_field_cb_iter* cbsf = new uvm_vreg_field_cb_iter(m_fields[i]);
    uvm_vreg_field* f = m_fields[i];

    lsb = f->get_lsb_pos_in_register();

    msk = uvm_mask_size(f->get_n_bits()) << lsb;
    tmp = (value & msk) >> lsb;

    for (uvm_vreg_field_cbs* cb = cbsf->first(); cb != nullptr;
      cb = cbsf->next())
    {
      cb->m_fname = m_fname;
      cb->m_lineno = m_lineno;
      cb->post_read(f, idx, tmp, path, map, status);
    }

    f->post_read(idx, tmp, path, map, status);

    value = (value & ~msk) | (tmp << lsb);

    delete cbsf;
  }

  std::ostringstream inf;
  inf << "Read virtual register '"
      << get_full_name()
      << "'["
      << idx
      << "] via "
      << ((path == UVM_FRONTDOOR) ? "frontdoor" : "backdoor")
      << ": 0x"
      << std::hex << value.to_uint64()
      << ".";
  UVM_INFO("RegModel", inf.str(), UVM_MEDIUM);

  m_read_in_progress = false;
  m_fname = "";
  m_lineno = 0;

  delete cbs;
}


//----------------------------------------------------------------------
// Member function: poke
//
//! Deposit the specified value in a virtual register
//!
//! Deposit \p value in the DUT memory location(s) that implements
//! the virtual register array that corresponds to this
//! abstraction class instance using the memory backdoor access.
//!
//! The operation is eventually mapped into set of
//! memory-poke operations at the location where the virtual register
//! specified by \p idx in the virtual register array is implemented.
//----------------------------------------------------------------------

void uvm_vreg::poke( unsigned long idx,
                     uvm_status_e& status, // output
                     uvm_reg_data_t value,
                     uvm_sequence_base* parent,
                     uvm_object* extension,
                     const std::string& fname,
                     int lineno )
{
  uvm_reg_addr_t addr;
  uvm_reg_data_t tmp;
  uvm_reg_data_t msk;
  int lsb;

  m_fname = fname;
  m_lineno = lineno;

  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel",
      "Cannot poke in unimplemented virtual register '" +
      get_full_name() + "'." );

    status = UVM_NOT_OK;
    return;
  }

  addr = m_offset + (idx * m_incr);

  lsb = 0;
  status = UVM_IS_OK;

  for( unsigned int i = 0; i < get_n_memlocs(); i++ )
  {
    // uvm_status_e s; // TODO: this original code as no meaning, as the peek directly updates the status

    msk = uvm_mask_size(m_mem->get_n_bytes() * 8) << lsb;
    tmp = (value & msk) >> lsb;

    m_mem->poke(status, addr + i, tmp, "", parent, extension, fname, lineno);

    //if (s != UVM_IS_OK && s != UVM_HAS_X)
    //  status = s;

    lsb += m_mem->get_n_bytes() * 8;
  }

  std::ostringstream inf;
  inf << "Poked virtual register '"
      << get_full_name()
      << "'["
      << std::dec << idx
      << "] with: 0x"
      << std::hex << value.to_uint64()
      << ".";
  UVM_INFO("RegModel", inf.str(), UVM_MEDIUM);

  m_fname = "";
  m_lineno = 0;
}

//----------------------------------------------------------------------
// Member function: peek
//
//! Sample the current value in a virtual register
//!
//! Sample the DUT memory location(s) that implements
//! the virtual register array that corresponds to this
//! abstraction class instance using the memory backdoor access,
//! and return the sampled \p value.
//!
//! The operation is eventually mapped into set of
//! memory-peek operations at the location where the virtual register
//! specified by \p idx in the virtual register array is implemented.
//----------------------------------------------------------------------

void uvm_vreg::peek( unsigned long idx,
                     uvm_status_e& status, // output
                     uvm_reg_data_t& value, // output
                     uvm_sequence_base* parent,
                     uvm_object * extension,
                     const std::string& fname,
                     int lineno )
{
  uvm_reg_addr_t addr;
  uvm_reg_data_t tmp;
  uvm_reg_data_t msk;
  int lsb;

  m_fname = fname;
  m_lineno = lineno;

  if (m_mem == nullptr)
  {
    UVM_ERROR("RegModel",
      "Cannot peek in from unimplemented virtual register '" +
      get_full_name() + "'." );
    status = UVM_NOT_OK;
    return;
  }

  addr = m_offset + (idx * m_incr);

  lsb = 0;
  value = 0;
  status = UVM_IS_OK;

  for( unsigned int i = 0; i < get_n_memlocs(); i++ )
  {
    //uvm_status_e s; // TODO: this original code as no meaning, as the peek directly updates the status

    m_mem->peek(status, addr + i, tmp, "", parent, extension, fname, lineno);

    //if (s != UVM_IS_OK && s != UVM_HAS_X)
    //  status = s;

    value |= tmp << lsb;
    lsb += m_mem->get_n_bytes() * 8;
  }

  std::ostringstream inf;
  inf << "Peeked virtual register '"
      << get_full_name()
      << "'["
      << idx
      << "]: "
      << std::hex <<value.to_uint64()
      << ".";

  UVM_INFO("RegModel", inf.str(), UVM_MEDIUM);

  m_fname = "";
  m_lineno = 0;
}

//----------------------------------------------------------------------
// Member function: reset
//
//! Reset the access semaphore
//!
//! Reset the semaphore that prevents concurrent access
//! to the virtual register.
//! This semaphore must be explicitly reset if a thread accessing
//! this virtual register array was killed in before the access
//! was completed
//----------------------------------------------------------------------

void uvm_vreg::reset( const std::string& kind )
{
   // Put back a key in the semaphore if it is checked out
   // in case a thread was killed during an operation

   // TODO
   //void'(this.atomic.try_get(1));
   //this.atomic.put(1);
}

//----------------------------------------------------------------------
// Group: Callbacks
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: pre_write
//
//! Called before virtual register write.
//!
//! If the specified data value, access \p path or address \p map are modified,
//! the updated data value, access path or address map will be used
//! to perform the virtual register operation.
//!
//! The registered callback methods are invoked after the invocation
//! of this method.
//! All register callbacks are executed after the corresponding
//! field callbacks
//! The pre-write virtual register and field callbacks are executed
//! before the corresponding pre-write memory callbacks
//----------------------------------------------------------------------

void uvm_vreg::pre_write( unsigned long idx,
                          uvm_reg_data_t& wdat,
                          uvm_path_e& path,
                          uvm_reg_map*& map )
{}

//----------------------------------------------------------------------
// Member function: post_write
//
//! Called after virtual register write.
//!
//! If the specified \p status is modified,
//! the updated status will be
//! returned by the virtual register operation.
//!
//! The registered callback methods are invoked before the invocation
//! of this method.
//! All register callbacks are executed before the corresponding
//! field callbacks
//! The post-write virtual register and field callbacks are executed
//! after the corresponding post-write memory callbacks
//----------------------------------------------------------------------

void uvm_vreg::post_write( unsigned long idx,
                           uvm_reg_data_t wdat,
                           uvm_path_e path,
                           uvm_reg_map* map,
                           uvm_status_e& status )
{}

//----------------------------------------------------------------------
// Member function: pre_read
//
//! Called before virtual register read.
//!
//! If the specified access \p path or address \p map are modified,
//! the updated access path or address map will be used to perform
//! the register operation.
//!
//! The registered callback methods are invoked after the invocation
//! of this method.
//! All register callbacks are executed after the corresponding
//! field callbacks
//! The pre-read virtual register and field callbacks are executed
//! before the corresponding pre-read memory callbacks
//----------------------------------------------------------------------

void uvm_vreg::pre_read( unsigned long idx,
                         uvm_path_e& path,
                         uvm_reg_map*& map )
{}

//----------------------------------------------------------------------
// Member function: post_read
//
//! Called after virtual register read.
//!
//! If the specified readback data or \p status is modified,
//! the updated readback data or status will be
//! returned by the register operation.
//!
//! The registered callback methods are invoked before the invocation
//! of this method.
//! All register callbacks are executed before the corresponding
//! field callbacks
//! The post-read virtual register and field callbacks are executed
//! after the corresponding post-read memory callbacks
//----------------------------------------------------------------------

void uvm_vreg::post_read( unsigned long idx,
                          uvm_reg_data_t& rdat,
                          uvm_path_e path,
                          uvm_reg_map* map,
                          uvm_status_e& status )
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

void uvm_vreg::set_parent( uvm_reg_block* parent )
{
  m_parent = parent;
}

//----------------------------------------------------------------------
// Member function: m_lock_model
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_vreg::m_lock_model()
{
  if (m_locked) return;
  m_locked = true;
}

//----------------------------------------------------------------------
// Member function: add_field
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_vreg::add_field( uvm_vreg_field* field )
{
  unsigned int offset;
  unsigned int idx;
  unsigned int i;

  if (m_locked)
  {
    UVM_ERROR("RegModel",
      "Cannot add virtual field to locked virtual register model");
    return;
  }

  if (field == nullptr)
    UVM_FATAL("RegModel", "Attempting to register nullptr virtual field");

  // Store fields in LSB to MSB order
  offset = field->get_lsb_pos_in_register();

  idx = -1;  // Mark for invalid index into m_fields
  i = 0;

  for( m_fields_itt it = m_fields.begin();
       it != m_fields.end();
       it++)
  {
    i++;
    if (offset < (*it)->get_lsb_pos_in_register())
    {
      m_fields.insert(it, field);
      idx = i;
      break;
    }
  }
  if (idx >= m_fields.size())
  {
    m_fields.push_back(field);
    idx = (int)m_fields.size()-1;
  }

  m_n_used_bits += field->get_n_bits();

  // Check if there are too many fields in the register
  if (m_n_used_bits > m_n_bits)
  {
    std::ostringstream str;
    str << "Virtual fields use more bits ("
        << m_n_used_bits
        << ") than available in virtual register '"
        << get_full_name()
        << "' ("
        << m_n_bits
        << ").";
    UVM_ERROR("RegModel", str.str() );
  }

  // Check if there are overlapping fields
  if (idx > 0)
  {
    if (m_fields[idx-1]->get_lsb_pos_in_register() +
        m_fields[idx-1]->get_n_bits() > offset)
    {
      std::ostringstream str;
      str << "Field "
          << m_fields[idx-1]->get_name()
          << " overlaps field "
          << field->get_name()
          << " in virtual register '"
          << get_full_name()
          << "'.";
      UVM_ERROR("RegModel", str.str());
    }
  }

  if (idx < m_fields.size()-1)
  {
    if (offset + field->get_n_bits() >
    m_fields[idx+1]->get_lsb_pos_in_register())
    {
      std::ostringstream str;
      str << "Field "
          << field->get_name()
          << " overlaps field "
          << m_fields[idx+1]->get_name()
          << " in virtual register '"
          << get_full_name()
          << "'.";
      UVM_ERROR("RegModel", str.str());
    }
  }
}

//----------------------------------------------------------------------
// Member function: m_atomic_check
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_vreg::m_atomic_check( bool on )
{
  /*
  if (on)
     m_atomic.lock(); // get
  else
  {
    // Maybe a key was put back in by a spurious call to reset()
    m_atomic.trylock();
    m_atomic.unlock(); // put
  }
  */
}

//----------------------------------------------------------------------
// Member function: get_block
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_block* uvm_vreg::get_block() const
{
  return m_parent;
}

//----------------------------------------------------------------------
// Member function: do_print
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_vreg::do_print( const uvm_printer& printer ) const
{
  uvm_object::do_print(printer);
  printer.print_generic( "initiator",
                         m_parent->get_type_name(),
                         -1,
                         convert2string());
}

//----------------------------------------------------------------------
// Member function: convert2string
//
// Implementation defined
//----------------------------------------------------------------------

std::string uvm_vreg::convert2string() const
{
  std::string res_str;
  std::ostringstream str;

  str << "Virtual register %s -- " << get_full_name();

  if (m_size == 0)
    str << " unimplemented";
  else
  {
    std::vector<uvm_reg_map*> maps;
    m_mem->get_maps(maps);

    str << "[" << m_size << "] in " << m_mem->get_full_name()
        << "[ 0x" << std::hex << m_offset.to_uint64() << " + 0x " << std::hex << m_incr << " ]\n";

    for( unsigned int i = 0; i < maps.size(); i++ )
    {
      uvm_reg_addr_t addr0 = get_address(0, maps[i]);
      uvm_reg_addr_t addr1 = get_address(1, maps[i])- addr0;

      str << "  Address in map '"
          << maps[i]->get_full_name()
          << "' -- @ 0x"
          << std::hex << addr0.to_uint64()
          << " + 0x"
          << std::hex << addr1.to_uint64();
    }
  }

  for( unsigned int i = 0; i < m_fields.size(); i++ )
      str << std::endl << m_fields[i]->convert2string();

  return str.str();
}

//----------------------------------------------------------------------
// member function: clone
//
// Implementation defined
//----------------------------------------------------------------------

uvm_object* uvm_vreg::clone()
{
  UVM_FATAL("RegModel","RegModel virtual register cannot be cloned");
  return nullptr;
}

//----------------------------------------------------------------------
// member function: do_copy
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_vreg::do_copy( const uvm_object& rhs )
{
  UVM_FATAL("RegModel","RegModel virtual register cannot be copied");
  // TODO disable copy constructor
}


//----------------------------------------------------------------------
// member function: do_compare
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_vreg::do_compare( const uvm_object& rhs,
                          const uvm_comparer* comparer) const
{
  UVM_WARNING("RegModel","RegModel virtual register cannot be compared");
  return false;
}

//----------------------------------------------------------------------
// member function: do_pack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_vreg::do_pack( uvm_packer& packer ) const
{
  UVM_WARNING("RegModel","RegModel virtual register cannot be packed");
}

//----------------------------------------------------------------------
// member function: do_unpack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_vreg::do_unpack( uvm_packer& packer )
{
  UVM_WARNING("RegModel","RegModel virtual register cannot be unpacked");
}

} // namespace uvm
