//----------------------------------------------------------------------
//   Copyright 2013-2016 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
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

#ifndef UVM_MEM_H_
#define UVM_MEM_H_

#include <map>
#include <vector>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/conf/uvm_object_string_pool.h"
#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/reg/uvm_reg_cbs_types.h"

namespace uvm {


// forward class declarations
class uvm_reg_block;
class uvm_reg_map;
class uvm_reg_map_info;
class uvm_reg_item;
class uvm_reg_backdoor;
class uvm_reg_frontdoor;
class uvm_mem_mam;
class uvm_vreg;
class uvm_vreg_field;
class uvm_sequence_base;

//----------------------------------------------------------------------
// CLASS: uvm_mem
//
//! Memory abstraction base class
//!
//! A memory is a collection of contiguous locations.
//! A memory may be accessible via more than one address map.
//!
//! Unlike registers, memories are not mirrored because of the potentially
//! large data space: tests that walk the entire memory space would negate
//! any benefit from sparse memory modelling techniques.
//! Rather than relying on a mirror, it is recommended that
//! backdoor access be used instead.
//----------------------------------------------------------------------

class uvm_mem : public uvm_object
{
 public:
  friend class uvm_reg_block;
  friend class uvm_reg_map;
  friend class uvm_vreg;

  typedef enum {UNKNOWNS, ZEROES, ONES, ADDRESS, VALUE, INCR, DECR} init_e;


  //--------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------

  //--------------------------------------------------------------------
  // Group: Initialization
  //--------------------------------------------------------------------

  explicit uvm_mem( const std::string& name,
                    unsigned long size,
                    unsigned int n_bits,
                    const std::string& access = "RW",
                    int has_coverage = UVM_NO_COVERAGE );

  void configure( uvm_reg_block* parent,
                  const std::string& hdl_path = "" );

  void set_offset( uvm_reg_map* map,
                   uvm_reg_addr_t offset,
                   bool unmapped = 0 );

  //--------------------------------------------------------------------
  // Group: Introspection
  //--------------------------------------------------------------------

  // virtual string get_name() const; // inherit from base class

  const std::string get_full_name() const override;

  virtual uvm_reg_block* get_parent() const;

  virtual int get_n_maps() const;

  bool is_in_map( uvm_reg_map* map ) const;

  virtual void get_maps( std::vector<uvm_reg_map*>& maps ) const;

  virtual std::string get_rights( const uvm_reg_map* map = nullptr ) const;

  virtual std::string get_access( const uvm_reg_map* map = nullptr ) const;

  unsigned long get_size() const;

  unsigned int get_n_bytes() const;

  unsigned int get_n_bits() const;

  static unsigned int get_max_size();

  virtual void get_virtual_registers( std::vector<uvm_vreg*>& regs ) const;

  virtual void get_virtual_fields( std::vector<uvm_vreg_field*>& fields) const;

  virtual uvm_vreg* get_vreg_by_name( const std::string& name ) const;

  virtual uvm_vreg_field* get_vfield_by_name( const std::string& name ) const;

  virtual uvm_vreg* get_vreg_by_offset( uvm_reg_addr_t offset,
                                        const uvm_reg_map* map = nullptr ) const;

  virtual uvm_reg_addr_t get_offset( uvm_reg_addr_t offset = 0,
                                     const uvm_reg_map* map = nullptr ) const;

  virtual uvm_reg_addr_t get_address( uvm_reg_addr_t offset = 0,
                                      const uvm_reg_map* map = nullptr ) const;

  // note: different argument order as in UVM-SV to respect defaults
  virtual int get_addresses( std::vector<uvm_reg_addr_t>& addr,
                             const uvm_reg_map* map = nullptr,
                             uvm_reg_addr_t offset = 0 ) const;

  //--------------------------------------------------------------------
  // Group: HDL Access
  //--------------------------------------------------------------------

  virtual void write( uvm_status_e& status, // output
                      uvm_reg_addr_t offset,
                      uvm_reg_data_t value,
                      uvm_path_e path   = UVM_DEFAULT_PATH,
                      uvm_reg_map* map = nullptr,
                      uvm_sequence_base* parent = nullptr,
                      int prior = -1,
                      uvm_object* extension = nullptr,
                      const std::string& fname = "",
                      int lineno = 0 );


