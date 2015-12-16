//----------------------------------------------------------------------
//   Copyright 2013-2015 NXP B.V.
//   Copyright 2004-2011 Synopsys, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2015-2015 Agnisys, Inc.
//
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

#include "uvmsc/reg/uvm_reg_block.h"
#include "uvmsc/reg/uvm_reg_map.h"
#include "uvmsc/reg/uvm_mem.h"
#include "uvmsc/reg/uvm_reg_field.h"
#include "uvmsc/reg/uvm_vreg.h"
#include "uvmsc/reg/uvm_vreg_field.h"
#include "uvmsc/reg/uvm_reg_backdoor.h"


namespace uvm {

//------------------------------------------------------------------------------
// static data member initialization
//------------------------------------------------------------------------------

int uvm_reg_block::id = 0;
uvm_reg_block::m_rootsT uvm_reg_block::m_roots;

//----------------------------------------------------------------------
// Group: Initialization
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Constructor
//
//! Create a new instance and type-specific configuration
//!
//! Creates an instance of a block abstraction class with the specified
//! name.
//!
//! \p has_coverage specifies which functional coverage models are present in
//! the extension of the block abstraction class.
//! Multiple functional coverage models may be specified by adding their
//! symbolic names, as defined by the #uvm_coverage_model_e type.
//----------------------------------------------------------------------

uvm_reg_block::uvm_reg_block( const std::string& name,
                              int has_coverage) : uvm_object(name)
{
  m_parent = NULL;

  m_blks.clear();
  m_regs.clear();
  m_vregs.clear();
  m_mems.clear();
  m_maps.clear();
  m_backdoor = NULL;
  m_root_hdl_paths.clear();

  default_path = UVM_DEFAULT_PATH;
  m_default_hdl_path = "RTL";

  m_hdl_paths_pool = new uvm_object_string_pool<uvm_queue<std::string>* >("hdl_paths");
  this->m_has_cover = has_coverage;

  // Root block until registered with a parent
  m_roots[this] = 0;

  m_locked = false;
  m_has_cover = 0;
  m_cover_on = 0;
  m_fname = "";
  m_lineno = 0;
}

//----------------------------------------------------------------------
// member function: configure
//
//! Instance-specific configuration
//!
//! Specify the parent block of this block.
//! A block without parent is a root block.
//!
//! If the block file corresponds to a hierarchical RTL structure,
//! it's contribution to the HDL path is specified as the \p hdl_path.
//! Otherwise, the block does not correspond to a hierarchical RTL
//! structure (e.g. it is physically flattened) and does not contribute
//! to the hierarchical HDL path of any contained registers or memories.
//----------------------------------------------------------------------

void uvm_reg_block::configure( uvm_reg_block* parent,
                               const std::string& hdl_path)
{
  m_parent = parent;

  if (m_parent != NULL)
    m_parent->add_block(this);

  add_hdl_path(hdl_path);

  uvm_resource_db<uvm_reg_block*>::set("uvm_reg::*", this->get_full_name(), this);
}

//----------------------------------------------------------------------
// member function: create_map
//
//! Create an address map in this block
//!
//! Create an address map with the specified \p name, then
//! configures it with the following properties.
//!
//! base_addr - the base address for the map. All registers, memories,
//!             and sub-blocks within the map will be at offsets to this
//!             address
//!
//! n_bytes   - the byte-width of the bus on which this map is used
//!
//! endian    - the endian format. See #uvm_endianness_e for possible
//!             values
//!
//! byte_addressing - specifies whether consecutive addresses refer are 1 byte
//!             apart (TRUE) or \p n_bytes apart (FALSE). Default is TRUE.
//----------------------------------------------------------------------

uvm_reg_map* uvm_reg_block::create_map( const std::string& name,
                                        uvm_reg_addr_t base_addr,
                                        unsigned int n_bytes,
                                        uvm_endianness_e endian,
                                        bool byte_addressing )
{
  uvm_reg_map* map;

  if (this->is_locked() )
  {
    UVM_ERROR("RegModel", "Cannot add map to locked model");
    return NULL;
  }

  map = uvm_reg_map::type_id::create(name, NULL, this->get_full_name());

  map->configure(this, base_addr, n_bytes, endian, byte_addressing);

  m_maps[map] = true;
  if (m_maps.size() == 1)
    default_map = map;

  return map;
}


//----------------------------------------------------------------------
// member function: check_data_width
//
//! Check that the specified data width (in bits) is less than
//! or equal to the value of UVM_REG_DATA_WIDTH
//!
//! This method is designed to be called by a static initializer
//----------------------------------------------------------------------

bool uvm_reg_block::check_data_width( unsigned int width )
{
  if (width <= UVM_REG_DATA_WIDTH) // was $bits(uvm_reg_data_t)
    return true;

  std::ostringstream str;
  str << "Register model requires that UVM_REG_DATA_WIDTH be defined as "
      << width
      << " or greater. Currently defined as "
      << UVM_REG_DATA_WIDTH;

  UVM_FATAL("RegModel", str.str());
  return false;
}

//----------------------------------------------------------------------
// member function: set_default_map
//
//! Defines the default address map
//!
//! Set the specified address map as the #default_map for this
//! block. The address map must be a map of this address block.
//----------------------------------------------------------------------

void uvm_reg_block::set_default_map( uvm_reg_map* map )
{
  if ( m_maps.find(map) == m_maps.end()) // if not exists
   UVM_WARNING("RegModel", "Map '" + map->get_full_name() + "' does not exist in block");

  default_map = map;
}

//----------------------------------------------------------------------
// member function: get_default_map
//
//! Retrieves the default address map
//!
//! Get the specified address map as the #default_map for this
//! block.
//----------------------------------------------------------------------

uvm_reg_map* uvm_reg_block::get_default_map() const
{
  return default_map;
}


//----------------------------------------------------------------------
// member function: lock_model
//
//! Lock a model and build the address map.
//!
//! Recursively lock an entire register model
//! and build the address maps to enable the
//! uvm_reg_map::get_reg_by_offset() and
//! uvm_reg_map::get_mem_by_offset() methods.
//!
//! Once locked, no further structural changes,
//! such as adding registers or memories,
//! can be made.
//!
//! It is not possible to unlock a model.
//----------------------------------------------------------------------

void uvm_reg_block::lock_model()
{
  if(is_locked())
    return;

  m_locked = true;

  for( m_regs_itt it = m_regs.begin(); it != m_regs.end(); it++ )
  {
    uvm_reg* rg = (*it).first;
    rg->m_lock_model();
  }

  for( m_mems_itt it =m_mems.begin(); it != m_mems.end(); it++ )
  {
    uvm_mem* mem = (*it).first;
    mem->m_lock_model();
  }

  for( m_blks_itt it = m_blks.begin(); it !=  m_blks.end(); it++ )
  {
    uvm_reg_block* blk = (*it).first;
    blk->lock_model();
  }

  if (m_parent == NULL)
  {
    unsigned int max_size = uvm_reg::get_max_size();

    if (uvm_reg_field::get_max_size() > max_size)
      max_size = uvm_reg_field::get_max_size();

    if (uvm_mem::get_max_size() > max_size)
      max_size = uvm_mem::get_max_size();

    if ( (int)max_size > UVM_REG_DATA_WIDTH)
    {
      std::ostringstream str;
      str << "Register model requires that UVM_REG_DATA_WIDTH be defined as "
          << max_size
          << " or greater. Currently defined as "
          << UVM_REG_DATA_WIDTH;
      UVM_FATAL("RegModel", str.str() );
    }

    m_init_address_maps();

    // Check that root register models have unique names

    // Has this name has been checked before?
    if (m_roots[this] != true)
    {
      int n = 0;

      for( m_roots_itt it = m_roots.begin(); it != m_roots.end(); it++)
      {
        uvm_reg_block* blk = (*it).first;

        if (blk->get_name() == get_name())
        {
          m_roots[blk] = true;
          n++;
        }
      }

      if (n > 1)
      {
        std::ostringstream str;
        str << "There are " << n
            << " root register models named '"
            << get_name()
            << "'. The names of the root register models have to be unique.";
        UVM_ERROR("UVM/REG/DUPLROOT", str.str());
      }
    }
  }
}

//----------------------------------------------------------------------
// member function: is_locked
//
//! Return true if the model is locked.
//----------------------------------------------------------------------

bool uvm_reg_block::is_locked() const
{
   return m_locked;
}

//----------------------------------------------------------------------
// Group: Introspection
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// member function: get_name
//
//! Get the simple name
//!
//! Return the simple object name of this block.
//----------------------------------------------------------------------

// TODO: should we implement this?

//----------------------------------------------------------------------
// member function: get_full_name
//
//! Get the hierarchical name
//!
//! Return the hierarchical name of this block.
//! The base of the hierarchical name is the root block.
//----------------------------------------------------------------------

const std::string uvm_reg_block::get_full_name() const
{
  if (m_parent == NULL)
    return get_name();

  return m_parent->get_full_name() + "." + get_name();
}

//----------------------------------------------------------------------
// member function: get_parent
//
//! Get the parent block
//!
//! If this a top-level block, returns NULL.
//----------------------------------------------------------------------

uvm_reg_block* uvm_reg_block::get_parent() const
{
  return m_parent;
}


//----------------------------------------------------------------------
// member function: get_root_blocks
//
//! Get the all root blocks
//!
//! Returns an array of all root blocks in the simulation.
//----------------------------------------------------------------------

void uvm_reg_block::get_root_blocks( std::vector<uvm_reg_block*>& blks )
{
  for( m_roots_itt it = m_roots.begin(); it != m_roots.end(); it++ )
    blks.push_back( (*it).first );
}

//----------------------------------------------------------------------
// member function: find_blocks
//
//! Find the blocks whose hierarchical names match the
//! specified \p name glob.
//! If a ~root~ block is specified, the name of the blocks are
//! relative to that block, otherwise they are absolute.
//!
//! Returns the number of blocks found.
//----------------------------------------------------------------------

int uvm_reg_block::find_blocks( std::string name,
                                std::vector<uvm_reg_block*>& blks,
                                uvm_reg_block* root,
                                uvm_object* accessor )
{
  uvm_resource_pool* rpl = uvm_resource_pool::get();
  uvm_resource_types::rsrc_q_t* rs;

  blks.clear(); // delete all entries

  if (root != NULL)
    name = root->get_full_name() + "." + name;

  rs = rpl->lookup_regex(name, "uvm_reg::");

  for ( int i = 0; i < rs->size(); i++)
  {
    uvm_resource<uvm_reg_block*>* blk;
    blk = dynamic_cast<uvm_resource<uvm_reg_block*>*>(rs->get(i));
    if (blk == NULL)
      continue;
    blks.push_back(blk->read(accessor));
  }

  return blks.size();
}

//----------------------------------------------------------------------
// member function: find_block (static)
//
//! Find the first block whose hierarchical names match the
//! specified \p name glob.
//! If a ~root~ block is specified, the name of the blocks are
//! relative to that block, otherwise they are absolute.
//!
//! Returns the first block found or NULL otherwise.
//! A warning is issued if more than one block is found.
//----------------------------------------------------------------------

uvm_reg_block* uvm_reg_block::find_block( const std::string& name,
                                          uvm_reg_block* root,
                                          uvm_object* accessor )
{
  std::vector<uvm_reg_block*> blks;
  if (!find_blocks(name, blks, root, accessor))
    return NULL;

  if (blks.size() > 1)
  {
    UVM_WARNING("MRTH1BLK",
      "More than one block matched the name \"" + name + "\".");
  }
  return blks[0];
}

//----------------------------------------------------------------------
// member function: get_blocks (virtual)
//
//! Get the sub-blocks
//!
//! Get the blocks instantiated in this blocks.
//! If \p hier is true, recursively includes any sub-blocks.
//----------------------------------------------------------------------

void uvm_reg_block::get_blocks( std::vector<uvm_reg_block*>& blks,
                                uvm_hier_e hier ) const
{
  for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    uvm_reg_block* blk = (*it).first;
    blks.push_back(blk);

    if (hier == UVM_HIER)
      blk->get_blocks(blks);
  }
}

