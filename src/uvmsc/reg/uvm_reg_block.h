//----------------------------------------------------------------------
//   Copyright 2013-2015 NXP B.V.
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

#ifndef UVM_REG_BLOCK_H_
#define UVM_REG_BLOCK_H_

#include <map>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/reg/uvm_reg.h"
#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/reg/uvm_reg_cbs_types.h"
#include "uvmsc/conf/uvm_object_string_pool.h"

//////////////

namespace uvm {


// forward class declaration
class uvm_mem;
class uvm_reg_map;
class uvm_vreg;
class uvm_reg_field;
class uvm_vreg_field;
class uvm_reg_backdoor;

//----------------------------------------------------------------------
// Class: uvm_reg_block
//
//! Block abstraction base class
//!
//! A block represents a design hierarchy. It can contain registers,
//! register files, memories and sub-blocks.
//!
//! A block has one or more address maps, each corresponding to a physical
//! interface on the block.
//----------------------------------------------------------------------

class uvm_reg_block : public uvm_object
{
 public:
  friend class uvm_reg;
  friend class uvm_vreg;
  friend class uvm_mem;
  template <typename BUSTYPE> friend class uvm_reg_predictor;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Group: Initialization
  //--------------------------------------------------------------------------

  // Constructor
  uvm_reg_block( const std::string& name = "",
                 int has_coverage = UVM_NO_COVERAGE );

  void configure( uvm_reg_block* parent = nullptr,
		  const std::string& hdl_path = "" );

  virtual uvm_reg_map* create_map( const std::string& name,
                                   uvm_reg_addr_t base_addr,
                                   unsigned int n_bytes,
                                   uvm_endianness_e endian,
                                   bool byte_addressing = true );

  static bool check_data_width( unsigned int width ); // protected?

  void set_default_map( uvm_reg_map* map );

  uvm_reg_map* get_default_map() const ; // TODO not in UVM LRM?

  virtual void lock_model();

  bool is_locked() const;

  //--------------------------------------------------------------------------
  // Group: Introspection
  //--------------------------------------------------------------------------

  //virtual string get_name();

  const std::string get_full_name() const override;

  virtual uvm_reg_block* get_parent() const;

  static void get_root_blocks( std::vector<uvm_reg_block*>& blks );

  static int find_blocks( std::string name,
		  	  	  	      std::vector<uvm_reg_block*>& blks,
                          uvm_reg_block* root = nullptr,
                          uvm_object* accessor = nullptr );
      
  static uvm_reg_block* find_block( const std::string& name,
                                    uvm_reg_block* root = nullptr,
                                    uvm_object* accessor = nullptr );

  virtual void get_blocks( std::vector<uvm_reg_block*>& blks,
                           uvm_hier_e hier = UVM_HIER ) const;

  virtual void get_maps( std::vector<uvm_reg_map*>& maps ) const;

  virtual void get_registers( std::vector<uvm_reg*>& regs,
                              uvm_hier_e hier = UVM_HIER ) const;

  virtual void get_fields( std::vector<uvm_reg_field*>& fields,
                           uvm_hier_e hier = UVM_HIER ) const;

  // TODO get_memories not in UVM LRM?
  void get_memories( std::vector<uvm_mem*>& mems,
                     uvm_hier_e hier = UVM_HIER ) const;

  void get_virtual_registers( std::vector<uvm_vreg*>& regs,
                              uvm_hier_e hier = UVM_HIER ) const;

  void get_virtual_fields( std::vector<uvm_vreg_field*>& fields,
                           uvm_hier_e hier = UVM_HIER ) const;

  uvm_reg_block* get_block_by_name( const std::string& name ) const;

  uvm_reg_map* get_map_by_name( const std::string& name ) const;

  uvm_reg* get_reg_by_name( const std::string& name ) const;

  uvm_reg_field* get_field_by_name( const std::string& name ) const;

  uvm_mem* get_mem_by_name( const std::string& name ) const;

  uvm_vreg* get_vreg_by_name( const std::string& name ) const;

