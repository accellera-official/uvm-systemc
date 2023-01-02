//----------------------------------------------------------------------
//   Copyright 2013-2020 NXP B.V.
//   Copyright 2004-2011 Synopsys, Inc.
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

#include <systemc>
#include <vector>

#include "uvmsc/reg/uvm_reg_map.h"
#include "uvmsc/reg/uvm_reg_block.h"
#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/reg/uvm_reg_item.h"
#include "uvmsc/reg/uvm_reg_field.h"
#include "uvmsc/reg/uvm_reg_adapter.h"
#include "uvmsc/reg/uvm_mem.h"
#include "uvmsc/reg/uvm_vreg.h"
#include "uvmsc/reg/uvm_reg_read_only_cbs.h"
#include "uvmsc/reg/uvm_reg_write_only_cbs.h"
#include "uvmsc/seq/uvm_sequencer_base.h"
#include "uvmsc/seq/uvm_sequence_base.h"
#include "uvmsc/seq/uvm_sequence.h"

namespace uvm {

class reg_rw : public uvm_sequence_item
{
 public:

  bool read; // TODO randomize these 4
  sc_dt::sc_bv<32> addr;
  sc_dt::sc_lv<32> data;
  sc_dt::sc_lv<4>  byte_en;

  reg_rw (const std::string& name = "reg_rw") : uvm_sequence_item(name)
  {}

  UVM_OBJECT_UTILS(reg_rw);

  /* TODO implement other methods
   `uvm_object_utils_begin(reg_rw)
     `uvm_field_int(read, UVM_ALL_ON | UVM_NOPACK);
     `uvm_field_int(addr, UVM_ALL_ON | UVM_NOPACK);
     `uvm_field_int(data, UVM_ALL_ON | UVM_NOPACK);
     `uvm_field_int(byte_en, UVM_ALL_ON | UVM_NOPACK);
   `uvm_object_utils_end
   */