  virtual void read( uvm_status_e& status, // output
                     uvm_reg_addr_t offset,
                     uvm_reg_data_t& value, // output
                     uvm_path_e path = UVM_DEFAULT_PATH,
                     uvm_reg_map* map = nullptr,
                     uvm_sequence_base* parent = nullptr,
                     int prior = -1,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void burst_write( uvm_status_e& status, // output
                            uvm_reg_addr_t offset,
                            std::vector<uvm_reg_data_t> value,
                            uvm_path_e path = UVM_DEFAULT_PATH,
                            uvm_reg_map* map = nullptr,
                            uvm_sequence_base* parent = nullptr,
                            int prior = -1,
                            uvm_object* extension = nullptr,
                            const std::string& fname = "",
                            int lineno = 0);

  virtual void burst_read( uvm_status_e& status, // output
                           uvm_reg_addr_t offset,
                           std::vector<uvm_reg_data_t>& value,
                           uvm_path_e path = UVM_DEFAULT_PATH,
                           uvm_reg_map* map = nullptr,
                           uvm_sequence_base* parent = nullptr,
                           int prior = -1,
                           uvm_object* extension = nullptr,
                           const std::string& fname = "",
                           int lineno = 0 );

  virtual void poke( uvm_status_e& status, // output
                     uvm_reg_addr_t offset,
                     uvm_reg_data_t value,
                     const std::string& kind = "",
                     uvm_sequence_base* parent = nullptr,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void peek( uvm_status_e& status,
                     uvm_reg_addr_t offset,
                     uvm_reg_data_t& value, // output
                     const std::string& kind = "",
                     uvm_sequence_base* parent = nullptr,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  //--------------------------------------------------------------------
  // Group: Frontdoor
  //--------------------------------------------------------------------

  void set_frontdoor( uvm_reg_frontdoor* ftdr,
                      uvm_reg_map* map = nullptr,
                      const std::string& fname = "",
                      int lineno = 0 );


  uvm_reg_frontdoor* get_frontdoor( const uvm_reg_map* map = nullptr ) const;


  //--------------------------------------------------------------------
  // Group: Backdoor
  //--------------------------------------------------------------------

  void set_backdoor( uvm_reg_backdoor* bkdr,
                     const std::string& fname = "",
                     int lineno = 0 );

  uvm_reg_backdoor* get_backdoor( bool inherited = true );

  void clear_hdl_path( const std::string& kind = "RTL" );

  void add_hdl_path( std::vector<uvm_hdl_path_slice> slices,
                     const std::string& kind = "RTL" );

  void add_hdl_path_slice( const std::string& name,
                           int offset,
                           int size,
                           bool first = false,
                           const std::string& kind = "RTL" );

  bool has_hdl_path( const std::string& kind = "" ) const;

  void get_hdl_path( std::vector<uvm_hdl_path_concat>& paths,
                     const std::string& kind = "" ) const;

  void get_full_hdl_path( std::vector<uvm_hdl_path_concat>& paths,
                          const std::string& kind = "",
                          const std::string& separator = "." ) const;

  void get_hdl_path_kinds( std::vector<std::string>& kinds ) const;

 protected:
  virtual void backdoor_read( uvm_reg_item* rw );

 public: // public from here on...
  virtual void backdoor_write( uvm_reg_item* rw );

  virtual uvm_status_e backdoor_read_func( uvm_reg_item* rw );

  //--------------------------------------------------------------------
  // Group: Callbacks
  //--------------------------------------------------------------------

  virtual void pre_write( uvm_reg_item* rw );

  virtual void post_write( uvm_reg_item* rw );

  virtual void pre_read( uvm_reg_item* rw );

  virtual void post_read( uvm_reg_item* rw );


  //--------------------------------------------------------------------
  // Group: Coverage
  //--------------------------------------------------------------------

 protected: // protected from here on...
  uvm_reg_cvr_t build_coverage( uvm_reg_cvr_t models );

  //also protected
  virtual void add_coverage( uvm_reg_cvr_t models );

 public:
  virtual bool has_coverage( uvm_reg_cvr_t models ) const;

  virtual uvm_reg_cvr_t set_coverage( uvm_reg_cvr_t is_on );

  virtual bool get_coverage( uvm_reg_cvr_t is_on );

 protected:
  virtual void sample( uvm_reg_addr_t offset,
                       bool is_read,
                       uvm_reg_map* map );
 public:
  // public variables
  uvm_mem_mam* mam;

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

 private:

  UVM_REGISTER_CB(uvm_mem, uvm_reg_cbs)

  virtual void set_parent( uvm_reg_block* parent );

  void add_map( uvm_reg_map* map );

  void m_lock_model();

  void m_add_vreg( uvm_vreg* vreg );

  void m_delete_vreg( uvm_vreg* vreg );

  virtual uvm_reg_block* get_block();

  // Implementation-defined - Introspection

  uvm_reg_map* get_local_map( const uvm_reg_map* map,
                              const std::string& caller = "" ) const;

  uvm_reg_map* get_default_map( const std::string& caller = "" ) const;

  // Implementation-defined - HDL access

  // was protected; private better?
  bool m_check_access( uvm_reg_item* rw,
                       uvm_reg_map_info*& map_info, // output
                       const std::string& caller );

  virtual void do_write( uvm_reg_item* rw );

  virtual void do_read( uvm_reg_item* rw );

  // Implementation defined - coverage

  void m_sample( uvm_reg_addr_t addr,
                 bool is_read,
                 uvm_reg_map* map );

  // Implementation-defined: Core uvm_object operations

  void do_print( const uvm_printer& printer ) const override;

  std::string convert2string() const override;

  uvm_object* clone() override;

  void do_copy( const uvm_object& rhs) override;

  bool do_compare( const uvm_object& rhs,
                           const uvm_comparer* comparer ) const override;

  void do_pack( uvm_packer& packer ) const override;

  void do_unpack( uvm_packer& packer ) override;

  // local data members

  bool m_locked;
  bool m_read_in_progress;
  bool m_write_in_progress;

  std::string m_access;
  unsigned long m_size;

  typedef std::map<uvm_reg_map*, bool> m_maps_t;
  typedef m_maps_t::iterator m_maps_itt;
  mutable m_maps_t m_maps;

  unsigned int m_n_bits;

  uvm_reg_block* m_parent;
  uvm_reg_backdoor* m_backdoor;

  bool m_is_powered_down;
  int m_has_cover;
  int m_cover_on;
  std::string m_fname;
  int m_lineno;

  typedef std::map<uvm_vreg*, bool> m_vregs_t;
  typedef m_vregs_t::iterator m_vregs_itt;
  mutable m_vregs_t m_vregs;

  std::map<std::string, std::vector<uvm_hdl_path_concat> > m_hdl_paths_pool;
  typedef std::map<std::string, std::vector<uvm_hdl_path_concat> >::const_iterator m_hdl_paths_pool_itT;

  static int unsigned  m_max_size;

}; // class uvm_mem

} // namespace uvm

#endif // UVM_MEM_H_