  uvm_vreg_field* get_vfield_by_name( const std::string& name ) const;

  //--------------------------------------------------------------------------
  // Group: Coverage
  //--------------------------------------------------------------------------

 protected:
  uvm_reg_cvr_t build_coverage( uvm_reg_cvr_t models );

  virtual void add_coverage( uvm_reg_cvr_t models );

 public:
  bool has_coverage( uvm_reg_cvr_t models ) const;

  uvm_reg_cvr_t set_coverage( uvm_reg_cvr_t is_on );

  bool get_coverage( uvm_reg_cvr_t is_on = UVM_CVR_ALL ) const;

 protected:
  virtual void sample( uvm_reg_addr_t offset,
                       bool is_read,
                       uvm_reg_map* map );
 public:
  void sample_values();

  //--------------------------------------------------------------------------
  // Group: Access
  //--------------------------------------------------------------------------

  uvm_path_e get_default_path() const;

  void reset( const std::string& kind = "HARD" );

  bool needs_update();

  virtual void update( uvm_status_e status,
                       uvm_path_e path = UVM_DEFAULT_PATH,
                       uvm_sequence_base* parent = nullptr,
                       int prior = -1,
                       uvm_object* extension = nullptr,
                       const std::string& fname = "",
                       int lineno = 0 );

  virtual void mirror( uvm_status_e status,
                       uvm_check_e check = UVM_NO_CHECK,
                       uvm_path_e path  = UVM_DEFAULT_PATH,
                       uvm_sequence_base* parent = nullptr,
                       int prior = -1,
                       uvm_object* extension = nullptr,
                       const std::string& fname = "",
                       int lineno = 0 );

  virtual void write_reg_by_name( uvm_status_e status,
		  	  	  	  	  	      const std::string& name,
                                  uvm_reg_data_t data,
                                  uvm_path_e path = UVM_DEFAULT_PATH,
                                  uvm_reg_map* map = nullptr,
                                  uvm_sequence_base* parent = nullptr,
                                  int prior = -1,
                                  uvm_object* extension = nullptr,
                                  const std::string& fname = "",
                                  int lineno = 0 );

  virtual void read_reg_by_name( uvm_status_e status,
		  	  	  	  	  	  	 const std::string& name,
                                 uvm_reg_data_t data,
                                 uvm_path_e path = UVM_DEFAULT_PATH,
                                 uvm_reg_map* map = nullptr,
                                 uvm_sequence_base* parent = nullptr,
                                 int prior = -1,
                                 uvm_object* extension = nullptr,
                                 const std::string& fname = "",
                                 int lineno = 0 );

  virtual void write_mem_by_name( uvm_status_e status,
		  	  	  	  	  	  	  const std::string& name,
                                  uvm_reg_addr_t offset,
                                  uvm_reg_data_t data,
                                  uvm_path_e path = UVM_DEFAULT_PATH,
                                  uvm_reg_map* map = nullptr,
                                  uvm_sequence_base* parent = nullptr,
                                  int prior = -1,
                                  uvm_object* extension = nullptr,
                                  const std::string& fname = "",
                                  int lineno = 0 );

  virtual void read_mem_by_name( uvm_status_e status,
		  	  	  	  	  	  	 const std::string& name,
                                 uvm_reg_addr_t offset,
                                 uvm_reg_data_t data,
                                 uvm_path_e path = UVM_DEFAULT_PATH,
                                 uvm_reg_map* map = nullptr,
                                 uvm_sequence_base* parent = nullptr,
                                 int prior = -1,
                                 uvm_object* extension = nullptr,
                                 const std::string& fname = "",
                                 int lineno = 0 );

  //--------------------------------------------------------------------------
  // Group: Backdoor
  //--------------------------------------------------------------------------

  uvm_reg_backdoor* get_backdoor( bool inherited = true ) const;

  void set_backdoor( uvm_reg_backdoor* bkdr,
                     const std::string& fname = "",
                     int lineno = 0);

  void clear_hdl_path( std::string kind = "RTL" );

