//----------------------------------------------------------------------
//   Copyright 2013-2019 NXP B.V.
//   Copyright 2004-2011 Synopsys, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2010-2011 Cadence Design Systems, Inc.
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

#ifndef UVM_REG_MAP_H_
#define UVM_REG_MAP_H_

#include <systemc>
#include <vector>
#include <map>

#include "uvmsc/reg/uvm_reg.h"
#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/macros/uvm_object_defines.h"
#include "uvmsc/factory/uvm_object_registry.h"

namespace uvm {

// forward class declarations
class uvm_reg_frontdoor;
class uvm_reg_block;
class uvm_mem;
class uvm_sequencer_base;
class uvm_reg_adapter;
class uvm_vreg;
class uvm_vreg_field;

class uvm_reg_map_info
{
 public:
  uvm_reg_addr_t offset;
  uvm_reg_field_access_e default_rights;
  std::string rights;
  bool unmapped;
  std::vector<uvm_reg_addr_t> addr;
  uvm_reg_frontdoor* frontdoor;
  uvm_reg_map_addr_range mem_range;

  // if set marks the uvm_reg_map_info as initialized, prevents using an uninitialized map (for instance if the model
  // has not been locked accidently and the maps have not been computed before)
  bool is_initialized;

  // Constructor
  uvm_reg_map_info()
  {
    frontdoor = nullptr;
    default_rights = RW;
    offset = 0;
    rights = "RW";
    is_initialized = false;
  }

}; // class uvm_reg_map_info

//------------------------------------------------------------------------------
// Class: uvm_reg_map
//
//! Address map abstraction class
//!
//! This class represents an address map.
//! An address map is a collection of registers and memories
//! accessible via a specific physical interface.
//! Address maps can be composed into higher-level address maps.
//!
//! Address maps are created using the uvm_reg_block::create_map()
//! method.
//------------------------------------------------------------------------------

class uvm_reg_map : public uvm_object
{
public:

  UVM_OBJECT_UTILS(uvm_reg_map)

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // Group: Initialization
  //----------------------------------------------------------------------------

  explicit uvm_reg_map( const std::string& name = "uvm_reg_map" );

  void configure( uvm_reg_block* parent,
                  uvm_reg_addr_t base_addr,
                  unsigned int n_bytes,
                  uvm_endianness_e endian,
                  bool byte_addressing = true );

  virtual void add_reg( uvm_reg* rg,
                        uvm_reg_addr_t offset,
                        const std::string& rights = "RW",
                        bool unmapped = false,
                        uvm_reg_frontdoor* frontdoor = nullptr );

  virtual void add_mem( uvm_mem* mem,
                        uvm_reg_addr_t offset,
                        const std::string& rights = "RW",
                        bool unmapped = false,
                        uvm_reg_frontdoor* frontdoor = nullptr );

  virtual void add_submap( uvm_reg_map* child_map,
                           uvm_reg_addr_t offset );

  virtual void set_sequencer( uvm_sequencer_base* sequencer,
                              uvm_reg_adapter* adapter = nullptr );

  virtual void set_submap_offset( uvm_reg_map* submap,
                                  uvm_reg_addr_t offset);

  virtual uvm_reg_addr_t get_submap_offset( const uvm_reg_map* submap ) const;

  virtual void set_base_addr( uvm_reg_addr_t offset);

  virtual void reset( const std::string& kind = "SOFT" );

  //----------------------------------------------------------------------------
  // Group: Introspection
  //----------------------------------------------------------------------------

  // virtual string get_name() const; // inherit from base class

  const std::string get_full_name() const override;

  virtual uvm_reg_map* get_root_map() const;

  virtual uvm_reg_block* get_parent() const;

  virtual uvm_reg_map* get_parent_map() const ;

  virtual uvm_reg_addr_t get_base_addr( uvm_hier_e hier = UVM_HIER) const;

  virtual unsigned int get_n_bytes( uvm_hier_e hier = UVM_HIER ) const;

  virtual unsigned int get_addr_unit_bytes() const;

  virtual uvm_endianness_e get_endian( uvm_hier_e hier = UVM_HIER ) const;

  virtual uvm_sequencer_base* get_sequencer( uvm_hier_e hier = UVM_HIER ) const;

  virtual uvm_reg_adapter* get_adapter( uvm_hier_e hier = UVM_HIER ) const;

  virtual void get_submaps( std::vector<uvm_reg_map*>& maps,
                            uvm_hier_e hier = UVM_HIER ) const;

  virtual void get_registers( std::vector<uvm_reg*>& regs,
                              uvm_hier_e hier = UVM_HIER ) const;

  virtual void get_fields( std::vector<uvm_reg_field*>& fields,
                           uvm_hier_e hier = UVM_HIER ) const;

  virtual void get_memories( std::vector<uvm_mem*>& mems,
                             uvm_hier_e hier = UVM_HIER ) const;

  virtual void get_virtual_registers( std::vector<uvm_vreg*>& vregs,
                                      uvm_hier_e hier = UVM_HIER) const;

  virtual void get_virtual_fields( std::vector<uvm_vreg_field*>& fields,
                                   uvm_hier_e hier = UVM_HIER) const;

  virtual int get_physical_addresses( uvm_reg_addr_t base_addr,
                                      uvm_reg_addr_t mem_offset,
                                      unsigned int n_bytes,
                                      std::vector<uvm_reg_addr_t>& addr ) const;