//----------------------------------------------------------------------
// member function: get_maps (virtual)
//
//! Get the address maps
//!
//! Get the address maps instantiated in this block.
//----------------------------------------------------------------------

void uvm_reg_block::get_maps( std::vector<uvm_reg_map*>& maps ) const
{
  for( m_maps_citt it = m_maps.begin(); it != m_maps.end(); it++)
    maps.push_back( (*it).first );
}

//----------------------------------------------------------------------
// member function: get_registers (virtual)
//
//! Get the registers
//!
//! Get the registers instantiated in this block.
//! If \p hier is true, recursively includes the registers
//! in the sub-blocks.
//!
//! Note that registers may be located in different and/or multiple
//! address maps. To get the registers in a specific address map,
//! use the uvm_reg_map::get_registers() method.
//----------------------------------------------------------------------

void uvm_reg_block::get_registers( std::vector<uvm_reg*>& regs,
                                   uvm_hier_e hier ) const
{
  for( m_regs_citt it = m_regs.begin(); it != m_regs.end(); it++ )
    regs.push_back( (*it).first );

  if (hier == UVM_HIER)
  {
    for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
    {
      uvm_reg_block* blk = (*it).first;
      blk->get_registers(regs);
    }
  }
}

//----------------------------------------------------------------------
// member function: get_fields (virtual)
//
//! Get the fields
//!
//! Get the fields in the registers instantiated in this block.
//! If \p hier is true, recursively includes the fields of the registers
//! in the sub-blocks.
//----------------------------------------------------------------------