  void add_hdl_path( const std::string& path, const std::string& kind = "RTL" );

  bool has_hdl_path( std::string kind = "" ) const;

  void get_hdl_path( std::vector<std::string>& paths, const std::string& kind = "" ) const;

  void get_full_hdl_path( std::vector<std::string>& paths,
                          std::string kind = "",
                          const std::string& separator = "." ) const;

  void set_default_hdl_path( std::string kind );

  std::string get_default_hdl_path() const;

  void set_hdl_path_root( const std::string& path, std::string kind = "RTL" );

  bool is_hdl_path_root( std::string kind = "" ) const;

  // public data members

  uvm_reg_map* default_map;

  // Variable: default_path
  // Default access path for the registers and memories in this block.
  uvm_path_e default_path; // default set to UVM_DEFAULT_PATH in constructor


  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  ~uvm_reg_block() override; // destructor

 private:

  void m_sample( uvm_reg_addr_t addr,
                 bool is_read,
                 uvm_reg_map* map);

  virtual void set_parent( uvm_reg_block* parent );

  void add_block( uvm_reg_block* blk );

  void add_map( uvm_reg_map* map );

#if defined(ALLOW_NON_IEEE_1800_2_2020)
 public:
#endif
  void add_reg( uvm_reg* rg );
#if defined(ALLOW_NON_IEEE_1800_2_2020)
 private:
#endif

  void add_vreg( uvm_vreg* vreg );

  void add_mem( uvm_mem* mem );

  void do_print( const uvm_printer& printer ) const override;

  uvm_object* clone() override;

  void do_copy( const uvm_object& rhs ) override;

  bool do_compare( const uvm_object& rhs,
                   const uvm_comparer* comparer ) const override;

  void do_pack( uvm_packer& packer ) const override;

  void do_unpack( uvm_packer& packer ) override;

  std::string convert2string() const override;

  void m_init_address_maps();

  // local data members

  uvm_reg_block* m_parent;

  typedef std::map<uvm_reg_block*, bool> m_rootsT;
  typedef m_rootsT::iterator m_roots_itt;
  typedef m_rootsT::const_iterator m_roots_citt;
  static m_rootsT m_roots;

  typedef std::map<uvm_reg_block*, unsigned int> m_blksT;
  typedef m_blksT::iterator m_blks_itt;
  typedef m_blksT::const_iterator m_blks_citt;
  m_blksT m_blks;

  typedef std::map<uvm_reg*, unsigned int> m_regsT;
  typedef m_regsT::iterator m_regs_itt;
  typedef m_regsT::const_iterator m_regs_citt;
  m_regsT m_regs;

  typedef std::map<uvm_vreg*, unsigned int> m_vregsT;
  typedef m_vregsT::iterator m_vregs_itt;
  typedef m_vregsT::const_iterator m_vregs_citt;
  m_vregsT m_vregs;

  typedef std::map<uvm_mem*, unsigned int> m_memsT;
  typedef m_memsT::iterator m_mems_itt;
  typedef m_memsT::const_iterator m_mems_citt;
  m_memsT m_mems;

  typedef std::map<uvm_reg_map*, bool> m_mapsT;
  typedef m_mapsT::iterator m_maps_itt;
  typedef m_mapsT::const_iterator m_maps_citt;
  m_mapsT m_maps;


  std::string m_default_hdl_path; // default set to "RTL" in constructor
  uvm_reg_backdoor* m_backdoor;

  //uvm_object_string_pool<uvm_queue<std::string>* >* m_hdl_paths_pool;

  std::map<std::string, std::vector<std::string> > m_hdl_paths_pool;
  typedef std::map<std::string, std::vector<std::string> > m_hdl_paths_pool_itT;

  std::map<std::string, std::string> m_root_hdl_paths;

  bool m_locked;

  int m_has_cover;
  int m_cover_on;
  std::string m_fname;
  int m_lineno;

  static int id;

}; // class uvm_reg_block

//------------------------------------------------------------------------

} // namespace uvm

#endif /* UVM_REG_BLOCK_H_ */
