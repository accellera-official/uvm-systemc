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

#include "uvmsc/reg/uvm_vreg_field.h"
#include "uvmsc/reg/uvm_mem.h"
#include "uvmsc/reg/uvm_vreg.h"
#include "uvmsc/reg/uvm_reg_block.h"
#include "uvmsc/reg/uvm_vreg_field_cbs.h"

namespace uvm {

//----------------------------------------------------------------------
// Initialization of static data members
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Constructor
//
//! Create a new virtual field instance
//!
//! This method should not be used directly.
//! The uvm_vreg_field::type_id::create() method should be used instead.
//----------------------------------------------------------------------

uvm_vreg_field::uvm_vreg_field( const std::string& name ) : uvm_object(name)
{
  m_parent = nullptr;
  m_lsb = 0;
  m_size = 0;
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
//! Specify the \p parent virtual register of this virtual field, its
//! \p size in bits, and the position of its least-significant bit
//! within the virtual register relative to the least-significant bit
//! of the virtual register.
//----------------------------------------------------------------------

void uvm_vreg_field::configure( uvm_vreg* parent,
                                unsigned int size,
                                unsigned int lsb_pos )
{
  m_parent = parent;
  if (size == 0)
  {
    UVM_ERROR("RegModel", "Virtual field '" + get_full_name() + "' cannot have 0 bits");
    size = 1;
  }

  if (size > UVM_REG_DATA_WIDTH)
  {
    std::ostringstream str;
    str << "Virtual field '"
        << get_full_name()
        << "' cannot have more than "
        << UVM_REG_DATA_WIDTH
        << " bits.";
    UVM_ERROR("RegModel", str.str());

    size = UVM_REG_DATA_WIDTH;
  }

  m_size   = size;
  m_lsb    = lsb_pos;

  m_parent->add_field(this);
}

//----------------------------------------------------------------------
// Group: Introspection
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: get_name
//
//! Get the simple name
//!
//! Return the simple object name of this virtual field
//----------------------------------------------------------------------

// inherited from base class

//----------------------------------------------------------------------
// Member function: get_full_name
//
//! Get the hierarchical name
//!
//! Return the hierarchical name of this virtual field
//! The base of the hierarchical name is the root block.
//----------------------------------------------------------------------

const std::string uvm_vreg_field::get_full_name() const
{
   return m_parent->get_full_name() + "." + get_name();
}

//----------------------------------------------------------------------
// Member function: get_parent
//
//! Get the parent virtual register
//----------------------------------------------------------------------

uvm_vreg* uvm_vreg_field::get_parent() const
{
  return m_parent;
}


//----------------------------------------------------------------------
// Member function: get_lsb_pos_in_register
//
//! Return the position of the virtual field
//!
//! Returns the index of the least significant bit of the virtual field
//! in the virtual register that instantiates it.
//! An offset of 0 indicates a field that is aligned with the
//! least-significant bit of the register.
//----------------------------------------------------------------------

unsigned int uvm_vreg_field::get_lsb_pos_in_register() const
{
  return m_lsb;
}

//----------------------------------------------------------------------
// Member function: get_n_bits
//
//! Returns the width, in bits, of the virtual field.
//----------------------------------------------------------------------

unsigned int uvm_vreg_field::get_n_bits() const
{
  return m_size;
}

//----------------------------------------------------------------------
// Member function: get_access
//
//! Returns the access policy of the virtual field register
//! when written and read via an address map.
//!
//! If the memory implementing the virtual field
//! is mapped in more than one address map,
//! an address \p map must be specified.
//! If access restrictions are present when accessing a memory
//! through the specified address map, the access mode returned
//! takes the access restrictions into account.
//! For example, a read-write memory accessed
//! through an address map with read-only restrictions would return "RO".
//----------------------------------------------------------------------

std::string uvm_vreg_field::get_access( uvm_reg_map* map ) const
{
  if (m_parent->get_memory() == nullptr)
  {
    UVM_ERROR("RegModel", "Cannot call uvm_vreg_field::get_rights() on unimplemented virtual field '" +
        get_full_name() + "'.");
    return "RW";
  }

  return m_parent->get_access(map);
}

//----------------------------------------------------------------------
// Group: HDL Access
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: write
//
//! Write the specified value in a virtual field
//!
//! Write ~value~ in the DUT memory location(s) that implements
//! the virtual field that corresponds to this
//! abstraction class instance using the specified access
//! \p path.
//!
//! If the memory implementing the virtual register array
//! containing this virtual field
//! is mapped in more than one address map,
//! an address \p map must be
//! specified if a physical access is used (front-door access).
//!
//! The operation is eventually mapped into
//! memory read-modify-write operations at the location
//! where the virtual register
//! specified by \p idx in the virtual register array is implemented.
//! If a backdoor is available for the memory implementing the
//! virtual field, it will be used for the memory-read operation.
//----------------------------------------------------------------------

void uvm_vreg_field::write( unsigned long idx,
                            uvm_status_e& status, // output
                            uvm_reg_data_t value,
                            uvm_path_e path,
                            uvm_reg_map* map,
                            uvm_sequence_base* parent,
                            uvm_object* extension,
                            const std::string& fname,
                            int lineno )
{
  uvm_reg_data_t  tmp;
  uvm_reg_data_t  segval;
  uvm_reg_addr_t  segoff;
  uvm_status_e st;

  int flsb, fmsb, rmwbits;
  int segsiz, segn;
  uvm_mem* mem = nullptr;
  uvm_path_e rm_path;

  uvm_vreg_field_cb_iter* cbs = new uvm_vreg_field_cb_iter(this);

  m_fname = fname;
  m_lineno = lineno;

  int prior = -1;

  m_write_in_progress = true;
  mem = m_parent->get_memory();

  if (mem == nullptr)
  {
    UVM_ERROR("RegModel", "Cannot call uvm_vreg_field::write() on unimplemented virtual register '" +
        this->get_full_name() + "'.");
    status = UVM_NOT_OK;
    return;
  }

  if (path == UVM_DEFAULT_PATH)
  {
    uvm_reg_block* blk = m_parent->get_block();
    path = blk->get_default_path();
  }

  status = UVM_IS_OK;

  m_parent->m_atomic_check(true);

  if (value >> m_size)
  {
    std::ostringstream str;
    str << "Writing value 0x"
        << std::hex << value.to_uint64()
        << " that is greater than field '"
        << get_full_name()
        << "' size ("
        << get_n_bits()
        << " bits)";
    UVM_WARNING("RegModel", str.str());

    value &= value & uvm_mask_size(m_size);
  }

  tmp = 0;

  pre_write(idx, value, path, map);

  for( uvm_vreg_field_cbs* cb = cbs->first(); cb != nullptr;
      cb = cbs->next())
  {
    cb->m_fname = m_fname;
    cb->m_lineno = m_lineno;
    cb->pre_write(this, idx, value, path, map);
  }

  segsiz = mem->get_n_bytes() * 8;
  flsb    = get_lsb_pos_in_register();
  segoff  = m_parent->get_offset_in_memory(idx) + (flsb / segsiz);

  // Favor backdoor read to frontdoor read for the RMW operation
  rm_path = UVM_DEFAULT_PATH;
  if (mem->get_backdoor() != nullptr)
    rm_path = UVM_BACKDOOR;

  // Any bits on the LSB side we need to RMW?
  rmwbits = flsb % segsiz;

  // Total number of memory segment in this field
  segn = (rmwbits + get_n_bits() - 1) / segsiz + 1;

  if (rmwbits > 0)
  {
    uvm_reg_addr_t  segn;

    mem->read(st, segoff, tmp, rm_path, map, parent, prior, extension, fname, lineno);

    if (st != UVM_IS_OK && st != UVM_HAS_X)
    {
      std::ostringstream str;
      str << "Unable to read LSB bits in "
          << mem->get_full_name()
          << "[" << segoff << "]"
          << "to for RMW cycle on virtual field "
          << get_full_name()
          << ".";
      UVM_ERROR("RegModel", str.str() );

      status = UVM_NOT_OK;
      m_parent->m_atomic_check(false);
      return;
    }

    value = (value << rmwbits) | (tmp & uvm_mask_size(rmwbits));
  }

  // Any bits on the MSB side we need to RMW?
  fmsb = rmwbits + get_n_bits() - 1;

  rmwbits = (fmsb+1) % segsiz;
  if (rmwbits > 0) {
    if (segn > 0) {
      mem->read(st, segoff + segn - 1, tmp, rm_path, map, parent, prior, extension, fname, lineno);

      if (st != UVM_IS_OK && st != UVM_HAS_X)
      {
        std::ostringstream str;
        str << "Unable to read MSB bits in "
            << mem->get_full_name()
            << "[" << segoff+segn-1
            << "] to for RMW cycle on virtual field "
            << get_full_name()
            << ".";

        UVM_ERROR("RegModel", str.str());
        status = UVM_NOT_OK;
        m_parent->m_atomic_check(false);
        return;
      }
    }
    value |= (tmp & ~uvm_mask_size(rmwbits)) << ((segn-1)*segsiz);
  }

  // Now write each of the segments
  tmp = value;
  for( int i = 0; i < segn; i++)
  {

    mem->write(st, segoff, tmp, path, map, parent, prior, extension, fname, lineno);
    if (st != UVM_IS_OK && st != UVM_HAS_X) status = UVM_NOT_OK;

    segoff++;
    tmp = tmp >> segsiz;
  }

  post_write(idx, value, path, map, status);

  for( uvm_vreg_field_cbs* cb = cbs->first(); cb != nullptr;
      cb = cbs->next())
  {
    cb->m_fname = m_fname;
    cb->m_lineno = m_lineno;
    cb->post_write(this, idx, value, path, map, status);
  }

  m_parent->m_atomic_check(false);

  std::ostringstream inf;
  inf << "Wrote virtual field '"
      << get_full_name()
      << "' ["
      << idx
      << "] via "
      << ((path == UVM_FRONTDOOR) ? "frontdoor" : "backdoor")
      << " with: 0x"
      << std::hex << value.to_uint64();
  UVM_INFO("RegModel", inf.str(), UVM_MEDIUM);

  m_write_in_progress = false;
  m_fname = "";
  m_lineno = 0;

  delete cbs;
}

//----------------------------------------------------------------------
// Member function: read
//
//! Read the current value from a virtual field
//!
//! Read from the DUT memory location(s) that implements
//! the virtual field that corresponds to this
//! abstraction class instance using the specified access
//! \p path, and return the readback \p value.
//!
//! If the memory implementing the virtual register array
//! containing this virtual field
//! is mapped in more than one address map,
//! an address \p map must be
//! specified if a physical access is used (front-door access).
//!
//! The operation is eventually mapped into
//! memory read operations at the location(s)
//! where the virtual register
//! specified by \p idx in the virtual register array is implemented.
//----------------------------------------------------------------------

void uvm_vreg_field::read( unsigned long idx,
                           uvm_status_e& status,
                           uvm_reg_data_t& value,
                           uvm_path_e path,
                           uvm_reg_map* map,
                           uvm_sequence_base* parent,
                           uvm_object* extension,
                           const std::string& fname,
                           int lineno )
{
  uvm_reg_data_t  tmp;
  uvm_reg_data_t  segval;
  uvm_reg_addr_t  segoff;
  uvm_status_e st;

  int flsb, lsb;
  int segsiz, segn;
  uvm_mem* mem = nullptr;

  int prior = -1;

  uvm_vreg_field_cb_iter* cbs = new uvm_vreg_field_cb_iter(this);

  m_fname = fname;
  m_lineno = lineno;

  m_read_in_progress = true;

  mem = m_parent->get_memory();

  if (mem == nullptr)
  {
    std::ostringstream str;
    str << "Cannot call uvm_vreg_field::read() on unimplemented virtual register '"
        << get_full_name()
        << "'";
    UVM_ERROR("RegModel", str.str() );
    status = UVM_NOT_OK;
    return;
  }

  if (path == UVM_DEFAULT_PATH)
  {
    uvm_reg_block* blk = m_parent->get_block();
    path = blk->get_default_path();
  }

  status = UVM_IS_OK;

  m_parent->m_atomic_check(true);

  value = 0;

  pre_read(idx, path, map);

  for( uvm_vreg_field_cbs* cb = cbs->first(); cb != nullptr;
      cb = cbs->next())
  {
    cb->m_fname = m_fname;
    cb->m_lineno = m_lineno;
    cb->pre_read(this, idx, path, map);
  }

  segsiz = mem->get_n_bytes() * 8;
  flsb    = get_lsb_pos_in_register();
  segoff  = m_parent->get_offset_in_memory(idx) + (flsb / segsiz);
  lsb = flsb % segsiz;

  // Total number of memory segment in this field
  segn = (lsb + get_n_bits() - 1) / segsiz + 1;

  // Read each of the segments, MSB first
  segoff += segn - 1;
  for( int i = 0; i < segn; i++)
  {
    value = value << segsiz;

    mem->read(st, segoff, tmp, path, map, parent, prior, extension, fname, lineno);

    if (st != UVM_IS_OK && st != UVM_HAS_X)
      status = UVM_NOT_OK;

    segoff--;
    value |= tmp;
  }

  // Any bits on the LSB side we need to get rid of?
  value = value >> lsb;

  // Any bits on the MSB side we need to get rid of?
  value &= uvm_mask_size(get_n_bits());

  post_read(idx, value, path, map, status);

  for( uvm_vreg_field_cbs* cb = cbs->first(); cb != nullptr;
      cb = cbs->next())
  {
    cb->m_fname = m_fname;
    cb->m_lineno = m_lineno;
    cb->post_read(this, idx, value, path, map, status);
  }

  m_parent->m_atomic_check(false);

  std::ostringstream inf;
  inf << "Read virtual field '"
      << get_full_name()
      << "["
      << idx
      << "] via "
      << ((path == UVM_FRONTDOOR) ? "frontdoor" : "backdoor")
      << ": 0x"
      << std::hex << value.to_uint64();
  UVM_INFO("RegModel", inf.str(), UVM_MEDIUM);

  m_read_in_progress = false;
  m_fname = "";
  m_lineno = 0;

  delete cbs;
}


//----------------------------------------------------------------------
// Member function: poke
//
//! Deposit the specified value in a virtual field
//!
//! Deposit \p value in the DUT memory location(s) that implements
//! the virtual field that corresponds to this
//! abstraction class instance using the specified access
//! \p path.
//!
//! The operation is eventually mapped into
//! memory peek-modify-poke operations at the location
//! where the virtual register
//! specified by \p idx in the virtual register array is implemented.
//----------------------------------------------------------------------

void uvm_vreg_field::poke( unsigned long idx,
                           uvm_status_e& status, //output
                           uvm_reg_data_t value,
                           uvm_sequence_base* parent,
                           uvm_object* extension,
                           const std::string& fname,
                           int lineno )
{
  uvm_reg_data_t  tmp;
  uvm_reg_data_t  segval;
  uvm_reg_addr_t  segoff;
  uvm_status_e st;

  int flsb, fmsb, rmwbits;
  int segsiz, segn;
  uvm_mem* mem = nullptr;

  m_fname = fname;
  m_lineno = lineno;

  mem = m_parent->get_memory();

  if (mem == nullptr)
  {
    UVM_ERROR("RegModel",
        "Cannot call uvm_vreg_field::poke() on unimplemented virtual register '" +
        get_full_name() + "'." );
    status = UVM_NOT_OK;
    return;
  }

  status = UVM_IS_OK;

  m_parent->m_atomic_check(true);

  if (value >> m_size)
  {
    std::ostringstream str;
    str << "Writing value 0x"
        << std::hex << value.to_uint64()
        << " that is greater than field '"
        << get_full_name()
        << "' size ("
        << get_n_bits()
        << " bits)";
    UVM_WARNING("RegModel", str.str() );
    value &= value & uvm_mask_size(m_size);
  }
  tmp = 0;

  segsiz = mem->get_n_bytes() * 8;
  flsb   = get_lsb_pos_in_register();
  segoff = m_parent->get_offset_in_memory(idx) + (flsb / segsiz);

  // Any bits on the LSB side we need to RMW?
  rmwbits = flsb % segsiz;

  // Total number of memory segment in this field
  segn = (rmwbits + get_n_bits() - 1) / segsiz + 1;

  if (rmwbits > 0)
  {
    uvm_reg_addr_t  segn;

    mem->peek(st, segoff, tmp, "", parent, extension, fname, lineno);
    if (st != UVM_IS_OK && st != UVM_HAS_X)
    {
      std::ostringstream str;
      str << "Unable to read LSB bits in "
          << mem->get_full_name()
          << "["
          << segoff
          << "] to for RMW cycle on virtual field "
          << get_full_name()
          << ".";
      UVM_ERROR("RegModel", str.str() );
      status = UVM_NOT_OK;
      m_parent->m_atomic_check(false);
      return;
    }

    value = (value << rmwbits) | (tmp & uvm_mask_size(rmwbits));
  }

  // Any bits on the MSB side we need to RMW?
  fmsb = rmwbits + get_n_bits() - 1;
  rmwbits = (fmsb+1) % segsiz;
  if (rmwbits > 0)
  {
    if (segn > 0)
    {
      mem->peek(st, segoff + segn - 1, tmp, "", parent, extension, fname, lineno);
      if (st != UVM_IS_OK && st != UVM_HAS_X)
      {
        std::ostringstream str;
        str << "Unable to read MSB bits in "
            << mem->get_full_name()
            << "["
            << segoff+segn-1
            << "] to for RMW cycle on virtual field "
            << get_full_name()
            << ".";
        UVM_ERROR("RegModel", str.str() );
        status = UVM_NOT_OK;
        m_parent->m_atomic_check(false);
        return;
      }
    }
    value |= (tmp & ~uvm_mask_size(rmwbits)) << ((segn-1)*segsiz);
  }

  // Now write each of the segments
  tmp = value;
  for (int i = 0; i < segn; i++ )
  {
    mem->poke(st, segoff, tmp, "", parent, extension, fname, lineno);

    if (st != UVM_IS_OK && st != UVM_HAS_X)
      status = UVM_NOT_OK;

    segoff++;
    tmp = tmp >> segsiz;
  }

  m_parent->m_atomic_check(false);

  std::ostringstream inf;
  inf << "Wrote virtual field '"
      << get_full_name()
      << "'["
      << idx
      << "] with: 0x"
      << std::hex << value.to_uint64();
  UVM_INFO("RegModel", inf.str(), UVM_MEDIUM);

  m_fname = "";
  m_lineno = 0;
}

//----------------------------------------------------------------------
// Member function: peek
//
//! Sample the current value from a virtual field
//!
//! Sample from the DUT memory location(s) that implements
//! the virtual field that corresponds to this
//! abstraction class instance using the specified access
//! \p path, and return the readback \p value.
//!
//! If the memory implementing the virtual register array
//! containing this virtual field
//! is mapped in more than one address map,
//! an address \p map must be
//! specified if a physical access is used (front-door access).
//!
//! The operation is eventually mapped into
//! memory peek operations at the location(s)
//! where the virtual register
//! specified by \p idx in the virtual register array is implemented.
//----------------------------------------------------------------------

void uvm_vreg_field::peek( unsigned long idx,
                           uvm_status_e& status, // output
                           uvm_reg_data_t& value, // output
                           uvm_sequence_base* parent,
                           uvm_object* extension,
                           const std::string& fname,
                           int lineno )
{
   uvm_reg_data_t tmp;
   uvm_reg_data_t segval;
   uvm_reg_addr_t segoff;
   uvm_status_e st;

   int flsb, lsb;
   int segsiz, segn;
   uvm_mem* mem = nullptr;
   m_fname = fname;
   m_lineno = lineno;

   mem = m_parent->get_memory();

   if (mem == nullptr)
   {
      UVM_ERROR("RegModel",
          "Cannot call uvm_vreg_field::peek() on unimplemented virtual register '" +
          get_full_name() + "'." );
      status = UVM_NOT_OK;
      return;
   }

   status = UVM_IS_OK;

   m_parent->m_atomic_check(true);

   value = 0; // TODO check

   segsiz = mem->get_n_bytes() * 8;
   flsb    = get_lsb_pos_in_register();
   segoff  = m_parent->get_offset_in_memory(idx) + (flsb / segsiz);
   lsb = flsb % segsiz;

   // Total number of memory segment in this field
   segn = (lsb + get_n_bits() - 1) / segsiz + 1;

   // Read each of the segments, MSB first
   segoff += segn - 1;
   for( int i = 0; i < segn; i++)
   {
      value = value << segsiz;

      mem->peek(st, segoff, tmp, "", parent, extension, fname, lineno);

      if (st != UVM_IS_OK && st != UVM_HAS_X)
        status = UVM_NOT_OK;

      segoff--;
      value |= tmp;
   }

   // Any bits on the LSB side we need to get rid of?
   value = value >> lsb;

   // Any bits on the MSB side we need to get rid of?
   value &= uvm_mask_size(get_n_bits());

   m_parent->m_atomic_check(false);

   std::ostringstream inf;
   inf << "Peeked virtual field '"
       << get_full_name()
       << "' ["
       << idx
       << "]: 0x"
       << std::hex << value.to_uint64();
   UVM_INFO("RegModel", inf.str(), UVM_MEDIUM);

   m_fname = "";
   m_lineno = 0;
}

//----------------------------------------------------------------------
// Group: Callbacks
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Member function: pre_write
//
//! Called before virtual field write.
//!
//! If the specified data value, access \p path or address \p map are modified,
//! the updated data value, access path or address map will be used
//! to perform the virtual register operation.
//!
//! The virtual field callback methods are invoked before the callback methods
//! on the containing virtual register.
//! The registered callback methods are invoked after the invocation
//! of this method.
//! The pre-write virtual register and field callbacks are executed
//! before the corresponding pre-write memory callbacks
//----------------------------------------------------------------------

void uvm_vreg_field::pre_write( unsigned long idx,
                                uvm_reg_data_t& wdat,
                                uvm_path_e& path,
                                uvm_reg_map*& map )
{}

//----------------------------------------------------------------------
// Member function: post_write
//
//! Called after virtual field write
//!
//! If the specified \p status is modified,
//! the updated status will be
//! returned by the virtual register operation.
//!
//! The virtual field callback methods are invoked after the callback methods
//! on the containing virtual register.
//! The registered callback methods are invoked before the invocation
//! of this method.
//! The post-write virtual register and field callbacks are executed
//! after the corresponding post-write memory callbacks
//----------------------------------------------------------------------

void uvm_vreg_field::post_write( unsigned long idx,
                                 uvm_reg_data_t wdat,
                                 uvm_path_e path,
                                 uvm_reg_map* map,
                                 uvm_status_e& status )
{}

//----------------------------------------------------------------------
// Member function: pre_read
//
//! Called before virtual field read.
//!
//! If the specified access \p path or address \p map are modified,
//! the updated access path or address map will be used to perform
//! the virtual register operation.
//!
//! The virtual field callback methods are invoked after the callback methods
//! on the containing virtual register.
//! The registered callback methods are invoked after the invocation
//! of this method.
//! The pre-read virtual register and field callbacks are executed
//! before the corresponding pre-read memory callbacks
//----------------------------------------------------------------------

void uvm_vreg_field::pre_read( unsigned long idx,
                               uvm_path_e& path,
                               uvm_reg_map*& map)
{}

//----------------------------------------------------------------------
// Member function: post_read
//
//! Called after virtual field read.
//!
//! If the specified readback data \p rdat or \p status is modified,
//! the updated readback data or status will be
//! returned by the virtual register operation.
//!
//! The virtual field callback methods are invoked after the callback methods
//! on the containing virtual register.
//! The registered callback methods are invoked before the invocation
//! of this method.
//! The post-read virtual register and field callbacks are executed
//! after the corresponding post-read memory callbacks
//----------------------------------------------------------------------

void uvm_vreg_field::post_read( unsigned long idx,
                                uvm_reg_data_t& rdat,
                                uvm_path_e path,
                                uvm_reg_map* map,
                                uvm_status_e& status)
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

uvm_vreg* uvm_vreg_field::get_register() const
{
  return m_parent;
}

//----------------------------------------------------------------------
// member function: do_print
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_vreg_field::do_print( const uvm_printer& printer ) const
{
  uvm_object::do_print(printer);
  printer.print_generic("initiator", m_parent->get_type_name(), -1, convert2string());
}

//----------------------------------------------------------------------
// member function: convert2string
//
// Implementation defined
//----------------------------------------------------------------------

std::string uvm_vreg_field::convert2string() const
{
  std::ostringstream res_str;
  std::ostringstream str;

  str << get_name()
      << "["
      << get_lsb_pos_in_register() + get_n_bits() - 1
      << "-"
      << get_lsb_pos_in_register()
      << "]" ;

  if (m_read_in_progress)
  {
    if (!m_fname.empty() && m_lineno != 0)
      res_str << m_fname << ":" << m_lineno;

    str << std::endl << res_str.str() << " currently executing read method";
  }

  if ( m_write_in_progress)
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

uvm_object* uvm_vreg_field::clone()
{
  UVM_FATAL("RegModel","RegModel virtual fields cannot be cloned");
  return nullptr;
}

//----------------------------------------------------------------------
// member function: do_copy
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_vreg_field::do_copy( const uvm_object& rhs )
{
  UVM_FATAL("RegModel","RegModel virtual fields cannot be copied");
  // TODO disable copy constructor
}


//----------------------------------------------------------------------
// member function: do_compare
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_vreg_field::do_compare( const uvm_object& rhs,
                                 const uvm_comparer* comparer ) const
{
  UVM_WARNING("RegModel","RegModel virtual fields cannot be compared");
  return false;
}

//----------------------------------------------------------------------
// member function: do_pack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_vreg_field::do_pack( uvm_packer& packer ) const
{
  UVM_WARNING("RegModel","RegModel virtual fields cannot be packed");
}

//----------------------------------------------------------------------
// member function: do_unpack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_vreg_field::do_unpack( uvm_packer& packer )
{
  UVM_WARNING("RegModel","RegModel virtual fields cannot be unpacked");
}

} // namespace uvm