void uvm_reg_block::get_fields( std::vector<uvm_reg_field*>& fields,
                                uvm_hier_e hier ) const
{
  for( m_regs_citt it = m_regs.begin(); it != m_regs.end(); it++ )
  {
    uvm_reg* rg = (*it).first;
    rg->get_fields(fields);
  }

  if (hier == UVM_HIER)
  {
    for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
    {
      uvm_reg_block* blk = (*it).first;
      blk->get_fields(fields);
    }
  }
}

//----------------------------------------------------------------------
// member function: get_memories
//
//! Get the memories
//!
//! Get the memories instantiated in this block.
//! If \p hier is true, recursively includes the memories
//! in the sub-blocks.
//!
//! Note that memories may be located in different and/or multiple
//! address maps. To get the memories in a specific address map,
//! use the uvm_reg_map::get_memories() method.
//----------------------------------------------------------------------

void uvm_reg_block::get_memories( std::vector<uvm_mem*>& mems,
                                  uvm_hier_e hier ) const
{
  for( m_mems_citt it = m_mems.begin(); it != m_mems.end(); it++ )
  {
    uvm_mem* mem = (*it).first;
    mems.push_back(mem);
  }

  if (hier == UVM_HIER)
  {
    for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
    {
      uvm_reg_block* blk = (*it).first;
      blk->get_memories(mems);
    }
  }
}

//----------------------------------------------------------------------
// member function: get_virtual_registers
//
//! Get the virtual registers
//!
//! Get the virtual registers instantiated in this block.
//! If \p hier is true, recursively includes the virtual registers
//! in the sub-blocks.
//----------------------------------------------------------------------

void uvm_reg_block::get_virtual_registers( std::vector<uvm_vreg*>& regs,
                                           uvm_hier_e hier ) const
{
  for( m_vregs_citt it = m_vregs.begin(); it != m_vregs.end(); it++ )
    regs.push_back( (*it).first );

  if (hier == UVM_HIER)
  {
    for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
    {
      uvm_reg_block* blk = (*it).first;
      blk->get_virtual_registers(regs);
    }
  }
}

//----------------------------------------------------------------------
// member function: get_virtual_fields
//
//! Get the virtual fields
//!
//! Get the virtual fields from the virtual registers instantiated
//! in this block.
//! If \p hier is true, recursively includes the virtual fields
//! in the virtual registers in the sub-blocks.
//----------------------------------------------------------------------

void uvm_reg_block::get_virtual_fields( std::vector<uvm_vreg_field*>& fields,
                                        uvm_hier_e hier ) const
{
  for( m_vregs_citt it = m_vregs.begin(); it != m_vregs.end(); it++ )
  {
    uvm_vreg* vreg = (*it).first;
    vreg->get_fields(fields);
  }

  if (hier == UVM_HIER)
  {
    for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
    {
      uvm_reg_block* blk = (*it).first;
      blk->get_virtual_fields(fields);
    }
  }
}

//----------------------------------------------------------------------
// member function: get_block_by_name
//
//! Finds a sub-block with the specified simple name.
//!
//! The name is the simple name of the block, not a hierarchical name.
//! relative to this block.
//! If no block with that name is found in this block, the sub-blocks
//! are searched for a block of that name and the first one to be found
//! is returned.
//!
//! If no blocks are found, returns NULL.
//----------------------------------------------------------------------

uvm_reg_block* uvm_reg_block::get_block_by_name( const std::string& name ) const
{
  if (get_name() == name)
    return const_cast<uvm_reg_block*>(this); //TODO return const instead

  for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    uvm_reg_block* blk = (*it).first;

    if (blk->get_name() == name)
      return blk;
  }

  for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    std::vector<uvm_reg_block*> subblks;
    (*it).first->get_blocks(subblks, UVM_HIER);

    for( unsigned int j = 0; j < subblks.size(); j++ )
      if (subblks[j]->get_name() == name)
        return subblks[j];
  }

  UVM_WARNING("RegModel",
    "Unable to locate block '" + name + "' in block '" + get_full_name() + "'");
  return NULL;
}

//----------------------------------------------------------------------
// member function: get_map_by_name
//
//! Finds an address map with the specified simple name.
//!
//! The name is the simple name of the address map, not a hierarchical name.
//! relative to this block.
//! If no map with that name is found in this block, the sub-blocks
//! are searched for a map of that name and the first one to be found
//! is returned.
//!
//! If no address maps are found, returns NULL.
//----------------------------------------------------------------------

uvm_reg_map* uvm_reg_block::get_map_by_name( const std::string& name ) const
{
  std::vector<uvm_reg_map*> maps;

  this->get_maps(maps);

  for( unsigned int i = 0; i < maps.size(); i++ )
    if (maps[i]->get_name() == name)
      return maps[i];

  for( unsigned int i = 0; i < maps.size(); i++ )
  {
    std::vector<uvm_reg_map*> submaps;

    maps[i]->get_submaps(submaps, UVM_HIER);

    for( unsigned int j = 0; j < submaps.size(); j++ )
    if ( submaps[j]->get_name() == name)
      return submaps[j];
  }

  UVM_WARNING("RegModel", "Map with name '" + name + "' does not exist in block");
  return NULL;
}

//----------------------------------------------------------------------
// member function: get_reg_by_name
//
//! Finds a register with the specified simple name.
//!
//! The name is the simple name of the register, not a hierarchical name.
//! relative to this block.
//! If no register with that name is found in this block, the sub-blocks
//! are searched for a register of that name and the first one to be found
//! is returned.
//!
//! If no registers are found, returns NULL.
//----------------------------------------------------------------------

uvm_reg* uvm_reg_block::get_reg_by_name( const std::string& name ) const
{
  for( m_regs_citt it = m_regs.begin(); it != m_regs.end(); it++ )
  {
    uvm_reg* rg = (*it).first;
    if (rg->get_name() == name)
      return rg;
  }

  for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    std::vector<uvm_reg*> subregs;
    (*it).first->get_registers(subregs, UVM_HIER);

    for( unsigned int j = 0; j < subregs.size(); j++ )
      if ( subregs[j]->get_name() == name)
        return subregs[j];
  }

  UVM_WARNING("RegModel",
    "Unable to locate register '" + name + "' in block '" + get_full_name() + "'" );
  return NULL;
}

//----------------------------------------------------------------------
// member function: get_field_by_name
//
//! Finds a field with the specified simple name.
//!
//! The name is the simple name of the field, not a hierarchical name.
//! relative to this block.
//! If no field with that name is found in this block, the sub-blocks
//! are searched for a field of that name and the first one to be found
//! is returned.
//!
//! If no fields are found, returns NULL.
//----------------------------------------------------------------------