  std::string convert2string() const
  {
    std::ostringstream str;

    str << "reg_rw: "
        << ((read) ? "READ" : "WRITE" )
        << " addr = 0x" << addr.to_string(sc_dt::SC_HEX_US, false)
        << " data = 0x" << data.to_string(sc_dt::SC_HEX_US, false)
        << " byte_en = " << byte_en;
    return str.str();
  }

}; // class reg_rw


//------------------------------------------------------------------------------
// static data member initialization
//------------------------------------------------------------------------------

uvm_reg_map* uvm_reg_map::m_backdoor = NULL;

//----------------------------------------------------------------------
// Group: Initialization
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Constructor
//
//! Create a new instance
//----------------------------------------------------------------------

uvm_reg_map::uvm_reg_map( const std::string& name )
  : uvm_object( (name.empty()) ? "default_map" : name )
{
  m_parent = NULL;
  m_adapter = NULL;
  m_sequencer = NULL;
  m_parent_map = NULL;
  m_parent_maps.clear();
  m_submaps.clear();
  m_submap_rights.clear();
  m_regs_info.clear();
  m_mems_info.clear();
  m_regs_by_offset.clear();
  m_regs_by_offset_wo.clear();
  m_mems_by_offset.clear();

  m_auto_predict = false;
  m_check_on_read = false;

}

//----------------------------------------------------------------------
// member function: configure
//
//! Instance-specific configuration
//!
//! Configures this map with the following properties.
//!
//! parent    - the block in which this map is created and applied
//!
//! base_addr - the base address for this map. All registers, memories,
//!             and sub-blocks will be at offsets to this address
//!
//! n_bytes   - the byte-width of the bus on which this map is used
//!
//! endian    - the endian format. See #uvm_endianness_e for possible
//!             values
//!
//! byte_addressing - specifies whether the address increment is on a
//!             per-byte basis. For example, consecutive memory locations
//!             with \p n_bytes = 4 (32-bit bus) are 4 apart: 0, 4, 8, and
//!             so on. Default is TRUE.
//----------------------------------------------------------------------

void uvm_reg_map::configure( uvm_reg_block* parent,
                             uvm_reg_addr_t base_addr,
                             unsigned int n_bytes,
                             uvm_endianness_e endian,
                             bool byte_addressing )
{
   m_parent    = parent;
   m_n_bytes   = n_bytes;
   m_endian    = endian;
   m_base_addr = base_addr;
   m_byte_addressing = byte_addressing;
}

//----------------------------------------------------------------------
// member function: add_reg
//
//! Add a register
//!
//! Add the specified register instance \p rg to this address map.
//!
//! The register is located at the specified address \p offset from
//! this maps configured base address.
//!
//! The \p rights specify the register's accessibility via this map.
//! Valid values are "RW", "RO", and "WO". Whether a register field
//! can be read or written depends on both the field's configured access
//! policy (see uvm_reg_field::configure and the register's rights in
//! the map being used to access the field.
//!
//! The number of consecutive physical addresses occupied by the register
//! depends on the width of the register and the number of bytes in the
//! physical interface corresponding to this address map.
//!
//! If \p unmapped is true, the register does not occupy any
//! physical addresses and the base address is ignored.
//! Unmapped registers require a user-defined \p frontdoor to be specified.
//!
//! A register may be added to multiple address maps
//! if it is accessible from multiple physical interfaces.
//! A register may only be added to an address map whose parent block
//! is the same as the register's parent block.
//----------------------------------------------------------------------

void uvm_reg_map::add_reg( uvm_reg* rg,
                           uvm_reg_addr_t offset,
                           const std::string& rights,
                           bool unmapped,
                           uvm_reg_frontdoor* frontdoor )
{
  if (m_regs_info.find(rg) != m_regs_info.end()) // exists
  {
    UVM_ERROR("RegModel", "Register '" + rg->get_name() +
        "' has already been added to map '" + get_name() + "'");
    return;
  }

  if (rg->get_parent() != get_parent())
  {
    UVM_ERROR("RegModel",
        "Register '" + rg->get_full_name() +
        "' may not be added to address map '" +
        get_full_name() + "' : they are not in the same block");
    return;
  }

  rg->add_map(this);

  uvm_reg_map_info* info = new uvm_reg_map_info();
  info->offset   = offset;
  info->rights   = rights;
  info->unmapped = unmapped;
  info->frontdoor = frontdoor;
  m_regs_info[rg] = info;
}


//----------------------------------------------------------------------
// member function: add_mem
//
//! Add a memory
//!
//! Add the specified memory instance to this address map.
//! The memory is located at the specified base address and has the
//! specified access rights ("RW", "RO" or "WO").
//! The number of consecutive physical addresses occupied by the memory
//! depends on the width and size of the memory and the number of bytes in the
//! physical interface corresponding to this address map.
//!
//! If \p unmapped is true, the memory does not occupy any
//! physical addresses and the base address is ignored.
//! Unmapped memorys require a user-defined \p frontdoor to be specified.
//!
//! A memory may be added to multiple address maps
//! if it is accessible from multiple physical interfaces.
//! A memory may only be added to an address map whose parent block
//! is the same as the memory's parent block.
//----------------------------------------------------------------------

void uvm_reg_map::add_mem( uvm_mem* mem,
                           uvm_reg_addr_t offset,
                           const std::string& rights,
                           bool unmapped,
                           uvm_reg_frontdoor* frontdoor )
{
  if (m_mems_info.find(mem) != m_mems_info.end() ) // exists
  {
    UVM_ERROR("RegModel", "Memory '" + mem->get_name() +
        "' has already been added to map '" + get_name() + "'");
    return;
  }

  if (mem->get_parent() != get_parent())
  {
    UVM_ERROR("RegModel",
        "Memory '" + mem->get_full_name() + "' may not be added to address map '" +
        get_full_name() + "' : they are not in the same block");
    return;
  }

  mem->add_map(this);

  uvm_reg_map_info* info = new uvm_reg_map_info();
  info->offset   = offset;
  info->rights   = rights;
  info->unmapped = unmapped;
  info->frontdoor = frontdoor;
  m_mems_info[mem] = info;
}

//----------------------------------------------------------------------
// member function: add_submap
//
//! Add an address map
//!
//! Add the specified address map instance to this address map.
//! The address map is located at the specified base address.
//! The number of consecutive physical addresses occupied by the submap
//! depends on the number of bytes in the physical interface
//! that corresponds to the submap,
//! the number of addresses used in the submap and
//! the number of bytes in the
//! physical interface corresponding to this address map.
//!
//! An address map may be added to multiple address maps
//! if it is accessible from multiple physical interfaces.
//! An address map may only be added to an address map
//! in the grand-parent block of the address submap.
//----------------------------------------------------------------------

void uvm_reg_map::add_submap( uvm_reg_map* child_map,
                              uvm_reg_addr_t offset )
{
  uvm_reg_map* parent_map;

  if(child_map == NULL)
  {
    UVM_ERROR("RegModel", "Attempting to add NULL map to map '" + get_full_name() + "'");
    return;
  }

  parent_map = child_map->get_parent_map();

  // Can not have more than one parent (currently)
  if(parent_map != NULL)
  {
    UVM_ERROR("RegModel", "Map '" +  child_map->get_full_name() +
        "' is already a child of map '" +
        parent_map->get_full_name() +
        "'. Cannot also be a child of map '" +
        get_full_name() + "'");
    return;
  }

  // parent_block_check
  uvm_reg_block* child_blk = child_map->get_parent();
  if (child_blk == NULL)
  {
    UVM_ERROR("RegModel", "Cannot add submap '" + child_map->get_full_name() +
        "' because it does not have a parent block");
    return;
  }
  if (get_parent() != child_blk->get_parent())
  {
    UVM_ERROR("RegModel",
        "Submap '" + child_map->get_full_name() + "' may not be added to this " +
        "address map, '" + get_full_name() + "', as the submap's parent block, '" +
        child_blk->get_full_name() + "', is not a child of this map's parent block, '" +
        m_parent->get_full_name() + "'");
    return;
  }


  // n_bytes_match_check
  if (m_n_bytes > child_map->get_n_bytes(UVM_NO_HIER))
  {
    std::ostringstream str;
    str << "Adding " << child_map->get_n_bytes(UVM_NO_HIER)
                << "-byte submap '" << child_map->get_full_name()
                << "' to " << m_n_bytes
                << "-byte parent map '" << get_full_name() << "'";
    UVM_WARNING("RegModel", str.str() );
  }

  child_map->add_parent_map(this, offset);

  set_submap_offset(child_map, offset);
}

//----------------------------------------------------------------------
// member function: set_sequencer
//
//! Set the sequencer and adapter associated with this map. This method
//! must be called before starting any sequences based on #uvm_reg_sequence.
//----------------------------------------------------------------------

void uvm_reg_map::set_sequencer( uvm_sequencer_base* sequencer,
                                 uvm_reg_adapter* adapter )
{
  if( sequencer == NULL )
  {
    UVM_ERROR("REG_NULL_SQR", "NULL reference specified for bus sequencer");
    return;
  }

  if( adapter == NULL )
  {
    UVM_INFO("REG_NO_ADAPT", "Adapter not specified for map '" + get_full_name() +
        "'. Accesses via this map will send abstract 'uvm_reg_item' items to sequencer '" +
        sequencer->get_full_name() + "'", UVM_MEDIUM);
  }

  m_sequencer = sequencer;
  m_adapter = adapter;
}


//----------------------------------------------------------------------
// member function: set_submap_offset
//
//! Set the offset of the given \p submap to \p offset.
//----------------------------------------------------------------------

void uvm_reg_map::set_submap_offset( uvm_reg_map* submap, uvm_reg_addr_t offset)
{
  if( submap == NULL )
  {
    UVM_ERROR("REG/NULL","set_submap_offset: submap handle is NULL");
    return;
  }

  m_submaps[submap] = offset;

  if (m_parent->is_locked())
  {
    uvm_reg_map* root_map = get_root_map();
    root_map->m_init_address_map();
  }
}


//----------------------------------------------------------------------
// member function: get_submap_offset
//
//! Return the offset of the given \p submap.
//----------------------------------------------------------------------

uvm_reg_addr_t uvm_reg_map::get_submap_offset( const uvm_reg_map* submap ) const
{
  uvm_reg_map* map = const_cast<uvm_reg_map*>(submap);

  if(map == NULL)
  {
    UVM_ERROR("REG/NULL","set_submap_offset: submap handle is NULL");
    return -1; // TODO is this compatible with the return type?
  }

  if(m_submaps.find(map) == m_submaps.end()) // not exists
  {
    UVM_ERROR("RegModel","Map '" + map->get_full_name() +
        "' is not a submap of '" + get_full_name() + "'");
    return -1; // TODO is this compatible with the return type?
  }

  return m_submaps.find(map)->second;
}

//----------------------------------------------------------------------
// member function: set_base_addr
//
//! Set the base address of this map.
//----------------------------------------------------------------------

void uvm_reg_map::set_base_addr( uvm_reg_addr_t offset )
{
  if(m_parent_map != NULL)
  {
    m_parent_map->set_submap_offset(this, offset);
  }
  else
  {
    m_base_addr = offset;
    if(m_parent->is_locked())
    {
      uvm_reg_map* top_map = get_root_map();
      top_map->m_init_address_map();
    }
  }
}

//----------------------------------------------------------------------
// member function: reset
//
//! Reset the mirror for all registers in this address map.
//!
//! Sets the mirror value of all registers in this address map
//! and all of its submaps
//! to the reset value corresponding to the specified reset event.
//! See uvm_reg_field::reset() for more details.
//! Does not actually set the value of the registers in the design,
//! only the values mirrored in their corresponding mirror.
//!
//! Note that, unlike the other reset() method, the default
//! reset event for this method is "SOFT".
//----------------------------------------------------------------------

void uvm_reg_map::reset( const std::string& kind )
{
   std::vector<uvm_reg*> regs;

   get_registers(regs);

   for( unsigned int i = 0; i < regs.size(); i++)
      regs[i]->reset(kind);
}

//----------------------------------------------------------------------
// Group: Introspection
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: get_name
//
//! Get the simple name
//!
//! Return the simple object name of this address map.
//----------------------------------------------------------------------

// inherited from base class

//----------------------------------------------------------------------
// member function: get_full_name
//
//! Get the hierarchical name
//!
//! Return the hierarchical name of this address map.
//! The base of the hierarchical name is the root block.
//----------------------------------------------------------------------

const std::string uvm_reg_map::get_full_name() const
{
  std::string name = get_name();

  if(m_parent == NULL)
    return name;

  return m_parent->get_full_name() + "." + name;
}

//----------------------------------------------------------------------
// member function: get_root_map
//
//! Get the externally-visible address map
//!
//! Get the top-most address map where this address map is instantiated.
//! It corresponds to the externally-visible address map that can
//! be accessed by the verification environment.
//----------------------------------------------------------------------

uvm_reg_map* uvm_reg_map::get_root_map() const
{
  const uvm_reg_map* ret;
  ret = (m_parent_map == NULL) ? this : m_parent_map->get_root_map();
  return const_cast<uvm_reg_map*>(ret); // TODO avoid this?
}

//----------------------------------------------------------------------
// member function: get_parent
//
//! Get the parent block
//!
//! Return the block that is the parent of this address map.
//----------------------------------------------------------------------

uvm_reg_block* uvm_reg_map::get_parent() const
{
  return m_parent;
}

//----------------------------------------------------------------------
// member function: get_parent_map
//
//! Get the higher-level address map
//!
//! Return the address map in which this address map is mapped.
//! returns NULL if this is a top-level address map.
//----------------------------------------------------------------------

uvm_reg_map* uvm_reg_map::get_parent_map() const
{
  return m_parent_map;
}
//----------------------------------------------------------------------
// member function: get_base_addr
//
//! Get the base offset address for this map. If this map is the
//! root map, the base address is that set with the \p base_addr argument
//! to uvm_reg_block::create_map(). If this map is a submap of a higher-level map,
//! the base address is offset given this submap by the parent map.
//! See #set_submap_offset.
//----------------------------------------------------------------------

uvm_reg_addr_t uvm_reg_map::get_base_addr( uvm_hier_e hier ) const
{
  uvm_reg_addr_t addr;

  if (hier == UVM_NO_HIER || m_parent_map == NULL)
    return m_base_addr;

  addr = m_parent_map->get_submap_offset(this);
  addr += m_parent_map->get_base_addr(UVM_HIER);

  return addr;
}

//----------------------------------------------------------------------
// member function: get_n_bytes
//
//! Get the width in bytes of the bus associated with this map. If \p hier
//! is UVM_HIER, then gets the effective bus width relative to the system
//! level. The effective bus width is the narrowest bus width from this
//! map to the top-level root map. Each bus access will be limited to this
//! bus width.
//----------------------------------------------------------------------

unsigned int uvm_reg_map::get_n_bytes( uvm_hier_e hier ) const
{
  if (hier == UVM_NO_HIER)
    return m_n_bytes;

  return m_system_n_bytes;
}
//----------------------------------------------------------------------
// member function: get_addr_unit_bytes
//
//! Get the number of bytes in the smallest addressable unit in the map.
//! Returns 1 if the address map was configured using byte-level addressing.
//! Returns get_n_bytes() otherwise.
//----------------------------------------------------------------------

unsigned int uvm_reg_map::get_addr_unit_bytes() const
{
  return (m_byte_addressing) ? 1 : m_n_bytes;
}

//----------------------------------------------------------------------
// member function: get_base_addr
//
//! Gets the endianness of the bus associated with this map. If \p hier is
//! set to UVM_HIER, gets the system-level endianness.
//----------------------------------------------------------------------

uvm_endianness_e uvm_reg_map::get_endian( uvm_hier_e hier ) const
{
  if (hier == UVM_NO_HIER || m_parent_map == NULL)
    return m_endian;

  return m_parent_map->get_endian(hier);
}

//----------------------------------------------------------------------
// member function: get_sequencer
//
//! Gets the sequencer for the bus associated with this map. If \p hier is
//! set to UVM_HIER, gets the sequencer for the bus at the system-level.
//! See #set_sequencer.
//----------------------------------------------------------------------

uvm_sequencer_base* uvm_reg_map::get_sequencer( uvm_hier_e hier ) const
{
  if (hier == UVM_NO_HIER || m_parent_map == NULL)
    return m_sequencer;

  return m_parent_map->get_sequencer(hier);
}

//----------------------------------------------------------------------
// member function: get_adapter
//
//! Gets the bus adapter for the bus associated with this map. If \p hier is
//! set to UVM_HIER, gets the adapter for the bus used at the system-level.
//! See #set_sequencer.
//----------------------------------------------------------------------

uvm_reg_adapter* uvm_reg_map::get_adapter( uvm_hier_e hier ) const
{
  if (hier == UVM_NO_HIER || m_parent_map == NULL)
    return m_adapter;

  return m_parent_map->get_adapter(hier);
}

//----------------------------------------------------------------------
// member function: get_submaps
//
//! Get the address sub-maps
//!
//! Get the address maps instantiated in this address map.
//! If \p hier is UVM_HIER, recursively includes the address maps,
//! in the sub-maps.
//----------------------------------------------------------------------

void uvm_reg_map::get_submaps( std::vector<uvm_reg_map*>& maps, uvm_hier_e hier )  const
{
  for( m_submaps_citt it = m_submaps.begin(); it != m_submaps.end(); it++ )
    maps.push_back( (*it).first );

  if (hier == UVM_HIER)
    for( m_submaps_citt it = m_submaps.begin(); it != m_submaps.end(); it++ )
    {
      uvm_reg_map* submap = (*it).first;
      submap->get_submaps(maps);
    }
}

//----------------------------------------------------------------------
// member function: get_registers
//
//! Get the registers
//!
//! Get the registers instantiated in this address map.
//! If \p hier is UVM_HIER, recursively includes the registers
//! in the sub-maps.
//----------------------------------------------------------------------

void uvm_reg_map::get_registers( std::vector<uvm_reg*>& regs, uvm_hier_e hier ) const
{
  for( m_regs_info_citt it = m_regs_info.begin(); it != m_regs_info.end(); it++ )
    regs.push_back( (*it).first );

  if (hier == UVM_HIER)
    for( m_submaps_citt it = m_submaps.begin(); it != m_submaps.end(); it++ )
    {
      uvm_reg_map* submap = (*it).first;
      submap->get_registers(regs);
    }
}

//----------------------------------------------------------------------
// member function: get_fields
//
//! Get the fields
//!
//! Get the fields in the registers instantiated in this address map.
//! If \p hier is UVM_HIER, recursively includes the fields of the registers
//! in the sub-maps.
//----------------------------------------------------------------------

void uvm_reg_map::get_fields( std::vector<uvm_reg_field*>& fields, uvm_hier_e hier ) const
{
  for( m_regs_info_citt it = m_regs_info.begin(); it != m_regs_info.end(); it++ )
  {
    uvm_reg* rg = (*it).first;
    rg->get_fields(fields);
  }

  if (hier == UVM_HIER)
    for( m_submaps_citt it = m_submaps.begin(); it != m_submaps.end(); it++ )
    {
      uvm_reg_map* submap = (*it).first;
      submap->get_fields(fields);
    }
}


//----------------------------------------------------------------------
// member function: get_memories
//
//! Get the memories
//!
//! Get the memories instantiated in this address map.
//! If \p hier is UVM_HIER, recursively includes the memories
//! in the sub-maps.
//----------------------------------------------------------------------

void uvm_reg_map::get_memories( std::vector<uvm_mem*>& mems, uvm_hier_e hier ) const
{
  for( m_mems_info_citt it = m_mems_info.begin(); it != m_mems_info.end(); it++ )
    mems.push_back( (*it).first);

  if (hier == UVM_HIER)
    for( m_submaps_citt it = m_submaps.begin(); it != m_submaps.end(); it++ )
    {
      uvm_reg_map* submap = (*it).first;
      submap->get_memories(mems);
    }
}

//----------------------------------------------------------------------
// member function: get_virtual_registers
//
//! Get the virtual registers
//!
//! Get the virtual registers instantiated in this address map.
//! If \p hier is UVM_HIER, recursively includes the virtual registers
//! in the sub-maps.
//----------------------------------------------------------------------

void uvm_reg_map::get_virtual_registers( std::vector<uvm_vreg*>& regs,
                                         uvm_hier_e hier ) const
{
  std::vector<uvm_mem*> mems;
  get_memories(mems, hier);

  for( unsigned int i = 0; i < mems.size(); i++ )
    mems[i]->get_virtual_registers(regs);
}

//----------------------------------------------------------------------
// member function: get_virtual_fields
//
//! Get the virtual fields
//!
//! Get the virtual fields from the virtual registers instantiated
//! in this address map.
//! If \p hier is UVM_HIER, recursively includes the virtual fields
//! in the virtual registers in the sub-maps.
//----------------------------------------------------------------------

void uvm_reg_map::get_virtual_fields( std::vector<uvm_vreg_field*>& fields,
                                      uvm_hier_e hier ) const
{
  std::vector<uvm_vreg*> regs;

  get_virtual_registers(regs, hier);

  for( unsigned int i = 0; i < regs.size(); i++ )
  regs[i]->get_fields(fields);
}

//----------------------------------------------------------------------
// member function: get_physical_addresses
//
//! Translate a local address into external addresses
//!
//! Identify the sequence of addresses that must be accessed physically
//! to access the specified number of bytes at the specified address
//! within this address map.
//! Returns the number of bytes of valid data in each access.
//!
//! Returns in \p addr a list of address in little endian order,
//! with the granularity of the top-level address map.
//!
//! A register is specified using a base address with \p mem_offset as 0.
//! A location within a memory is specified using the base address
//! of the memory and the index of the location within that memory.
//----------------------------------------------------------------------

int uvm_reg_map::get_physical_addresses( uvm_reg_addr_t base_addr,
                                         uvm_reg_addr_t mem_offset,
                                         unsigned int n_bytes,
                                         std::vector<uvm_reg_addr_t>& addr ) const
{
  unsigned int bus_width = get_n_bytes(UVM_NO_HIER);
  uvm_reg_map* up_map = NULL;
  std::vector<uvm_reg_addr_t> local_addr;
  int multiplier = m_byte_addressing ? bus_width : 1;

  // TODO usage of the address map
  //addr = new [0];

  std::vector<uvm_reg_addr_t> addr2;

  if (n_bytes <= 0)
  {
    std::ostringstream str;
    str << "Cannot access " << n_bytes << " bytes. Must be greater than 0";
    UVM_FATAL("RegModel", str.str());
    return 0;
  }

  // First, identify the addresses within the block/system
  if (n_bytes <= bus_width)
  {
    local_addr.resize(1);
    local_addr[0] = base_addr + (mem_offset * multiplier);
  }
  else
  {
    int n;
    n = ((n_bytes-1) / bus_width) + 1;
    local_addr.resize(n);
    base_addr = base_addr + mem_offset * (n * multiplier);

    switch (get_endian(UVM_NO_HIER))
    {
    case UVM_LITTLE_ENDIAN:
    {
      for( int i = 0; local_addr.size(); i++ )
        local_addr[i] = base_addr + (i * multiplier);
      break;
    }
    case UVM_BIG_ENDIAN:
    {
      for( int i = 0; local_addr.size(); i++ )
      {
        n--;
        local_addr[i] = base_addr + (n * multiplier);
      }
      break;
    }
    case UVM_LITTLE_FIFO:
    {
      for( int i = 0; local_addr.size(); i++ )
        local_addr[i] = base_addr;
      break;
    }
    case UVM_BIG_FIFO:
    {
      for( int i = 0; local_addr.size(); i++ )
        local_addr[i] = base_addr;
      break;
    }
    default:
    {
      std::ostringstream str;
      str << "Map has no specified endianness. "
          << "Cannot access " << n_bytes << " bytes register via its "
          << bus_width << " byte '" << get_full_name() << "' interface";
      UVM_ERROR("RegModel", str.str() );
      break;
    }
    } // switch
  }

  up_map = get_parent_map();

  // Then translate these addresses in the parent's space
  if (up_map == NULL)
  {
    // This is the top-most system/block!
    for( unsigned int i = 0; i < local_addr.size(); i++ )
    {
      // TODO check eq: addr = new [local_addr.size()] (local_addr);
      addr.resize(local_addr.size());
      addr = local_addr;
    }

    for( unsigned int i = 0; i < addr.size(); i++ )
      addr[i] += m_base_addr;
  }
  else
  {
    std::vector<uvm_reg_addr_t> sys_addr;
    unsigned int w = bus_width;

    // Scale the consecutive local address in the system's granularity
    int k = 1;
    if (bus_width >= up_map->get_n_bytes(UVM_NO_HIER))
      k = ((bus_width-1) / up_map->get_n_bytes(UVM_NO_HIER)) + 1;

    uvm_reg_addr_t base_addr = up_map->get_submap_offset(this);

    for( unsigned int i = 0; i < local_addr.size(); i++ )
    {
      int n = (int)addr.size();

      w = up_map->get_physical_addresses(base_addr + local_addr[i] * k,
          0,
          bus_width,
          sys_addr);

      //TODO check eq addr = new [n + sys_addr.size()] (addr);
      std::vector<uvm_reg_addr_t> tmp(addr);
      addr.resize(n + sys_addr.size()); // resize, values lost, needs to be assigned again
      addr = tmp;

      for( unsigned int j = 0; j < sys_addr.size(); j++ )
        addr[n+j] = sys_addr[j];
    }

    // The width of each access is the minimum of this block or the system's width
    if (w < bus_width)
      bus_width = w;
  }

  return bus_width;
}

//----------------------------------------------------------------------
// member function: get_reg_by_offset
//
//! Get register mapped at offset
//!
//! Identify the register located at the specified offset within
//! this address map for the specified type of access.
//! Returns NULL if no such register is found.
//!
//! The model must be locked using uvm_reg_block::lock_model()
//! to enable this functionality.
//----------------------------------------------------------------------

uvm_reg* uvm_reg_map::get_reg_by_offset( uvm_reg_addr_t offset,
                                         bool read ) const
{
  if (!m_parent->is_locked())
  {
    UVM_ERROR("RegModel",
      "Cannot get register by offset: Block " + m_parent->get_full_name() + " is not locked.");
    return NULL;
  }

  if (!read && (m_regs_by_offset_wo.find(offset) != m_regs_by_offset_wo.end() )) //exists
    return m_regs_by_offset_wo.find(offset)->second;

  if (m_regs_by_offset.find(offset) != m_regs_by_offset.end()) //exists
    return m_regs_by_offset.find(offset)->second;

  return NULL;
}

//----------------------------------------------------------------------
// member function: get_mem_by_offset
//
//! Get memory mapped at offset
//!
//! Identify the memory located at the specified offset within
//! this address map. The offset may refer to any memory location
//! in that memory.
//! Returns NULL if no such memory is found.
//!
//! The model must be locked using uvm_reg_block::lock_model()
//! to enable this functionality.
//----------------------------------------------------------------------

uvm_mem* uvm_reg_map::get_mem_by_offset( uvm_reg_addr_t offset ) const
{
  if (!m_parent->is_locked())
  {
    UVM_ERROR("RegModel",
      "Cannot memory register by offset: Block " + m_parent->get_full_name() + " is not locked.");
    return NULL;
  }

  for( m_mems_by_offset_citt it = m_mems_by_offset.begin();
       it != m_mems_by_offset.end();
       it++ )
  {
    uvm_reg_map_addr_range* range = (*it).first;

    if( (range->min <= offset) && (offset <= range->max) )
      return m_mems_by_offset.find(range)->second;
  }

  return NULL;
}

//----------------------------------------------------------------------
// Group: Bus Access
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: set_auto_predict
//
//! Sets the auto-predict mode for his map.
//!
//! When \p on is TRUE,
//! the register model will automatically update its mirror
//! (what it thinks should be in the DUT) immediately after
//! any bus read or write operation via this map. Before a uvm_reg::write
//! or uvm_reg::read operation returns, the register's uvm_reg::predict
//! method is called to update the mirrored value in the register.
//!
//! When \p on is false, bus reads and writes via this map do not
//! automatically update the mirror. For real-time updates to the mirror
//! in this mode, you connect a #uvm_reg_predictor instance to the bus
//! monitor. The predictor takes observed bus transactions from the
//! bus monitor, looks up the associated #uvm_reg register given
//! the address, then calls that register's uvm_reg::predict method.
//! While more complex, this mode will capture all register read/write
//! activity, including that not directly descendant from calls to
//! uvm_reg::write and uvm_reg::read.
//!
//! By default, auto-prediction is turned off.
//----------------------------------------------------------------------

void uvm_reg_map::set_auto_predict( bool on )
{
  m_auto_predict = on;
}


//----------------------------------------------------------------------
// member function: get_auto_predict
//
// Gets the auto-predict mode setting for this map.
//----------------------------------------------------------------------

bool uvm_reg_map::get_auto_predict() const
{
  return m_auto_predict;
}


//----------------------------------------------------------------------
// member function: set_check_on_read
//
//! Sets the check-on-read mode for his map
//! and all of its submaps.
//!
//! When \p on is true,
//! the register model will automatically check any value read back from
//! a register or field against the current value in its mirror
//! and report any discrepancy.
//! This effectively combines the functionality of the
//! uvm_reg::read() and uvm_reg::mirror(UVM_CHECK) method.
//! This mode is useful when the register model is used passively.
//!
//! When \p on is false, no check is made against the mirrored value.
//!
//! At the end of the read operation, the mirror value is updated based
//! on the value that was read regardless of this mode setting.
//!
//! By default, auto-prediction is turned off.
//----------------------------------------------------------------------

void uvm_reg_map::set_check_on_read( bool on )
{
  m_check_on_read = on;
  for( m_submaps_itt it = m_submaps.begin(); it != m_submaps.end(); it++ )
    ((*it).first)->set_check_on_read(on);
}

//----------------------------------------------------------------------
// member function: get_check_on_read
//
//! Gets the check-on-read mode setting for this map.
//----------------------------------------------------------------------

bool uvm_reg_map::get_check_on_read() const
{
  return m_check_on_read;
}


//----------------------------------------------------------------------
// member function: do_bus_write
//
//! Perform a bus write operation.
//----------------------------------------------------------------------

void uvm_reg_map::do_bus_write( uvm_reg_item* rw,
                                uvm_sequencer_base* sequencer,
                                uvm_reg_adapter* adapter )
{
  std::vector<uvm_reg_addr_t> addrs;
  //uvm_reg_map*      system_map = get_root_map(); // TODO remove, not used?
  unsigned int      bus_width  = get_n_bytes();
  uvm_reg_byte_en_t byte_en    = -1;
  uvm_reg_map_info* map_info;
  unsigned int      n_bits;
  int               lsb;
  int               skip;
  unsigned int      curr_byte;
  int               n_access_extra;
  unsigned int      n_bits_init;

  m_get_bus_info(rw, map_info, n_bits_init, lsb, skip);

  // TODO check eq: addrs = map_info->addr;
  addrs.clear();
  for (unsigned int i = 0; i < map_info->addr.size(); i++)
    addrs.push_back(map_info->addr[i]);

  // if a memory, adjust addresses based on offset
  if (rw->element_kind == UVM_MEM)
    for( unsigned int i = 0; i < addrs.size(); i++ )
      addrs[i] = addrs[i] + map_info->mem_range.stride * rw->offset;

  for( unsigned int val_idx = 0; val_idx < rw->value.size(); val_idx++ )
  {
    uvm_reg_data_t value = rw->value[val_idx];

    /* calculate byte_enables */
    if (rw->element_kind == UVM_FIELD)
    {
      int temp_be = 0;
      int idx = 0;

      n_access_extra = lsb%(bus_width*8);
      temp_be = n_access_extra;

      value = value << n_access_extra;

      while(temp_be >= 8)
      {
        byte_en[idx++] = 0;
        temp_be -= 8;
      }

      temp_be += n_bits_init;

      while(temp_be > 0)
      {
        byte_en[idx++] = 1;
        temp_be -= 8;
      }

      byte_en &= (1<<idx)-1;

      for (int i = 0; i < skip; i++)
        addrs.erase(addrs.begin()); // pop_front()

      while (addrs.size() > (n_bits_init/(bus_width*8) + 1))
        addrs.pop_back();
    }

    curr_byte = 0;
    n_bits= n_bits_init;

    for( unsigned int i = 0; i < addrs.size(); i++ )
    {
      uvm_sequence_item* bus_req;
      uvm_reg_bus_op rw_access;
      uvm_reg_data_t data;

      // TODO check
      // eq: (value >> (curr_byte*8)) & (( 1 << (bus_width * 8))-1);
      data = (value >> (curr_byte*8)) & uvm_mask_size(bus_width * 8);

      std::ostringstream str;
      str << "Writing 0x"
          <<  data.to_string(sc_dt::SC_HEX_US, false)
          << " at address 0x"
          << std::hex << addrs[i].to_uint64()
          << " via map '"
          << rw->map->get_full_name()
          << "'...";
      UVM_INFO(get_type_name(), str.str(), UVM_FULL);

      if (rw->element_kind == UVM_FIELD)
      {
        for( unsigned int z = 0; z < bus_width; z++ )
          rw_access.byte_en[z] = byte_en[curr_byte+z];
      }

      rw_access.kind = rw->access_kind;
      rw_access.addr = addrs[i];
      rw_access.data = data;
      rw_access.n_bits = (n_bits > bus_width*8) ? bus_width*8 : n_bits;
      rw_access.byte_en = byte_en;

      adapter->m_set_item(rw);
      bus_req = adapter->reg2bus(rw_access);
      adapter->m_set_item(NULL);

      if (bus_req == NULL)
        UVM_FATAL("RegMem","adapter [" + adapter->get_name() + "] did not return a bus transaction");

      bus_req->set_sequencer(sequencer);
      rw->parent->start_item(bus_req, rw->prior);

      if (rw->parent != NULL && i == 0)
        rw->parent->mid_do(rw);

      rw->parent->finish_item(bus_req);

      bus_req->end_event.wait_on();

      if (adapter->provides_responses)
      {
        uvm_sequence_item* bus_rsp;
        // TODO: need to test for right trans type, if not put back in q
        bus_rsp = rw->parent->get_base_response();
        adapter->bus2reg(bus_rsp, rw_access);
        rw->parent->del_base_response(bus_rsp); // remove item from response queue
      }
      else
        adapter->bus2reg(bus_req, rw_access);

       if (rw->parent != NULL && i == addrs.size()-1)
        rw->parent->post_do(rw);

      rw->status = rw_access.status;

      std::ostringstream str1;
      str1 << "Wrote 0x"
           << data.to_string(sc_dt::SC_HEX_US, false)
           << " at address 0x"
           << std::hex << addrs[i].to_uint64()
           << " via map '"
           << rw->map->get_full_name()
           << "': "
           << uvm_status_name[rw->status]
           << "...";
      UVM_INFO(get_type_name(), str1.str(), UVM_FULL);

      if (rw->status == UVM_NOT_OK)
        break;

      curr_byte += bus_width;
      n_bits -= bus_width * 8;

    } // for-addr

    for( unsigned int i = 0; i < addrs.size(); i++ )
      addrs[i] = addrs[i] + map_info->mem_range.stride;

  } // for-value
}


//----------------------------------------------------------------------
// member function: do_bus_read
//
//! Perform a bus read operation.
//----------------------------------------------------------------------

void uvm_reg_map::do_bus_read( uvm_reg_item* rw,
                               uvm_sequencer_base* sequencer,
                               uvm_reg_adapter* adapter)
{
  std::vector<uvm_reg_addr_t> addrs;
  //uvm_reg_map*      system_map = get_root_map(); // TODO remove, is not used?
  unsigned int      bus_width  = get_n_bytes();
  uvm_reg_byte_en_t byte_en    = -1;
  uvm_reg_map_info* map_info;
  int               size;
  unsigned int      n_bits;
  int               skip;
  int               lsb;
  unsigned int      curr_byte;
  int n_access_extra = 0;

  m_get_bus_info(rw, map_info, n_bits, lsb, skip);

  // TODO check eq: addrs = map_info->addr;
  addrs.clear();
  for (unsigned int i = 0; i < map_info->addr.size(); i++)
    addrs.push_back(map_info->addr[i]);

  size = n_bits;

  // if a memory, adjust addresses based on offset
  if (rw->element_kind == UVM_MEM)
    for( unsigned int i = 0; i < addrs.size(); i++ )
      addrs[i] = addrs[i] + map_info->mem_range.stride * rw->offset;

  for( unsigned int val_idx = 0; val_idx < rw->value.size(); val_idx++ )
  {
    /* calculate byte_enables */
    if (rw->element_kind == UVM_FIELD)
    {
      int idx = 0;
      n_access_extra = lsb%(bus_width*8);
      int temp_be = n_access_extra;

      while(temp_be >= 8)
      {
        byte_en[idx++] = 0;
        temp_be -= 8;
      }

      temp_be += n_bits;

      while(temp_be > 0)
      {
        byte_en[idx++] = 1;
        temp_be -= 8;
      }

      byte_en &= (1<<idx)-1;
      for( int i = 0; i<skip; i++ )
        addrs.erase(addrs.begin()); // pop_front()

      while (addrs.size() > (n_bits/(bus_width*8) + 1))
        addrs.pop_back();
    } // if

    curr_byte = 0;
    rw->value[val_idx] = 0;

    for( unsigned int i = 0; i < addrs.size(); i++ )
    {
      uvm_sequence_item* bus_req;
      uvm_reg_bus_op rw_access;
      uvm_reg_data_logic_t data;

      std::ostringstream inf;
      inf << "Reading address 0x"
          << std::hex << addrs[i].to_uint64()
          << " via map '"
          << get_full_name()
          << "'...";
      UVM_INFO(get_type_name(), inf.str(), UVM_FULL);

      if (rw->element_kind == UVM_FIELD)
        for( unsigned int z = 0; z < bus_width; z++)
          rw_access.byte_en[z] = byte_en[curr_byte+z];

      rw_access.kind = rw->access_kind;
      rw_access.addr = addrs[i];
      rw_access.data = 0;
      rw_access.byte_en = byte_en;
      rw_access.n_bits = (n_bits > bus_width*8) ? bus_width*8 : n_bits;

      adapter->m_set_item(rw);
      bus_req = adapter->reg2bus(rw_access);
      adapter->m_set_item(NULL);

      if (bus_req == NULL)
        UVM_FATAL("RegMem","Adapter [" + adapter->get_name() + "] did not return a bus transaction");

      bus_req->set_sequencer(sequencer);
      rw->parent->start_item(bus_req, rw->prior);

      if (rw->parent != NULL && i == 0)
        rw->parent->mid_do(rw);

      rw->parent->finish_item(bus_req);

      bus_req->end_event.wait_on();

      if (adapter->provides_responses)
      {
        uvm_sequence_item* bus_rsp;
        // TODO: need to test for right trans type, if not put back in q
        bus_rsp = rw->parent->get_base_response();
        adapter->bus2reg(bus_rsp, rw_access);
        rw->parent->del_base_response(bus_rsp); // remove item from response queue
      }
      else
        adapter->bus2reg(bus_req, rw_access);

      data = rw_access.data & uvm_mask_size(bus_width*8);
      rw->status = rw_access.status;

      std::string data_str = data.to_string(); // convert logic vector in string

      if( (rw->status == UVM_IS_OK) &&
          (data_str.find("X") < data_str.size()) ) // TODO check
        rw->status = UVM_HAS_X;

      std::ostringstream str1;
      str1 << "Read 0x"
           << data.to_string(sc_dt::SC_HEX_US, false)
           << " at address 0x"
           << std::hex << addrs[i].to_uint64()
           << " via map '"
           << get_full_name()
           << "': "
           << uvm_status_name[rw->status]
           << "...";

      UVM_INFO(get_type_name(), str1.str(), UVM_FULL);

      if (rw->status == UVM_NOT_OK)
        break;

      uvm_reg_data_t val = rw->value[val_idx];
      uvm_reg_data_t val2 = data << curr_byte * 8;
      val |= val2;
      rw->value[val_idx] = val;

      if (rw->parent != NULL && i == addrs.size()-1)
        rw->parent->post_do(rw);

      curr_byte += bus_width;
      n_bits -= bus_width * 8;
    }

    for( unsigned int i = 0; i < addrs.size(); i++ )
      addrs[i] = addrs[i] + map_info->mem_range.stride;

    if (rw->element_kind == UVM_FIELD)
      rw->value[val_idx] = (rw->value[val_idx] >> (n_access_extra)) & ((1<<size)-1);
  }
}

//----------------------------------------------------------------------
// member function: do_write
//
//! Perform a write operation.
//----------------------------------------------------------------------

void uvm_reg_map::do_write( uvm_reg_item* rw )
{
  uvm_sequence_base* tmp_parent_seq = NULL;
  uvm_reg_map* system_map = get_root_map();
  uvm_reg_adapter* adapter = system_map->get_adapter();
  uvm_sequencer_base* sequencer = system_map->get_sequencer();

  if (adapter != NULL && adapter->parent_sequence != NULL)
  {
    uvm_object* obj = adapter->parent_sequence->clone();
    uvm_sequence_base* seq = dynamic_cast<uvm_sequence_base*>(obj);
    assert(seq);

    seq->set_parent_sequence(rw->parent);
    rw->parent = seq;
    tmp_parent_seq = seq;
  }

  if (rw->parent == NULL)
  {
     uvm_sequence_base* seq = new uvm_sequence_base("default_parent_seq");
     rw->parent = seq;
     tmp_parent_seq = rw->parent;
  }

  if (adapter == NULL)
  {
    rw->set_sequencer(sequencer);
    rw->parent->start_item(rw, rw->prior);
    rw->parent->finish_item(rw);
    rw->end_event.wait_on();
  }
  else
    do_bus_write(rw, sequencer, adapter);

  if (tmp_parent_seq != NULL)
    sequencer->m_sequence_exiting(tmp_parent_seq);
}

//----------------------------------------------------------------------
// member function: do_read
//
//! Perform a read operation.
//----------------------------------------------------------------------

void uvm_reg_map::do_read( uvm_reg_item* rw )
{
  uvm_sequence_base* tmp_parent_seq = NULL;
  uvm_reg_map* system_map = get_root_map();
  uvm_reg_adapter* adapter = system_map->get_adapter();
  uvm_sequencer_base* sequencer = system_map->get_sequencer();

  if (adapter != NULL && adapter->parent_sequence != NULL)
  {
    uvm_object* obj = adapter->parent_sequence->clone();
    uvm_sequence_base* seq = dynamic_cast<uvm_sequence_base*>(obj);
    assert(seq);

    seq->set_parent_sequence(rw->parent);
    rw->parent = seq;
    tmp_parent_seq = seq;
  }

  if (rw->parent == NULL)
  {
    uvm_sequence_base* seq = new uvm_sequence_base("default_parent_seq");
    rw->parent = seq; // TODO check/delete afterwards
    tmp_parent_seq = rw->parent;
  }

  if (adapter == NULL)
  {
    rw->set_sequencer(sequencer);
    rw->parent->start_item(rw, rw->prior);
    rw->parent->finish_item(rw);
    rw->end_event.wait_on();
  }
  else
    do_bus_read(rw, sequencer, adapter);

  if (tmp_parent_seq != NULL)
    sequencer->m_sequence_exiting(tmp_parent_seq);
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------
// Destructor
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_map::~uvm_reg_map()
{
  // TODO clean-up memory...

  // delete packed
  // delete uvm_reg_map_info
  // delete uvm_sequence_base
  // delete uvm_reg_map/backdoor
}

//----------------------------------------------------------------------
// member function: add_parent_map
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_map::add_parent_map( uvm_reg_map* parent_map,
                                  uvm_reg_addr_t offset )
{
  if (parent_map == NULL)
  {
    UVM_ERROR("RegModel",
       "Attempting to add NULL parent map to map '" + get_full_name() + "'");
        return;
  }

  if (m_parent_map != NULL)
  {
    std::ostringstream str;
    str << "Map '"
        << get_full_name()
        << "' already a submap of map '"
        << m_parent_map->get_full_name()
        << "' at offset 0x"
        << std::hex << m_parent_map->get_submap_offset(this).to_uint64();
    UVM_ERROR("RegModel", str.str());
    return;
  }

  m_parent_map = parent_map;
  m_parent_maps[parent_map] = offset; // prep for multiple parents
  parent_map->m_submaps[this] = offset;
}

//----------------------------------------------------------------------
// member function: m_verify_map_config
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_map::m_verify_map_config() const
{
   // Make sure there is a generic payload sequence for each map
   // in the model and vice-versa if this is a root sequencer
   int error = 0;

   uvm_reg_map* root_map = get_root_map();

   if (root_map->get_adapter() == NULL)
   {
      UVM_ERROR("RegModel", "Map '" + root_map->get_full_name() +
        "' does not have an adapter registered");
      error++;
   }
   if (root_map->get_sequencer() == NULL)
   {
      UVM_ERROR("RegModel", "Map '" + root_map->get_full_name() +
        "' does not have a sequencer registered");
      error++;
   }
   if (error)
   {
      UVM_FATAL("RegModel", "Must register an adapter and sequencer for each top-level map in RegModel model");
      return;
   }
}

//----------------------------------------------------------------------
// member function: m_set_reg_offset
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_map::m_set_reg_offset( uvm_reg* rg,
                                    uvm_reg_addr_t offset,
                                    bool unmapped )
{
  if (m_regs_info.find(rg) == m_regs_info.end()) // not exists
  {
    UVM_ERROR("RegModel",
      "Cannot modify offset of register '" + rg->get_full_name() +
      "' in address map '" + get_full_name() +
      "' : register not mapped in that address map");
    return;
  }

  uvm_reg_map_info* info   = m_regs_info[rg];
  uvm_reg_block*   blk     = get_parent();
  uvm_reg_map*     top_map = get_root_map();
  std::vector<uvm_reg_addr_t> addrs;

  // if block is not locked, m_init_address_map will resolve map when block is locked
  if (blk->is_locked())
  {
    // remove any existing cached addresses
    if (!info->unmapped)
    {
      for( unsigned int i = 0; i < info->addr.size(); i++)
      {
        if (top_map->m_regs_by_offset_wo.find(info->addr[i]) == top_map->m_regs_by_offset_wo.end()) // not exists
          top_map->m_regs_by_offset.erase(info->addr[i]);
        else
        {
          if (top_map->m_regs_by_offset[info->addr[i]] == rg)
          {
            top_map->m_regs_by_offset[info->addr[i]] =
                top_map->m_regs_by_offset_wo[info->addr[i]];

            uvm_reg_read_only_cbs::remove(rg);
            uvm_reg_write_only_cbs::remove(top_map->m_regs_by_offset[info->addr[i]]);
          }
          else
          {
            uvm_reg_write_only_cbs::remove(rg);
            uvm_reg_read_only_cbs::remove(top_map->m_regs_by_offset[info->addr[i]]);
          }
          top_map->m_regs_by_offset_wo.erase(info->addr[i]);
        }
      } // for
    } // if unmapped

    // if we are remapping...
    if (!unmapped)
    {
      std::string rg_acc = rg->m_get_fields_access(this);

      // get new addresses
      get_physical_addresses(offset, 0, rg->get_n_bytes(), addrs);

      // make sure they do not conflict with others
      for( unsigned int i = 0; i < addrs.size(); i++ )
      {
        uvm_reg_addr_t addr = addrs[i];
        if (top_map->m_regs_by_offset.find(addr) != top_map->m_regs_by_offset.end()) //exists
        {
          uvm_reg* rg2 = top_map->m_regs_by_offset[addr];
          std::string rg2_acc = rg2->m_get_fields_access(this);

          // If the register at the same address is RO or WO
          // and this register is WO or RO, this is OK
          if (rg_acc == "RO" && rg2_acc == "WO")
          {
            top_map->m_regs_by_offset[addr] = rg;
            uvm_reg_read_only_cbs::add(rg);
            top_map->m_regs_by_offset_wo[addr] = rg2;
            uvm_reg_write_only_cbs::add(rg2);
          }
          else
            if (rg_acc == "WO" && rg2_acc == "RO")
            {
              top_map->m_regs_by_offset_wo[addr] = rg;
              uvm_reg_write_only_cbs::add(rg);
              uvm_reg_read_only_cbs::add(rg2);
            }
            else
            {
              std::ostringstream str;
              str << "In map '"
                  << get_full_name()
                  << "' register '"
                  << rg->get_full_name()
                  << "' maps to same address as register '"
                  << top_map->m_regs_by_offset[addr]->get_full_name()
                  << "': 0x"
                  << std::hex << addr.to_uint64();
              UVM_WARNING("RegModel", str.str());
            }
        }
        else
          top_map->m_regs_by_offset[addr] = rg;

        for( m_mems_by_offset_itt it = top_map->m_mems_by_offset.begin();
             it != top_map->m_mems_by_offset.end();
             it++ )
        {
          uvm_reg_map_addr_range* range = (*it).first;
          if (addrs[i] >= range->min && addrs[i] <= range->max)
          {
            std::ostringstream str;
            str << "In map '"
                << get_full_name()
                << "' register '"
                << rg->get_full_name()
                << "' overlaps with address range of memory '"
                << top_map->m_mems_by_offset[range]->get_full_name()
                << "': 0x"
                << std::hex << addrs[i].to_uint64();
            UVM_WARNING("RegModel", str.str());
          }
        }
      }
      info->addr = addrs; // cache it
    }
  } // if blk is locked

  if (unmapped)
  {
    info->offset   = -1;
    info->unmapped = true;
  }
  else
  {
    info->offset   = offset;
    info->unmapped = false;
  }
}

//----------------------------------------------------------------------
// member function: m_set_mem_offset
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_map::m_set_mem_offset( uvm_mem* mem,
                                    uvm_reg_addr_t offset,
                                    bool unmapped )
{
  if (m_mems_info.find(mem) == m_mems_info.end()) // not exists
  {
    UVM_ERROR("RegModel",
       "Cannot modify offset of memory '" + mem->get_full_name() +
       "' in address map '" + get_full_name() +
       "' : memory not mapped in that address map");
    return;
  }

  uvm_reg_map_info* info   = m_mems_info[mem];
  uvm_reg_block*    blk     = get_parent();
  uvm_reg_map*      top_map = get_root_map();
  std::vector<uvm_reg_addr_t> addrs;

  // if block is not locked, m_init_address_map will resolve map when block is locked
  if (blk->is_locked())
  {
    // remove any existing cached addresses
    if (!info->unmapped)
    {
      for( m_mems_by_offset_itt it = top_map->m_mems_by_offset.begin();
           it != top_map->m_mems_by_offset.end();
           it++ )
      {
        uvm_reg_map_addr_range* range = (*it).first;
        if (top_map->m_mems_by_offset[range] == mem)
          top_map->m_mems_by_offset.erase(range);
      }
    }

    // if we are remapping...
    if (!unmapped)
    {
      std::vector<uvm_reg_addr_t> addrs;
      std::vector<uvm_reg_addr_t> addrs_max;
      uvm_reg_addr_t min, max, min2, max2;
      unsigned long stride;

      get_physical_addresses(offset, 0, mem->get_n_bytes(), addrs);

      min = (addrs[0] < addrs[addrs.size()-1]) ? addrs[0] : addrs[addrs.size()-1];
      min2 = addrs[0];

      get_physical_addresses(offset, (mem->get_size()-1), mem->get_n_bytes(), addrs_max);

      max = (addrs_max[0] > addrs_max[addrs_max.size()-1]) ?
          addrs_max[0] : addrs_max[addrs_max.size()-1];
      max2 = addrs_max[0];
      // address interval between consecutive mem locations
      stride = (max2 - max)/(mem->get_size()-1);

      // make sure new offset does not conflict with others
      for( m_regs_by_offset_itt it = top_map->m_regs_by_offset.begin();
           it != top_map->m_regs_by_offset.end();
           it++)
      {
        uvm_reg_addr_t reg_addr = (*it).first;
        if ( reg_addr >= min &&  reg_addr <= max)
        {
          std::ostringstream str;
          str << "In map '"
              << get_full_name()
              << "' memory '"
              << mem->get_full_name()
              << "' with range ["
              << min
              << ":"
              << max
              << "] overlaps with address of existing register '"
              << top_map->m_regs_by_offset[reg_addr]->get_full_name()
              << "': "
              << reg_addr;
          UVM_WARNING("RegModel", str.str() );
        }
      }

      for( m_mems_by_offset_itt it = top_map->m_mems_by_offset.begin();
           it != top_map->m_mems_by_offset.end();
           it++ )
      {
        uvm_reg_map_addr_range* range = (*it).first;
        if ( ( min <= range->max && max >= range->max ) ||
             ( min <= range->min && max >= range->min ) ||
             ( min >= range->min && max <= range->max ) )
        {
          std::ostringstream str;
          str << "In map '"
              << get_full_name()
              << "' memory '"
              << mem->get_full_name()
              << "' with range ["
              << min
              << ":"
              << max
              << "] overlaps existing memory with range '"
              << top_map->m_mems_by_offset[range]->get_full_name()
              << "': ["
              << range->min
              << ":"
              << range->max
              << "]";
          UVM_WARNING("RegModel", str.str());
        }
      }

      uvm_reg_map_addr_range* range = new packed();
      range->min = min;
      range->max = max;
      range->stride = stride;

      top_map->m_mems_by_offset[range] = mem;

      info->addr  = addrs;
      info->mem_range = *range;
    }
  }

  if (unmapped)
  {
    info->offset   = -1;
    info->unmapped = true;
  }
  else
  {
    info->offset   = offset;
    info->unmapped = false;
  }

}

//----------------------------------------------------------------------
// member function: get_reg_map_info
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_map_info* uvm_reg_map::get_reg_map_info( const uvm_reg* rg, bool error ) const
{
  uvm_reg_map_info* result = NULL;
  uvm_reg* reg = const_cast<uvm_reg*>(rg);

  if (m_regs_info.find(reg) == m_regs_info.end()) // not exists
  {
    if (error)
      UVM_ERROR("REG_NO_MAP","Register '" + reg->get_name() +
          "' not in map '" + get_name() + "'");
    return NULL;
  }

  result = m_regs_info.find(reg)->second;

  if(!result->is_initialized)
    UVM_WARNING("RegModel",
      "map '" + get_name() + "' does not seem to be initialized correctly, check that the top register model is locked()");

  return result;
}

//----------------------------------------------------------------------
// member function: get_mem_map_info
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_map_info* uvm_reg_map::get_mem_map_info( const uvm_mem* mem, bool error ) const
{

  uvm_mem* lmem = const_cast<uvm_mem*>(mem);

  if ( m_mems_info.find(lmem) == m_mems_info.end() ) // not exists
  {
    if (error)
      UVM_ERROR("REG_NO_MAP", "Memory '" + lmem->get_name() + "' not in map '" + get_name() + "'");
    return NULL;
  }
  return m_mems_info.find(lmem)->second;
}

//----------------------------------------------------------------------
// member function: get_size
//
// Implementation defined
//----------------------------------------------------------------------

unsigned int uvm_reg_map::get_size() const
{
  unsigned int addr = 0;
  unsigned int max_addr = addr;

  // get max offset from registers
  for( m_regs_info_citt it = m_regs_info.begin();
       it != m_regs_info.end();
       it++)
  {
    uvm_reg* rg = (*it).first;
    addr = m_regs_info.find(rg)->second->offset + ((rg->get_n_bytes()-1)/m_n_bytes);
    if (addr > max_addr)
      max_addr = addr;
  }

  // get max offset from memories
  for( m_mems_info_citt it = m_mems_info.begin();
       it != m_mems_info.end();
       it++ )
  {
    uvm_mem* mem = (*it).first;
    addr = m_mems_info.find(mem)->second->offset + (mem->get_size() * (((mem->get_n_bytes()-1)/m_n_bytes)+1)) -1;
    if (addr > max_addr)
      max_addr = addr;
  }

  // get max offset from submaps
  for( m_submaps_citt it = m_submaps.begin();
       it != m_submaps.end();
       it++ )
  {
    uvm_reg_map* submap = (*it).first;
    addr = m_submaps.find(submap)->second + submap->get_size();
    if (addr > max_addr)
      max_addr = addr;
  }

  return max_addr + 1;
}

//----------------------------------------------------------------------
// member function: backdoor
//
// Implementation defined
//
// Return the backdoor pseudo-map singleton
//
// This pseudo-map is used to specify or configure the backdoor
// instead of a real address map.
//----------------------------------------------------------------------

uvm_reg_map* uvm_reg_map::backdoor()
{
   if (m_backdoor == NULL)
     m_backdoor = new uvm_reg_map("Backdoor");

   return m_backdoor;
}

//----------------------------------------------------------------------
// member function: m_get_bus_info
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_map::m_get_bus_info( uvm_reg_item* rw,
                                  uvm_reg_map_info*& map_info,
                                  unsigned int& size,
                                  int& lsb,
                                  int& addr_skip ) const
{
  if (rw->element_kind == UVM_MEM)
  {
    uvm_mem* mem = dynamic_cast<uvm_mem*>(rw->element);
    if(rw->element == NULL || mem == NULL )
      UVM_FATAL("REG/CAST", "uvm_reg_item 'element_kind' is UVM_MEM, but 'element' does not point to a memory: "
        + rw->get_name() );
    map_info = get_mem_map_info(mem);
    size = mem->get_n_bits();
  }
  else
    if (rw->element_kind == UVM_REG)
    {
      uvm_reg* rg;
      rg = dynamic_cast<uvm_reg*>(rw->element);
      if(rw->element == NULL || rg == NULL)
        UVM_FATAL("REG/CAST", "uvm_reg_item 'element_kind' is UVM_REG, but 'element' does not point to a register: "
          + rw->get_name() );
      map_info = get_reg_map_info(rg);
      size = rg->get_n_bits();
    }
    else
      if (rw->element_kind == UVM_FIELD)
      {
        uvm_reg_field* field = dynamic_cast<uvm_reg_field*>(rw->element);
        if(rw->element == NULL || field == NULL)
          UVM_FATAL("REG/CAST", "uvm_reg_item 'element_kind' is UVM_FIELD, but 'element' does not point to a field: "
            + rw->get_name() );
        map_info = get_reg_map_info(field->get_parent());
        size = field->get_n_bits();
        lsb = field->get_lsb_pos();
        addr_skip = lsb/(get_n_bytes()*8);
      }
}

//----------------------------------------------------------------------
// member function: m_init_address_map
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_map::m_init_address_map()
{
  unsigned int bus_width = 0;

  uvm_reg_map* top_map = get_root_map();

  if (this == top_map)
  {
    top_map->m_regs_by_offset.clear(); // delete all
    top_map->m_regs_by_offset_wo.clear(); // delete all
    top_map->m_mems_by_offset.clear(); // delete all
  }

  for( m_submaps_itt it = m_submaps.begin();
      it != m_submaps.end();
      it++ )
  {
    uvm_reg_map* map = (*it).first;
    map->m_init_address_map();
  }

  for( m_regs_info_itt it = m_regs_info.begin();
      it != m_regs_info.end();
      it++ )
  {
    uvm_reg* rg = (*it).first;
    m_regs_info[rg]->is_initialized = true;

    if (!m_regs_info[rg]->unmapped)
    {
      std::string rg_acc = rg->m_get_fields_access(this);
      std::vector<uvm_reg_addr_t> addrs;

      bus_width = get_physical_addresses( m_regs_info[rg]->offset, 0, rg->get_n_bytes(), addrs);

      for( unsigned int i = 0; i < addrs.size(); i++ )
      {
        uvm_reg_addr_t addr = addrs[i];

        if( top_map->m_regs_by_offset.find(addr) != top_map->m_regs_by_offset.end() ) // exists
        {
          uvm_reg* rg2 = top_map->m_regs_by_offset[addr];
          std::string rg2_acc = rg2->m_get_fields_access(this);

          // If the register at the same address is RO or WO
          // and this register is WO or RO, this is OK
          if (rg_acc == "RO" && rg2_acc == "WO")
          {
            top_map->m_regs_by_offset[addr] = rg;
            uvm_reg_read_only_cbs::add(rg);

            top_map->m_regs_by_offset_wo[addr] = rg2;
            uvm_reg_write_only_cbs::add(rg2);
          }
          else if (rg_acc == "WO" && rg2_acc == "RO")
          {
            top_map->m_regs_by_offset_wo[addr] = rg;
            uvm_reg_write_only_cbs::add(rg);
            uvm_reg_read_only_cbs::add(rg2);
          }
          else
          {
            std::ostringstream str;
            str << "In map '"
                << get_full_name()
                << "' register '"
                << rg->get_full_name()
                << "' maps to same address as register '"
                << top_map->m_regs_by_offset[addr]->get_full_name()
                << "': 0x"
                << std::hex << addr.to_uint64();
            UVM_WARNING("RegModel", str.str());
          }
        }
        else
          top_map->m_regs_by_offset[addr] = rg;

        for( m_mems_by_offset_itt ita = m_mems_by_offset.begin();
             ita != m_mems_by_offset.end();
             ita++ )
        {
          uvm_reg_map_addr_range* range = (*ita).first;

          if (addr >= range->min && addr <= range->max)
          {
            std::ostringstream str;
            str << "In map '"
                << get_full_name()
                << "' register '"
                << rg->get_full_name()
                << "' with address 0x"
                << std::hex << addr.to_uint64()
                << " maps to same address as memory '"
                << top_map->m_mems_by_offset[range]->get_full_name()
                << "': ["
                << range->min
                << ":"
                << std::dec << range->max
                << "]";
            UVM_WARNING("RegModel", str.str() );
          }
        }
      }
      m_regs_info[rg]->addr = addrs;
    }
  }

  for( m_mems_info_itt it = m_mems_info.begin();
       it != m_mems_info.end();
       it++ )
  {
    uvm_mem* mem = (*it).first;

    if (!m_mems_info[mem]->unmapped)
    {
      std::vector<uvm_reg_addr_t> addrs;
      std::vector<uvm_reg_addr_t> addrs_max;
      uvm_reg_addr_t min, max, min2, max2;
      unsigned long stride;

      bus_width = get_physical_addresses( m_mems_info[mem]->offset, 0, mem->get_n_bytes(), addrs);

      min = (addrs[0] < addrs[addrs.size()-1]) ? addrs[0] : addrs[addrs.size()-1];
      min2 = addrs[0];

      get_physical_addresses( m_mems_info[mem]->offset,
                              (mem->get_size()-1), mem->get_n_bytes(), addrs_max);

      max = (addrs_max[0] > addrs_max[addrs_max.size()-1]) ?
        addrs_max[0] : addrs_max[addrs_max.size()-1];
      max2 = addrs_max[0];

      // address interval between consecutive mem offsets
      stride = (max2 - min2)/(mem->get_size()-1);

      for( m_regs_by_offset_itt ita = top_map->m_regs_by_offset.begin();
          ita != top_map->m_regs_by_offset.end();
          ita++ )
      {
        uvm_reg_addr_t reg_addr = (*ita).first;

        if (reg_addr >= min && reg_addr <= max)
        {
          std::ostringstream str;
          str << "In map '"
              << get_full_name()
              << "' memory '"
              << mem->get_full_name()
              << "' maps to same address as register '"
              << top_map->m_regs_by_offset[reg_addr]->get_full_name()
              << "': 0x"
              << std::hex << reg_addr.to_uint64();
          UVM_WARNING("RegModel", str.str() );
        }
      }

      for( m_mems_by_offset_itt itb = m_mems_by_offset.begin();
           itb != m_mems_by_offset.end();
           itb++ )
      {
        uvm_reg_map_addr_range* range = (*itb).first;

        if ( ( min <= range->max && max >= range->max ) ||
             ( min <= range->min && max >= range->min ) ||
             ( min >= range->min && max <= range->max ) )
        {
          std::ostringstream str;
          str << "In map '"
              << get_full_name()
              << "' memory '"
              << mem->get_full_name()
              << "' overlaps with address range of memory '"
              << top_map->m_mems_by_offset[range]->get_full_name()
              << "': ["
              << min
              << ":"
              << max
              << "]";
          UVM_WARNING("RegModel", str.str() );
        }
      }

      {
        uvm_reg_map_addr_range* _range = new packed();
        _range->min = min;
        _range->max = max;
        _range->stride = stride;
        top_map->m_mems_by_offset[_range] = mem;
        m_mems_info[mem]->addr = addrs;
        m_mems_info[mem]->mem_range = *_range;
      }
    }
  }

  // If the block has no registers or memories,
  // bus_width won't be set
  if (bus_width == 0) bus_width = m_n_bytes;

  m_system_n_bytes = bus_width;
}

//----------------------------------------------------------------------------
// Implementation defined: UVM object methods
//----------------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: convert2string
//
// Implementation defined
//----------------------------------------------------------------------

std::string uvm_reg_map::convert2string() const
{
  std::vector<uvm_reg*> regs;
  std::vector<uvm_vreg*> vregs;
  std::vector<uvm_mem*> mems;
  uvm_endianness_e endian;
  std::string prefix;

  std::ostringstream str;

  str << prefix
      << "Map "
      << get_full_name();

  endian = get_endian(UVM_NO_HIER);

  str << " -- "
      << get_n_bytes(UVM_NO_HIER)
      << " bytes ("
      << uvm_endianness_name[endian]
      << ")";

  get_registers(regs);

  for( unsigned int j = 0; j < regs.size(); j++ )
  {
    str << std::endl << regs[j]->convert2string(); // TODO in UVM-SV?? {prefix, "   "}, this));
  }

  get_memories(mems);

  for( unsigned int j = 0; j < mems.size(); j++ )
  {
    str << std::endl << mems[j]->convert2string(); // TODO in UVM-SV?? {prefix, "   "}, this));
  }

  get_virtual_registers(vregs);

  for( unsigned int j = 0; j < vregs.size(); j++ )
  {
    str << std::endl << vregs[j]->convert2string(); // TODO in UVM-SV?? {prefix, "   "}, this));
  }

  return str.str();
}

//----------------------------------------------------------------------
// member function: clone
//
// TODO: not implemented yet
//----------------------------------------------------------------------

uvm_object* uvm_reg_map::clone()
{
  //uvm_rag_map* me; // TODO reg_map clone
  //me = new this;
  //return me;
  return NULL; // dummy
}

//----------------------------------------------------------------------
// member function: do_print
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_map::do_print( const uvm_printer& printer ) const
{
  std::vector<uvm_reg*> regs;
  std::vector<uvm_vreg*> vregs;
  std::vector<uvm_mem*> mems;
  uvm_endianness_e endian;
  std::vector<uvm_reg_map*> maps;
  std::string prefix;

  uvm_sequencer_base* sqr = get_sequencer();

  uvm_object::do_print(printer);

  //UVMSV commented
  //printer.print_generic(get_name(), get_type_name(), -1, convert2string());

  endian = get_endian(UVM_NO_HIER);

  // UVMSV commented
  //   $sformat(convert2string, "%s -- %0d bytes (%s)", convert2string,
  //            get_n_bytes(UVM_NO_HIER), uvm_endianness_name[endian]);

  printer.print_generic("endian", "", -2, std::string(uvm_endianness_name[endian]) );

  if( sqr != NULL )
    printer.print_generic("effective sequencer", sqr->get_type_name(), -2, sqr->get_full_name());

  get_registers(regs, UVM_NO_HIER);

  for( unsigned int j = 0; j < regs.size(); j++ )
  {
    std::ostringstream str;
    str << "@"
        << regs[j]->get_inst_id()
        << "+ 0x"
        << std::hex << regs[j]->get_address(this).to_uint64();
    printer.print_generic( regs[j]->get_name(), regs[j]->get_type_name(), -2, str.str() );
  }

  get_memories(mems);

  for( unsigned int j = 0; j < mems.size(); j++ )
  {
    std::ostringstream str;
    str << "@"
        << mems[j]->get_inst_id()
        << " + 0x"
        << std::hex << mems[j]->get_address(0,this).to_uint64();
    printer.print_generic( mems[j]->get_name(), mems[j]->get_type_name(), -2, str.str() );
  }

  get_virtual_registers(vregs);

  for( unsigned int j = 0; j < vregs.size(); j++ )
  {
    std::ostringstream str;
    str << "@"
        << vregs[j]->get_inst_id()
        << " + 0x"
        << std::hex << vregs[j]->get_address(0, this).to_uint64();
    printer.print_generic( vregs[j]->get_name(), vregs[j]->get_type_name(),-2, str.str() );
  }

  get_submaps(maps);

  for( unsigned int j = 0; j < maps.size(); j++ )
  {
    printer.print_object( maps[j]->get_name(), *maps[j] );
  }
}


//----------------------------------------------------------------------
// member function: do_copy
//
// TODO: not implemented yet?
//----------------------------------------------------------------------

void uvm_reg_map::do_copy( const uvm_object& rhs )
{
  /* TODO do_copy
  uvm_reg_map* rhs_ = dynamic_cast<uvm_reg_map*>(&rhs)

  if (rhs_ == NULL)
    UVM_ERROR("");

  rhs_->regs = regs;
  rhs_->mems = mems;
  rhs_->vregs = vregs;
  rhs_.blks = blks;
  //... and so on
 */
}

} // namespace uvm
