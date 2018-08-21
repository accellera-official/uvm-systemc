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

#ifndef UVM_REG_H_
#define UVM_REG_H_

#include <systemc>
#include <vector>
#include <map>
#include <iostream>

#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/reg/uvm_reg_cbs_types.h"
#include "uvmsc/reg/uvm_hdl_path_concat.h"
#include "uvmsc/conf/uvm_object_string_pool.h"
#include "uvmsc/base/uvm_mutex.h"


namespace uvm {

// forward class declaration
class uvm_reg_block;
class uvm_reg_field;
class uvm_reg_item;
class uvm_reg_file;
class uvm_reg_map;
class uvm_reg_frontdoor;
class uvm_reg_backdoor;
class uvm_reg_map_info;
class uvm_sequence_base;

//-----------------------------------------------------------------
// CLASS: uvm_reg
//
//! Register abstraction base class
//!
//! A register represents a set of fields that are accessible
//! as a single entity.
//!
//! A register may be mapped to one or more address maps,
//! each with different access rights and policy.
//-----------------------------------------------------------------

class uvm_reg : public uvm_object
{
 public:
  friend class uvm_reg_block;
  friend class uvm_reg_field;
  friend class uvm_reg_map;
  friend class uvm_reg_indirect_data; // TODO change access policy instead?
  friend class uvm_reg_indirect_ftdr_seq;
  template <typename T, typename CB>  friend class uvm_callbacks;
  template <typename BUSTYPE> friend class uvm_reg_predictor;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Group: Initialization
  //--------------------------------------------------------------------------

  uvm_reg( const std::string& name, // no default possible
           unsigned int,
           int has_coverage );

  void configure( uvm_reg_block* blk_parent,
                  uvm_reg_file* regfile_parent = NULL,
                  const std::string& hdl_path = "" );

  void set_offset( uvm_reg_map* map,
                   uvm_reg_addr_t offset,
                   bool unmapped = false );

  //--------------------------------------------------------------------------
  // Group: Introspection
  //--------------------------------------------------------------------------

  // virtual string get_name() const;

  virtual const std::string get_full_name() const;

  virtual uvm_reg_block* get_parent() const;

  virtual uvm_reg_file* get_regfile() const;

  virtual int get_n_maps() const;

  bool is_in_map( uvm_reg_map* map ) const;

  virtual void get_maps( std::vector<uvm_reg_map*>& maps ) const;

  virtual std::string get_rights( uvm_reg_map* map = NULL ) const;

  virtual unsigned int get_n_bits() const;

  virtual unsigned int get_n_bytes() const;

  static unsigned int get_max_size();

  virtual void get_fields( std::vector<uvm_reg_field*>& fields ) const;

  virtual uvm_reg_field* get_field_by_name( const std::string& name ) const;

  virtual uvm_reg_addr_t get_offset( uvm_reg_map* map = NULL ) const;

  virtual uvm_reg_addr_t get_address( const uvm_reg_map* map = NULL ) const;

  // TODO arguments swapped in UVM-SystemC compared to UVM-SV to enable defaults
  virtual int get_addresses( std::vector<uvm_reg_addr_t>& addr,
                             const uvm_reg_map* map = NULL ) const;

  //--------------------------------------------------------------------------
  // Group: Access
  //--------------------------------------------------------------------------

  virtual void set( uvm_reg_data_t value,
                    const std::string& fname = "",
                    int lineno = 0 );

  virtual uvm_reg_data_t get( const std::string& fname = "",
                              int lineno = 0 ) const;

  // TODO not in LRM? - Implementation defined??
  virtual uvm_reg_data_t get_mirrored_value( const std::string& fname = "",
                                             int lineno = 0 ) const;

  virtual bool needs_update() const;

  virtual void reset( const std::string& kind = "HARD" );

  virtual uvm_reg_data_t get_reset( const std::string& kind = "HARD" ) const;

  virtual bool has_reset( const std::string& kind = "HARD",
                          bool do_delete = false );

  virtual void set_reset( uvm_reg_data_t value,
                          const std::string& kind = "HARD" );