uvm_reg_field* uvm_reg_block::get_field_by_name( const std::string& name ) const
{
  for( m_regs_citt it = m_regs.begin(); it != m_regs.end(); it++ )
  {
    uvm_reg* rg = (*it).first;
    std::vector<uvm_reg_field*> fields;

    rg->get_fields(fields);

    for ( unsigned int i = 0; i < fields.size(); i++ )
    if ( fields[i]->get_name() == name)
      return fields[i];
  }

  for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    std::vector<uvm_reg*> subregs;
    (*it).first->get_registers(subregs, UVM_HIER);

    for( unsigned int j = 0; j < subregs.size(); j++ )
    {
      std::vector<uvm_reg_field*> fields;

      subregs[j]->get_fields(fields);

      for ( unsigned int i = 0; i < fields.size(); i++ )
        if (fields[i]->get_name() == name)
          return fields[i];
    }
  }

  UVM_WARNING("RegModel", "Unable to locate field '" + name +
    "' in block '" + get_full_name() + "'");

  return NULL;
}

//----------------------------------------------------------------------
// member function: get_mem_by_name
//
//! Finds a memory with the specified simple name.
//!
//! The name is the simple name of the memory, not a hierarchical name.
//! relative to this block.
//! If no memory with that name is found in this block, the sub-blocks
//! are searched for a memory of that name and the first one to be found
//! is returned.
//!
//! If no memories are found, returns NULL.
//----------------------------------------------------------------------

uvm_mem* uvm_reg_block::get_mem_by_name( const std::string& name ) const
{
  for( m_mems_citt it = m_mems.begin(); it != m_mems.end(); it++ )
  {
    uvm_mem* mem = (*it).first;
    if (mem->get_name() == name)
      return mem;
  }

  for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    std::vector<uvm_mem*> submems;
    (*it).first->get_memories(submems, UVM_HIER);

    for( unsigned int j = 0; j < submems.size(); j++ )
      if ( submems[j]->get_name() == name)
        return submems[j];
  }

  UVM_WARNING("RegModel", "Unable to locate memory '" + name +
      "' in block '" + get_full_name() + "'");
  return NULL;
}

//----------------------------------------------------------------------
// member function: get_vreg_by_name
//
//! Finds a virtual register with the specified simple name.
//!
//! The name is the simple name of the virtual register,
//! not a hierarchical name.
//! relative to this block.
//! If no virtual register with that name is found in this block,
//! the sub-blocks are searched for a virtual register of that name
//! and the first one to be found is returned.
//!
//! If no virtual registers are found, returns NULL.
//----------------------------------------------------------------------

uvm_vreg* uvm_reg_block::get_vreg_by_name( const std::string& name ) const
{
  for( m_vregs_citt it = m_vregs.begin(); it != m_vregs.end(); it++ )
  {
    uvm_vreg* rg = (*it).first;
    if (rg->get_name() == name)
      return rg;
  }

  for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    std::vector<uvm_vreg*> subvregs;
    (*it).first->get_virtual_registers(subvregs, UVM_HIER);

    for( unsigned int j = 0; j < subvregs.size(); j++ )
      if ( subvregs[j]->get_name() == name)
        return subvregs[j];
  }

  UVM_WARNING("RegModel",
      "Unable to locate virtual register '" + name + "' in block '" + get_full_name() + "'");

  return NULL;
}

//----------------------------------------------------------------------
// member function: get_vfield_by_name
//
//! Finds a virtual field with the specified simple name.
//!
//! The name is the simple name of the virtual field,
//! not a hierarchical name.
//! relative to this block.
//! If no virtual field with that name is found in this block,
//! the sub-blocks are searched for a virtual field of that name
//! and the first one to be found is returned.
//!
//! If no virtual fields are found, returns NULL.
//----------------------------------------------------------------------

uvm_vreg_field* uvm_reg_block::get_vfield_by_name( const std::string& name ) const
{
  for( m_vregs_citt it = m_vregs.begin(); it != m_vregs.end(); it++ )
  {
    uvm_vreg* rg = (*it).first;
    std::vector<uvm_vreg_field*> fields;

    rg->get_fields(fields);

    for ( unsigned int i = 0; i < fields.size(); i++ )
      if ( fields[i]->get_name() == name)
        return fields[i];
  }

  for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    std::vector<uvm_vreg*> subvregs;
    (*it).first->get_virtual_registers(subvregs, UVM_HIER);

    for( unsigned int j = 0; j < subvregs.size(); j++ )
    {
      std::vector<uvm_vreg_field*> fields;
      subvregs[j]->get_fields(fields);

      for ( unsigned int i = 0; i < fields.size(); i++ )
        if ( fields[i]->get_name() == name)
          return fields[i];
    }
  }

  UVM_WARNING("RegModel",
   "Unable to locate virtual field '" + name + "' in block '" +
   get_full_name() + "'");

  return NULL;
}

//----------------------------------------------------------------------
// Group: Coverage
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// member function: build_coverage
//
//! Check if all of the specified coverage model must be built.
//!
//! Check which of the specified coverage model must be built
//! in this instance of the block abstraction class,
//! as specified by calls to uvm_reg::include_coverage().
//!
//! Models are specified by adding the symbolic value of individual
//! coverage model as defined in #uvm_coverage_model_e.
//! Returns the sum of all coverage models to be built in the
//! block model.
//----------------------------------------------------------------------