  virtual uvm_reg* get_reg_by_offset( uvm_reg_addr_t offset,
                                      bool read = true ) const;

  virtual uvm_mem* get_mem_by_offset( uvm_reg_addr_t offset ) const;

  //----------------------------------------------------------------------------
  // Group: Bus Access
  //----------------------------------------------------------------------------

  void set_auto_predict( bool on = true );

  bool get_auto_predict() const;

  void set_check_on_read( bool on = true );

  bool get_check_on_read() const;

  virtual void do_bus_write( uvm_reg_item* rw,
                             uvm_sequencer_base* sequencer,
                             uvm_reg_adapter* adapter );

  virtual void do_bus_read( uvm_reg_item* rw,
                            uvm_sequencer_base* sequencer,
                            uvm_reg_adapter* adapter );

  virtual void do_write( uvm_reg_item* rw );

  virtual void do_read( uvm_reg_item* rw );


  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  ~uvm_reg_map() override; // destructor

  virtual void add_parent_map( uvm_reg_map* parent_map,
                               uvm_reg_addr_t offset );

  virtual void m_verify_map_config() const;

  virtual void m_set_reg_offset( uvm_reg* rg,
                                 uvm_reg_addr_t offset,
                                 bool unmapped );

  virtual void m_set_mem_offset( uvm_mem* mem,
                                 uvm_reg_addr_t offset,
                                 bool unmapped );

  virtual uvm_reg_map_info* get_reg_map_info( const uvm_reg* rg, bool error = true ) const;

  virtual uvm_reg_map_info* get_mem_map_info( const uvm_mem* mem, bool error = true ) const;

  virtual unsigned int get_size() const;

  static uvm_reg_map* backdoor();

  void m_get_bus_info( uvm_reg_item* rw,
                       uvm_reg_map_info*& map_info,
                       unsigned int& size,
                       int& lsb,
                       int& addr_skip ) const;

  void m_init_address_map();

  //----------------------------------------------------------------------------
  // Implementation defined: UVM object methods
  //----------------------------------------------------------------------------

  std::string convert2string() const override;

  uvm_object* clone() override;
  void do_print( const uvm_printer& printer ) const override;
  void do_copy( const uvm_object& rhs ) override;
  // TODO add these methods?
  // virtual bool do_compare( const uvm_object& rhs, const uvm_comparer* comparer);
  // virtual void do_pack( uvm_packer& packer );
  // virtual void do_unpack( uvm_packer& packer );

  // local data members

private:
  // info that is valid only if top-level map
   uvm_reg_addr_t      m_base_addr;
   unsigned int        m_n_bytes;
   uvm_endianness_e    m_endian;
   bool                m_byte_addressing;
   // TODO: unused
   /* uvm_object_wrapper* m_sequence_wrapper;*/
   uvm_reg_adapter*    m_adapter;
   uvm_sequencer_base* m_sequencer;
   bool                m_auto_predict;
   bool                m_check_on_read;

   uvm_reg_block*      m_parent;

   unsigned int        m_system_n_bytes;

   uvm_reg_map*        m_parent_map;

   typedef std::map<uvm_reg_map*, uvm_reg_addr_t> m_parent_maps_t; // value=offset of this map at parent level
   typedef m_parent_maps_t::iterator m_parent_maps_itt;
   m_parent_maps_t m_parent_maps;

   typedef std::map< uvm_reg_map*, uvm_reg_addr_t> m_submaps_t;     // value=offset of submap at this level
   typedef m_submaps_t::iterator m_submaps_itt;
   typedef m_submaps_t::const_iterator m_submaps_citt;
   m_submaps_t m_submaps;

   typedef std::map< uvm_reg_map*, std::string> m_submap_rights_t;       // value=rights of submap at this level
   typedef m_submap_rights_t::const_iterator m_submap_rights_citt;
   m_submap_rights_t m_submap_rights;

   typedef std::map<uvm_reg*, uvm_reg_map_info*> m_regs_info_t;
   typedef m_regs_info_t::iterator m_regs_info_itt;
   typedef m_regs_info_t::const_iterator m_regs_info_citt;
   m_regs_info_t m_regs_info;

   typedef std::map<uvm_mem*, uvm_reg_map_info*> m_mems_info_t;
   typedef m_mems_info_t::iterator m_mems_info_itt;
   typedef m_mems_info_t::const_iterator m_mems_info_citt;
   m_mems_info_t m_mems_info;

   typedef std::map<uvm_reg_addr_t, uvm_reg*> m_regs_by_offset_t;
   typedef m_regs_by_offset_t::iterator m_regs_by_offset_itt;
   m_regs_by_offset_t m_regs_by_offset;

  // Use only in addition to above if a RO and a WO
  // register share the same address.

   typedef std::map<uvm_reg_addr_t, uvm_reg*> m_regs_by_offset_wo_t;
   typedef m_regs_by_offset_wo_t::iterator m_regs_by_offset_wo_itt;
   m_regs_by_offset_wo_t m_regs_by_offset_wo;

   typedef std::map<uvm_reg_map_addr_range*, uvm_mem* > m_mems_by_offset_t;
   typedef m_mems_by_offset_t::iterator m_mems_by_offset_itt;
   typedef m_mems_by_offset_t::const_iterator m_mems_by_offset_citt;
   m_mems_by_offset_t m_mems_by_offset;

   static uvm_reg_map* m_backdoor;

}; // class uvm_reg_map

} // namespace uvm

#endif // UVM_REG_MAP_H_