  virtual void write( uvm_status_e& status,
                      uvm_reg_data_t value,
                      uvm_path_e path = UVM_DEFAULT_PATH,
                      uvm_reg_map* map = NULL,
                      uvm_sequence_base* parent = NULL,
                      int prior = -1,
                      uvm_object* extension = NULL,
                      const std::string& fname = "",
                      int lineno = 0 );

  virtual void read( uvm_status_e& status,
                     uvm_reg_data_t& value,
                     uvm_path_e path = UVM_DEFAULT_PATH,
                     uvm_reg_map* map = NULL,
                     uvm_sequence_base* parent = NULL,
                     int prior = -1,
                     uvm_object* extension = NULL,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void poke( uvm_status_e& status,
                     uvm_reg_data_t value,
                     const std::string& kind = "",
                     uvm_sequence_base* parent = NULL,
                     uvm_object* extension = NULL,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void peek( uvm_status_e& status,
                     uvm_reg_data_t& value,
                     const std::string& kind = "",
                     uvm_sequence_base* parent = NULL,
                     uvm_object* extension = NULL,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void update( uvm_status_e& status,
                       uvm_path_e path = UVM_DEFAULT_PATH,
                       uvm_reg_map* map = NULL,
                       uvm_sequence_base* parent = NULL,
                       int prior = -1,
                       uvm_object* extension = NULL,
                       const std::string& fname = "",
                       int  lineno = 0 );

  virtual void mirror( uvm_status_e& status,
                       uvm_check_e check  = UVM_NO_CHECK,
                       uvm_path_e path = UVM_DEFAULT_PATH,
                       uvm_reg_map* map = NULL,
                       uvm_sequence_base* parent = NULL,
                       int prior = -1,
                       uvm_object* extension = NULL,
                       const std::string& fname = "",
                       int lineno = 0 );

  virtual bool predict( uvm_reg_data_t value,
                        uvm_reg_byte_en_t be = -1,
                        uvm_predict_e kind = UVM_PREDICT_DIRECT,
                        uvm_path_e path = UVM_FRONTDOOR,
                        uvm_reg_map* map = NULL,
                        const std::string& fname = "",
                        int lineno = 0 );

  bool is_busy() const;

  //--------------------------------------------------------------------------
  // Group: Frontdoor
  //--------------------------------------------------------------------------

  void set_frontdoor( uvm_reg_frontdoor* ftdr,
                      uvm_reg_map* map = NULL,
                      const std::string& fname = "",
                      int lineno = 0 );

  uvm_reg_frontdoor* get_frontdoor( uvm_reg_map* map = NULL ) const;

  //--------------------------------------------------------------------------
  // Group: Backdoor
  //--------------------------------------------------------------------------

  void set_backdoor( uvm_reg_backdoor* bkdr,
                     const std::string& fname = "",
                     int lineno = 0 );

  uvm_reg_backdoor* get_backdoor( bool inherited = true ) const;

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

  void get_hdl_path_kinds( std::vector<std::string>& kinds ) const;

  void get_full_hdl_path( std::vector<uvm_hdl_path_concat>& paths,
                          const std::string& kind = "",
                          const std::string& separator = ".") const;

  virtual void backdoor_read( uvm_reg_item* rw );

  virtual void backdoor_write( uvm_reg_item* rw );

  virtual uvm_status_e backdoor_read_func( uvm_reg_item* rw );

  virtual void backdoor_watch();

  //--------------------------------------------------------------------------
  // Group: Coverage
  //--------------------------------------------------------------------------

  static void include_coverage( const std::string& scope,
                                uvm_reg_cvr_t models,
                                uvm_object* accessor = NULL );

 protected:
  uvm_reg_cvr_t build_coverage( uvm_reg_cvr_t models );

  virtual void add_coverage( uvm_reg_cvr_t models );

 public:
  virtual bool has_coverage( uvm_reg_cvr_t models ) const;

  virtual uvm_reg_cvr_t set_coverage( uvm_reg_cvr_t is_on );

  virtual bool get_coverage( uvm_reg_cvr_t is_on ) const;

 protected:
  virtual void sample( uvm_reg_data_t data,
                       uvm_reg_data_t byte_en,
                       bool is_read,
                       uvm_reg_map* map );

 public:
  virtual void sample_values();


  //--------------------------------------------------------------------------
  // Group: Callbacks
  //--------------------------------------------------------------------------

  virtual void pre_write( uvm_reg_item* rw );

  virtual void post_write( uvm_reg_item* rw );

  virtual void pre_read( uvm_reg_item* rw );

  virtual void post_read( uvm_reg_item* rw );


  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

 private:

  UVM_REGISTER_CB(uvm_reg, uvm_reg_cbs)

  virtual void set_parent( uvm_reg_block* blk_parent,
                           uvm_reg_file* regfile_parent );

  uvm_reg_block* get_block() const;

  virtual void add_field( uvm_reg_field* field );


  virtual void add_map( uvm_reg_map* map );

  void m_lock_model();

  // Implementation defined - Introspection

  virtual uvm_reg_map* get_local_map( const uvm_reg_map* map,
		  	  	  	  	  	  	      const std::string& caller = "" ) const;

  virtual uvm_reg_map* get_default_map( const std::string& caller = "" ) const;

  std::string m_get_fields_access( uvm_reg_map* map ) const;

  /////////////

  // Implementation defined - Access

  void m_set_busy( bool busy );

  void m_read( uvm_status_e& status,
               uvm_reg_data_t& value,
               uvm_path_e path,
               uvm_reg_map* map,
               uvm_sequence_base* parent = NULL,
               int prior = -1,
               uvm_object* extension = NULL,
               const std::string& fname = "",
               int lineno = 0 );

  void m_atomic_check_lock( bool on );

  virtual bool m_check_access( uvm_reg_item* rw,
                               uvm_reg_map_info*& map_info,
                               const std::string& caller );

  bool m_get_is_locked_by_field() const;

  virtual bool do_check( uvm_reg_data_t expected,
                         uvm_reg_data_t actual,
                         uvm_reg_map* map);

  virtual void do_write( uvm_reg_item* rw );

  virtual void do_read( uvm_reg_item* rw );

  virtual void do_predict( uvm_reg_item* rw,
                           uvm_predict_e kind = UVM_PREDICT_DIRECT,
                           uvm_reg_byte_en_t be = -1 );

  // Implementation defined - Backdoor

  void m_sample( uvm_reg_data_t data,
                 uvm_reg_data_t byte_en,
                 bool is_read,
                 uvm_reg_map* map );


  // Implementation defined - UVM object

  virtual void do_print( const uvm_printer& printer ) const;

  virtual std::string convert2string() const;

  virtual uvm_object* clone();

  virtual void do_copy( const uvm_object& rhs );

  virtual bool do_compare( const uvm_object& rhs,
                           const uvm_comparer* comparer ) const;

  virtual void do_pack( uvm_packer& packer ) const;

  virtual void do_unpack( uvm_packer& packer );

  // data members

 protected:

  bool m_locked;

  uvm_reg_block* m_parent;
  uvm_reg_file* m_regfile_parent;

  unsigned int m_n_bits;
  unsigned int m_n_used_bits;

  typedef std::map<uvm_reg_map*, bool> m_maps_t;
  typedef m_maps_t::const_iterator m_maps_citt;
  m_maps_t m_maps;

  typedef std::vector<uvm_reg_field*> m_fields_t; // Fields in LSB to MSB order
  typedef m_fields_t::iterator m_fields_itt;
  m_fields_t m_fields;

 private:
  int m_has_cover;
  int m_cover_on;

  mutable std::string m_fname;
  mutable int m_lineno;

  uvm::uvm_mutex m_atomic; // semaphore
  sc_core::sc_process_handle m_process;
  bool m_process_valid;

  bool m_read_in_progress;
  bool m_write_in_progress;

  bool m_is_busy;
  bool m_is_locked_by_field;

  mutable uvm_reg_backdoor* m_backdoor;

  static unsigned int m_max_size;

  //uvm_object_string_pool< uvm_queue<uvm_hdl_path_concat*> >* m_hdl_paths_pool;

  std::map<std::string, std::vector<uvm_hdl_path_concat> > m_hdl_paths_pool;
  typedef std::map<std::string, std::vector<uvm_hdl_path_concat> >::const_iterator m_hdl_paths_pool_itT;

 protected:
  bool m_update_in_progress;

}; // class uvm_reg

} // namespace uvm

#endif /* UVM_REG_H_ */