uvm_reg_cvr_t uvm_reg_block::build_coverage( uvm_reg_cvr_t models )
{
  uvm_reg_cvr_t coverage = UVM_NO_COVERAGE;

  uvm_reg_cvr_rsrc_db::read_by_name( "uvm_reg::" + get_full_name(),
  "include_coverage", coverage, this);

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

void uvm_reg_block::add_coverage( uvm_reg_cvr_t models )
{
  this->m_has_cover |= models;
}

//----------------------------------------------------------------------
// member function: has_coverage
//
//! Check if block has coverage model(s)
//!
//! Returns TRUE if the block abstraction class contains a coverage model
//! for all of the models specified.
//! Models are specified by adding the symbolic value of individual
//! coverage model as defined in #uvm_coverage_model_e.
//----------------------------------------------------------------------

bool uvm_reg_block::has_coverage(uvm_reg_cvr_t models) const
{
   return ((this->m_has_cover & models) == models);
}

//----------------------------------------------------------------------
// member function: set_coverage
//
//! Turns on coverage measurement.
//!
//! Turns the collection of functional coverage measurements on or off
//! for this block and all blocks, registers, fields and memories within it.
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
//! coverage models that are present in the various abstraction classes,
//! then enabled during construction.
//! See the uvm_reg_block::has_coverage() method to identify
//! the available functional coverage models.
//----------------------------------------------------------------------

uvm_reg_cvr_t uvm_reg_block::set_coverage( uvm_reg_cvr_t is_on )
{
  this->m_cover_on = this->m_has_cover & is_on;

  for( m_regs_itt it = m_regs.begin(); it != m_regs.end(); it++ )
  {
    uvm_reg* rg = (*it).first;
    rg->set_coverage(is_on);
  }

  for( m_mems_itt it = m_mems.begin(); it != m_mems.end(); it++ )
  {
    uvm_mem* mem = (*it).first;
    mem->set_coverage(is_on);
  }

  for( m_blks_itt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    uvm_reg_block* blk = (*it).first;
    blk->set_coverage(is_on);
  }

  return this->m_cover_on;
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
//! See uvm_reg_block::set_coverage() for more details.
//----------------------------------------------------------------------

bool uvm_reg_block::get_coverage( uvm_reg_cvr_t is_on ) const
{
  if (this->has_coverage(is_on) == false)
    return false;

  return ((this->m_cover_on & is_on) == is_on);
}

//----------------------------------------------------------------------
// member function: sample
//
//! Functional coverage measurement method
//!
//! This method is invoked by the block abstraction class
//! whenever an address within one of its address map
//! is successfully read or written.
//! The specified offset is the offset within the block,
//! not an absolute address.
//!
//! Empty by default, this method may be extended by the
//! abstraction class generator to perform the required sampling
//! in any provided functional coverage model.
//----------------------------------------------------------------------

void uvm_reg_block::sample( uvm_reg_addr_t offset,
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
//! block-level functional coverage model.
//! It recursively invokes the uvm_reg_block::sample_values()
//! and uvm_reg::sample_values() methods
//! in the blocks and registers in this block.
//!
//! This method may be extended by the
//! abstraction class generator to perform the required sampling
//! in any provided field-value functional coverage model.
//! If this method is extended, it must call super.sample_values().
//----------------------------------------------------------------------

void uvm_reg_block::sample_values()
{
  for( m_regs_itt it = m_regs.begin(); it != m_regs.end(); it++ )
  {
    uvm_reg* rg = (*it).first;
    rg->sample_values();
  }

  for( m_blks_itt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    uvm_reg_block* blk = (*it).first;
    blk->sample_values();
  }
}

//----------------------------------------------------------------------
// Group: Access
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: get_default_path
//
//! Default access path
//!
//! Returns the default access path for this block.
//----------------------------------------------------------------------

uvm_path_e uvm_reg_block::get_default_path() const
{
  if (default_path != UVM_DEFAULT_PATH)
    return default_path;

  if (m_parent != NULL)
    return m_parent->get_default_path();

  return UVM_FRONTDOOR;
}

//----------------------------------------------------------------------
// member function: reset
//
//! Reset the mirror for this block.
//!
//! Sets the mirror value of all registers in the block and sub-blocks
//! to the reset value corresponding to the specified reset event.
//! See uvm_reg_field::reset() for more details.
//! Does not actually set the value of the registers in the design,
//! only the values mirrored in their corresponding mirror.
//----------------------------------------------------------------------

void uvm_reg_block::reset( const std::string& kind )
{
  for( m_regs_itt it = m_regs.begin(); it != m_regs.end(); it++ )
  {
    uvm_reg* rg = (*it).first;
    rg->reset(kind);
  }

  for( m_blks_itt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    uvm_reg_block* blk = (*it).first;
    blk->reset(kind);
  }
}

//----------------------------------------------------------------------
// member function: needs_update
//
//! Check if DUT registers need to be written
//!
//! If a mirror value has been modified in the abstraction model
//! without actually updating the actual register
//! (either through randomization or via the uvm_reg::set() method,
//! the mirror and state of the registers are outdated.
//! The corresponding registers in the DUT need to be updated.
//!
//! This method returns true if the state of at lest one register in
//! the block or sub-blocks needs to be updated to match the mirrored
//! values.
//! The mirror values, or actual content of registers, are not modified.
//! For additional information, see uvm_reg_block::update() method.
//----------------------------------------------------------------------

bool uvm_reg_block::needs_update()
{
  for( m_regs_itt it = m_regs.begin(); it != m_regs.end(); it++ )
  {
    uvm_reg* rg = (*it).first;
    if (rg->needs_update())
      return true;
  }

  for( m_blks_itt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    uvm_reg_block* blk = (*it).first;
    if (blk->needs_update())
      return true;
  }

  return false;
}

//----------------------------------------------------------------------
// member function: update
//
//! Batch update of register.
//!
//! Using the minimum number of write operations, updates the registers
//! in the design to match the mirrored values in this block and sub-blocks.
//! The update can be performed using the physical
//! interfaces (front-door access) or back-door accesses.
//! This method performs the reverse operation of uvm_reg_block::mirror().
//----------------------------------------------------------------------

void uvm_reg_block::update( uvm_status_e status,
                            uvm_path_e path,
                            uvm_sequence_base* parent,
                            int prior,
                            uvm_object* extension,
                            const std::string& fname,
                            int lineno )
{
  status = UVM_IS_OK;

  if (!needs_update())
  {
    std::ostringstream str;
    str << fname
        << ":"
        << lineno
        << "RegModel block "
        << get_name()
        << "does not need updating";

    UVM_INFO("RegModel", str.str(), UVM_HIGH);
    return;
  }

  std::ostringstream inf;
  inf << fname
      << ":"
      << lineno
      << " - Updating model block "
      << get_name()
      << " with "
      << uvm_path_name[path];
  UVM_INFO("RegModel", inf.str(), UVM_HIGH);

  for( m_regs_itt it = m_regs.begin(); it != m_regs.end(); it++ )
  {
    uvm_reg* rg = (*it).first;
    if (rg->needs_update())
    {
      rg->update(status, path, NULL, parent, prior, extension);

      if (status != UVM_IS_OK && status != UVM_HAS_X)
      {
        UVM_ERROR("RegModel", "Register '" +
            rg->get_full_name() + "' could not be updated");
        return;
      }
    }
  }

  for( m_blks_itt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    uvm_reg_block* blk = (*it).first;
    blk->update( status, path, parent, prior, extension, fname, lineno);
  }
}

//----------------------------------------------------------------------
// member function: mirror
//
//! Update the mirrored values
//!
//! Read all of the registers in this block and sub-blocks and update their
//! mirror values to match their corresponding values in the design.
//! The mirroring can be performed using the physical interfaces
//! (front-door access) or back-door accesses.
//! If the \p check argument is specified as UVM_CHECK,
//! an error message is issued if the current mirrored value
//! does not match the actual value in the design.
//! This method performs the reverse operation of uvm_reg_block::update().
//----------------------------------------------------------------------

void uvm_reg_block::mirror( uvm_status_e status,
                            uvm_check_e check,
                            uvm_path_e path,
                            uvm_sequence_base* parent,
                            int prior,
                            uvm_object* extension,
                            const std::string& fname,
                            int lineno )
{
  for( m_regs_itt it = m_regs.begin(); it != m_regs.end(); it++ )
  {
    uvm_reg* rg = (*it).first;
    rg->mirror( status, check, path, NULL,
                parent, prior, extension, fname, lineno );
  }

  for( m_blks_itt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    uvm_reg_block* blk = (*it).first;

    blk->mirror( status, check, path, parent, prior, extension, fname, lineno);
  }
}

//----------------------------------------------------------------------
// member function: write_reg_by_name
//
//! Write the named register
//!
//! Equivalent to get_reg_by_name() followed by uvm_reg::write()
//----------------------------------------------------------------------

void uvm_reg_block::write_reg_by_name( uvm_status_e status,
                                       const std::string& name,
                                       uvm_reg_data_t data,
                                       uvm_path_e path,
                                       uvm_reg_map* map,
                                       uvm_sequence_base* parent,
                                       int prior,
                                       uvm_object* extension,
                                       const std::string& fname,
                                       int lineno )
{
  uvm_reg* rg;
  m_fname = fname;
  m_lineno = lineno;

  status = UVM_NOT_OK;
  rg = this->get_reg_by_name(name);
  if (rg != NULL)
    rg->write(status, data, path, map, parent, prior, extension);
}

//----------------------------------------------------------------------
// member function: read_reg_by_name
//
//! Read the named register
//!
//! Equivalent to get_reg_by_name() followed by uvm_reg::read()
//----------------------------------------------------------------------

void uvm_reg_block::read_reg_by_name( uvm_status_e status,
                                      const std::string& name,
                                      uvm_reg_data_t data,
                                      uvm_path_e path,
                                      uvm_reg_map* map,
                                      uvm_sequence_base* parent,
                                      int prior,
                                      uvm_object* extension,
                                      const std::string& fname,
                                      int lineno )
{
  uvm_reg* rg;
  m_fname = fname;
  m_lineno = lineno;

  status = UVM_NOT_OK;
  rg = get_reg_by_name(name);
  if (rg != NULL)
    rg->read(status, data, path, map, parent, prior, extension);
}

//----------------------------------------------------------------------
// member function: write_mem_by_name
//
//! Write the named memory
//!
//! Equivalent to get_mem_by_name() followed by uvm_mem::write()
//----------------------------------------------------------------------

void uvm_reg_block::write_mem_by_name( uvm_status_e status,
                                       const std::string& name,
                                       uvm_reg_addr_t offset,
                                       uvm_reg_data_t data,
                                       uvm_path_e path,
                                       uvm_reg_map* map,
                                       uvm_sequence_base* parent,
                                       int prior,
                                       uvm_object* extension,
                                       const std::string& fname,
                                       int lineno )
{
  uvm_mem* mem;
  m_fname = fname;
  m_lineno = lineno;

  status = UVM_NOT_OK;
  mem = get_mem_by_name(name);
  if (mem != NULL)
    mem->write(status, offset, data, path, map, parent, prior, extension);
}

//----------------------------------------------------------------------
// member function: read_mem_by_name
//
//! Read the named memory
//!
//! Equivalent to get_mem_by_name() followed by uvm_mem::read()
//----------------------------------------------------------------------

void uvm_reg_block::read_mem_by_name( uvm_status_e status,
                                      const std::string& name,
                                      uvm_reg_addr_t offset,
                                      uvm_reg_data_t data,
                                      uvm_path_e path,
                                      uvm_reg_map* map,
                                      uvm_sequence_base* parent,
                                      int prior,
                                      uvm_object* extension,
                                      const std::string& fname,
                                      int lineno )
{
  uvm_mem* mem;
  m_fname = fname;
  m_lineno = lineno;

  status = UVM_NOT_OK;
  mem = get_mem_by_name(name);
  if (mem != NULL)
    mem->read(status, offset, data, path, map, parent, prior, extension);
}

//----------------------------------------------------------------------
// Group: Backdoor
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: get_backdoor
//
//! Get the user-defined backdoor for all registers in this block
//!
//! Return the user-defined backdoor for all register in this
//! block and all sub-blocks -- unless overriden by a backdoor set
//! in a lower-level block or in the register itself.
//!
//! If \p inherited is true, returns the backdoor of the parent block
//! if none have been specified for this block.
//----------------------------------------------------------------------

uvm_reg_backdoor* uvm_reg_block::get_backdoor( bool inherited ) const 
{
  if (m_backdoor == NULL && inherited)
  {
    uvm_reg_block* blk = get_parent();

    while (blk != NULL)
    {
      uvm_reg_backdoor* bkdr = blk->get_backdoor();
      if (bkdr != NULL)
        return bkdr;
      blk = blk->get_parent();
    }
  }
  return this->m_backdoor;
}

//----------------------------------------------------------------------
// member function: set_backdoor
//
//! Set the user-defined backdoor for all registers in this block
//!
//! Defines the backdoor mechanism for all registers instantiated
//! in this block and sub-blocks, unless overriden by a definition
//! in a lower-level block or register.
//----------------------------------------------------------------------

void uvm_reg_block::set_backdoor( uvm_reg_backdoor* bkdr,
                                  const std::string& fname,
                                  int lineno )
{
  bkdr->m_fname = fname;
  bkdr->m_lineno = lineno;

  if( m_backdoor != NULL &&
      m_backdoor->has_update_threads() )
  {
    UVM_WARNING("RegModel", "Previous register backdoor still has update threads running. Backdoors with active mirroring should only be set before simulation starts.");
  }

  m_backdoor = bkdr;
}

//----------------------------------------------------------------------
// member function:  clear_hdl_path
//
//! Delete HDL paths
//!
//! Remove any previously specified HDL path to the block instance
//! for the specified design abstraction.
//----------------------------------------------------------------------

void uvm_reg_block::clear_hdl_path( std::string kind )
{
  if (kind == "ALL")
  {
    m_hdl_paths_pool = new uvm_object_string_pool<uvm_queue<std::string>* >("hdl_paths");
    return;
  }

  if (kind.empty())
    kind = get_default_hdl_path();

  if (!m_hdl_paths_pool->exists(kind))
  {
    UVM_WARNING("RegModel", "Unknown HDL Abstraction '" + kind + "'");
    return;
  }

  m_hdl_paths_pool->do_delete(kind);
}

//----------------------------------------------------------------------
// member function:  add_hdl_path
//
//! Add an HDL path
//!
//! Add the specified HDL path to the block instance for the specified
//! design abstraction. This method may be called more than once for the
//! same design abstraction if the block is physically duplicated
//! in the design abstraction
//----------------------------------------------------------------------

void uvm_reg_block::add_hdl_path( const std::string& path, const std::string& kind ) 
{
  uvm_queue<std::string>* paths;

     paths = m_hdl_paths_pool->get(kind);
     paths->push_back(path);
}

//----------------------------------------------------------------------
// member function:   has_hdl_path
//
//! Check if a HDL path is specified
//!
//! Returns TRUE if the block instance has a HDL path defined for the
//! specified design abstraction. If no design abstraction is specified,
//! uses the default design abstraction specified for this block or
//! the nearest block ancestor with a specified default design abstraction.
//----------------------------------------------------------------------

bool uvm_reg_block::has_hdl_path( std::string kind ) const
{

  if (kind.empty())
    kind = get_default_hdl_path();

  return m_hdl_paths_pool->exists(kind);
}

//----------------------------------------------------------------------
// member function:  get_hdl_path
//
//! Get the incremental HDL path(s)
//!
//! Returns the HDL path(s) defined for the specified design abstraction
//! in the block instance.
//! Returns only the component of the HDL paths that corresponds to
//! the block, not a full hierarchical path
//!
//! If no design abstraction is specified, the default design abstraction
//! for this block is used.
//----------------------------------------------------------------------

void uvm_reg_block::get_hdl_path( std::vector<std::string>& paths, std::string& kind )const 
{
  uvm_queue<std::string>* hdl_paths;

  if (kind.empty())
    kind = get_default_hdl_path();

  if (!has_hdl_path(kind))
  {
    UVM_ERROR("RegModel",
      "Block does not have hdl path defined for abstraction '" + kind + "'");
    return;
  }

  hdl_paths = m_hdl_paths_pool->get(kind);

  for (int i = 0; i < hdl_paths->size(); i++)
    paths.push_back(hdl_paths->get(i));
}

//----------------------------------------------------------------------
// member function:  get_full_hdl_path
//
//! Get the full hierarchical HDL path(s)
//!
//! Returns the full hierarchical HDL path(s) defined for the specified
//! design abstraction in the block instance.
//! There may be more than one path returned even
//! if only one path was defined for the block instance, if any of the
//! parent components have more than one path defined for the same design
//! abstraction
//!
//! If no design abstraction is specified, the default design abstraction
//! for each ancestor block is used to get each incremental path.
//----------------------------------------------------------------------

void uvm_reg_block::get_full_hdl_path( std::vector<std::string>& paths,
                                       std::string kind,
                                       const std::string& separator ) const
{
  if (kind.empty())
    kind = get_default_hdl_path();

  paths.clear(); // delete all

  if (is_hdl_path_root(kind))
  {
    std::string path = m_root_hdl_paths.find(kind)->second;
    if (!path.empty())
      paths.push_back(path);
    return;
  }

  if (!has_hdl_path(kind))
  {
    UVM_ERROR("RegModel","Block does not have hdl path defined for abstraction '" + kind + "'");
    return;
  }
  /*TODO:unsave code - uvm_queue needs a pointer type for <T>, otherwise convert2string can't be called in umv_queue.h(400+409)
  uvm_queue<std::string>* hdl_paths = m_hdl_paths_pool->get(kind);
  std::vector<std::string> parent_paths;

  if (m_parent != NULL)
    m_parent->get_full_hdl_path(parent_paths, kind, separator);

  for ( int i = 0; i < hdl_paths->size(); i++ )
  {
    std::string hdl_path = hdl_paths->get(i);

    if (parent_paths.size() == 0)
    {
      if (!hdl_path.empty())
        paths.push_back(hdl_path);
      continue;
    }

    for( unsigned int j = 0; j < parent_paths.size(); j++ )
    {
      if (hdl_path.empty())
        paths.push_back(parent_paths[j]);
      else
        paths.push_back( parent_paths[j] + separator + hdl_path );
    }
  }*/
}

//----------------------------------------------------------------------
// member function: set_default_hdl_path
//
//! Set the default design abstraction
//!
//! Set the default design abstraction for this block instance.
//----------------------------------------------------------------------

void uvm_reg_block::set_default_hdl_path( std::string kind )
{
  if (kind.empty())
  {
    if (m_parent == NULL)
    {
      UVM_ERROR("RegModel",
        "Block has no parent. Must specify a valid HDL abstraction (kind)");
    }
    kind = m_parent->get_default_hdl_path();
  }

  m_default_hdl_path = kind;
}

//----------------------------------------------------------------------
// member function:  get_default_hdl_path
//
//! Get the default design abstraction
//!
//! Returns the default design abstraction for this block instance.
//! If a default design abstraction has not been explicitly set for this
//! block instance, returns the default design abstraction for the
//! nearest block ancestor.
//! Returns an empty string if no default design abstraction has been specified.
//----------------------------------------------------------------------

std::string uvm_reg_block::get_default_hdl_path() const
{
  if (m_default_hdl_path.empty() && m_parent != NULL)
    return m_parent->get_default_hdl_path();
  return m_default_hdl_path;
}

//----------------------------------------------------------------------
// member function: set_hdl_path_root
//
//! Specify a root HDL path
//!
//! Set the specified path as the absolute HDL path to the block instance
//! for the specified design abstraction.
//! This absolute root path is pre-appended to all hierarchical paths
//! under this block. The HDL path of any ancestor block is ignored.
//! This method overrides any incremental path for the
//! same design abstraction specified using #add_hdl_path.
//----------------------------------------------------------------------

void uvm_reg_block::set_hdl_path_root( const std::string& path, std::string kind )
{
  if (kind.empty())
    kind = get_default_hdl_path();

  m_root_hdl_paths[kind] = path;
}

//----------------------------------------------------------------------
// member function: is_hdl_path_root
//
//! Check if this block has an absolute path
//!
//! Returns TRUE if an absolute HDL path to the block instance
//! for the specified design abstraction has been defined.
//! If no design abstraction is specified, the default design abstraction
//! for this block is used.
//----------------------------------------------------------------------

bool uvm_reg_block::is_hdl_path_root( std::string kind ) const
{
  if (kind.empty())
    kind = get_default_hdl_path();

  return (m_root_hdl_paths.find(kind) != m_root_hdl_paths.end()) ; // exists
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
// Clean-up allocated memory
//----------------------------------------------------------------------

uvm_reg_block::~uvm_reg_block()
{
  if (m_hdl_paths_pool)
    delete m_hdl_paths_pool;
}

//----------------------------------------------------------------------
// member function: m_sample
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_block::m_sample( uvm_reg_addr_t addr,
                              bool is_read,
                              uvm_reg_map* map )
{
  sample(addr, is_read, map);
  if (m_parent != NULL)
  {
    // UVM-SV
    // ToDo: Call m_sample in the parent block
    //       with the offset and map within that block's context
  }
}

//----------------------------------------------------------------------
// member function: get_parent
//
// Set the parent block
// Implementation-defined??
//----------------------------------------------------------------------

void uvm_reg_block::set_parent( uvm_reg_block* parent )
{
  if (this != parent)
    m_parent = parent;
}

//----------------------------------------------------------------------
// member function: add_block
//
// Implementation-defined
//----------------------------------------------------------------------

void uvm_reg_block::add_block( uvm_reg_block* blk )
{
  if (this->is_locked())
  {
    UVM_ERROR("RegModel", "Cannot add subblock to locked block model");
    return;
  }
  if (m_blks.find(blk) != m_blks.end()) // exists
  {
    UVM_ERROR("RegModel", "Subblock '" + blk->get_name() +
      "' has already been registered with block '" + get_name() +"'");
    return;
  }
  m_blks[blk] = id++;
  if( m_roots.find(blk) != m_roots.end()) // exists
    m_roots.erase(blk);
}

//----------------------------------------------------------------------
// member function: add_reg
//
// Implementation-defined
//----------------------------------------------------------------------

void uvm_reg_block::add_reg( uvm_reg* rg )
{
  if (this->is_locked())
  {
    UVM_ERROR("RegModel", "Cannot add register to locked block model");
    return;
  }

  if (m_regs.find(rg) != m_regs.end() ) // exists
  {
    UVM_ERROR("RegModel",
      "Register '" + rg->get_name() + "' has already been registered with block '"
      + get_name() + "'" );
    return;
  }

  m_regs[rg] = id++;
}

//----------------------------------------------------------------------
// member function: add_vreg
//
// Implementation-defined
//----------------------------------------------------------------------

void uvm_reg_block::add_vreg( uvm_vreg* vreg )
{
  if (this->is_locked())
  {
    UVM_ERROR("RegModel", "Cannot add virtual register to locked block model");
    return;
  }

  if (m_vregs.find(vreg) != m_vregs.end() ) //exists
  {
    UVM_ERROR("RegModel", "Virtual register '" + vreg->get_name() +
     "' has already been registered with block '" + get_name() + "'");
      return;
  }
  m_vregs[vreg] = id++;
}

//----------------------------------------------------------------------
// member function: add_mem
//
// Implementation-defined
//----------------------------------------------------------------------

void uvm_reg_block::add_mem( uvm_mem* mem )
{
  if (this->is_locked())
  {
    UVM_ERROR("RegModel", "Cannot add memory to locked block model");
    return;
  }

  if (m_mems.find(mem) != m_mems.end() ) //exists
  {
    UVM_ERROR("RegModel", "Memory '" + mem->get_name() +
      "' has already been registered with block '" + get_name() + "'");
    return;
  }
  m_mems[mem] = id++;
}

//----------------------------------------------------------------------
// Implementation defined: Basic Object Operations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: do_print
//----------------------------------------------------------------------

void uvm_reg_block::do_print( const uvm_printer& printer ) const
{
  uvm_object::do_print(printer);

  for( m_blks_citt it = m_blks.begin(); it != m_blks.end(); it++ )
  {
    const uvm_reg_block* b = (*it).first;
    const uvm_object* obj = b;
    printer.print_object(obj->get_name(), *obj);
  }

  for( m_regs_citt it = m_regs.begin(); it != m_regs.end(); it++ )
  {
    const uvm_reg* r = (*it).first;
    const uvm_object* obj = r;
    printer.print_object(obj->get_name(), *obj);
  }

  for( m_vregs_citt it = m_vregs.begin(); it != m_vregs.end(); it++ )
  {
    const uvm_vreg* r = (*it).first;
    const uvm_object* obj = r;
    printer.print_object(obj->get_name(), *obj);
  }

  for( m_mems_citt it = m_mems.begin(); it != m_mems.end(); it++ )
  {
    const uvm_mem* m = (*it).first;
    const uvm_object* obj = m;
    printer.print_object(obj->get_name(), *obj);
  }

  for( m_maps_citt it = m_maps.begin(); it != m_maps.end(); it++ )
  {
    const uvm_reg_map* m = (*it).first;
    const uvm_object* obj = m;
    printer.print_object(obj->get_name(), *obj);
  }
}

//----------------------------------------------------------------------
// member function: clone
//----------------------------------------------------------------------

uvm_object* uvm_reg_block::clone()
{
  UVM_FATAL("RegModel","RegModel blocks cannot be cloned");
  return NULL;
}

//----------------------------------------------------------------------
// member function: do_copy
//----------------------------------------------------------------------

void uvm_reg_block::do_copy( const uvm_object& rhs )
{
  UVM_FATAL("RegModel","RegModel blocks cannot be copied");
}


//----------------------------------------------------------------------
// member function: do_compare
//----------------------------------------------------------------------

bool uvm_reg_block::do_compare( const uvm_object& rhs,
                                const uvm_comparer* comparer ) const
{
  UVM_WARNING("RegModel","RegModel blocks cannot be compared");
  return false;
}


//----------------------------------------------------------------------
// member function: do_pack
//----------------------------------------------------------------------

void uvm_reg_block::do_pack( uvm_packer& packer ) const
{
  UVM_WARNING("RegModel","RegModel blocks cannot be packed");
}

//----------------------------------------------------------------------
// member function: do_unpack
//----------------------------------------------------------------------

void uvm_reg_block::do_unpack( uvm_packer& packer )
{
  UVM_WARNING("RegModel","RegModel blocks cannot be unpacked");
}


//----------------------------------------------------------------------
// member function: convert2string
//----------------------------------------------------------------------
// TODO implement convert2string

std::string uvm_reg_block::convert2string() const
{
  /* TODO
  std::ostringstream image;
  std::valarray<std::string> maps;
  std::valarray<std::string> blk_maps;
  bool single_map;
  uvm_endianness_e endian;
  std::string prefix = "  ";


  single_map = true;
  if (map.empty())
  {
    this->get_maps(maps);
    if( maps.size() > 1 )
      single_map = false;
  }

  std::ostringstream image;
  if (single_map)
  {
    image << prefix << "Block " << this->get_full_name();

    if (!map.empty())
      image << "." << map;

    endian = this->get_endian(map);

    image << " -- "
          << this->get_n_bytes(map)
          << " bytes ("
          << uvm_endianness_name[endian]
          << ")";

    for( blks_itt it = blks.begin(); it != blks.end(); it++ )
    {
      std::string img;
      img = (*it).first.convert2string( prefix + "   " + blk_maps[i]);

      image << std::endl << img;
    }

  }
  else
  {
    image << prefix
        << "Block "
        << this->get_full_name();

    for( maps_itt it = maps.begin(); it != maps.end(); it++ )
    {
      endian = this->get_endian( (*it).first );

      image << std::endl
            << prefix
            << "   Map '"
            << (*it).first
            << "' -- " << this->get_n_bytes( (*it).first )
            << " bytes ("
            << uvm_endianness_name[endian]
            << ")";

      this->get_blocks(blks, blk_maps, maps[i]);

      for( blks_itt it = blks.begin(); it != blks.end(); it++ )
      {
        std::string img;
        img = (*it).first.convert2string( prefix +  "      " +
            blk_maps[j]); // TODO - what is blk_maps?
        image << std::endl << img;
      }

      this->get_subsys(sys, blk_maps, maps[i]);

      foreach (sys[j])
      {
        std::string img;
        img = sys[j].convert2string({prefix, "      "},
            blk_maps[j]); // TODO - what is blk_maps?
        image << std::endl << img;
      }
    }
  }
  return image.str();
  */

  return ""; // dummy return for now
}

//----------------------------------------------------------------------
// member function: m_init_address_maps
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_block::m_init_address_maps()
{
  for( m_maps_itt it = m_maps.begin(); it != m_maps.end(); it++ )
  {
    uvm_reg_map* map = (*it).first;
    map->m_init_address_map();
  }
  // commented in UVM-SV
  //map->m_verify_map_config();
}


//////

} // namespace uvm
