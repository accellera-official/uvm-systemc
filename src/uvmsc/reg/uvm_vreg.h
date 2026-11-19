//----------------------------------------------------------------------
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2013-2015 NXP B.V.
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

#ifndef UVM_VREG_H_
#define UVM_VREG_H_

#include <systemc>
#include <vector>
#include <iostream>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/macros/uvm_callback_defines.h"
#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/cb/uvm_callbacks.h"

namespace uvm {

// forward class declaration
class uvm_reg_block;
class uvm_reg_map;
class uvm_vreg_field;
class uvm_sequence_base;
class uvm_mem;
class uvm_mem_region;
class uvm_mem_mam;
class uvm_vreg_cbs;


//----------------------------------------------------------------------
// Title: Virtual Registers
//----------------------------------------------------------------------
//
// A virtual register is a collection of fields,
// overlaid on top of a memory, usually in an array.
// The semantics and layout of virtual registers comes from
// an agreement between the software and the hardware,
// not any physical structures in the DUT.
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Class: uvm_vreg
//
//! Virtual register abstraction base class
//!
//! A virtual register represents a set of fields that are
//! logically implemented in consecutive memory locations.
//!
//! All virtual register accesses eventually turn into memory accesses.
//!
//! A virtual register array may be implemented on top of
//! any memory abstraction class and possibly dynamically
//! resized and/or relocated.
//----------------------------------------------------------------------

class uvm_vreg : public uvm_object
{
 public:
  friend class uvm_reg_map;
  friend class uvm_vreg_field;

  //--------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------

  //--------------------------------------------------------------------
  // Group: Initialization
  //--------------------------------------------------------------------

  explicit uvm_vreg( const std::string& name, unsigned int n_bits );

  void configure( uvm_reg_block* parent,
                  uvm_mem* mem = nullptr,
                  unsigned long size   = 0,
                  uvm_reg_addr_t offset = 0,
                  unsigned int incr   = 0);

  virtual bool implement( unsigned long n,
                          uvm_mem* mem = nullptr,
                          uvm_reg_addr_t offset = 0,
                          unsigned int incr = 0);

  virtual uvm_mem_region* allocate( unsigned long n,
                                    uvm_mem_mam* mam );

  virtual uvm_mem_region* get_region() const;

  virtual void release_region();

  //--------------------------------------------------------------------
  // Group: Introspection
  //--------------------------------------------------------------------

  //virtual std::string get_name() const; // inherit from base class

  const std::string get_full_name() const override;

  virtual uvm_reg_block* get_parent() const;

  virtual uvm_mem* get_memory() const;

  virtual int get_n_maps() const;

  bool is_in_map( uvm_reg_map* map ) const;

  virtual void get_maps( std::vector<uvm_reg_map*>& maps ) const;

  virtual std::string get_rights( uvm_reg_map* map = nullptr ) const;

  virtual std::string get_access( uvm_reg_map* map = nullptr ) const;

  virtual unsigned int get_size() const;

  virtual unsigned int get_n_bytes() const;

  virtual unsigned int get_n_memlocs() const;

  virtual unsigned int get_incr() const;

  virtual void get_fields( std::vector<uvm_vreg_field*>& fields) const;

  virtual uvm_vreg_field* get_field_by_name( const std::string& name ) const;

  virtual uvm_reg_addr_t get_offset_in_memory( unsigned long idx ) const;

  virtual uvm_reg_addr_t get_address( unsigned long idx,
                                      const uvm_reg_map* map = nullptr ) const;

  //--------------------------------------------------------------------
  // Group: HDL Access
  //--------------------------------------------------------------------

  virtual void write( unsigned long idx,
                      uvm_status_e& status, //output
                      uvm_reg_data_t value,
                      uvm_path_e path = UVM_DEFAULT_PATH,
                      uvm_reg_map* map = nullptr,
                      uvm_sequence_base* parent = nullptr,
                      uvm_object* extension = nullptr,
                      const std::string& fname = "",
                      int lineno = 0 );

  virtual void read( unsigned long idx,
                     uvm_status_e& status,  // output
                     uvm_reg_data_t& value, // output
                     uvm_path_e path = UVM_DEFAULT_PATH,
                     uvm_reg_map* map = nullptr,
                     uvm_sequence_base* parent = nullptr,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void poke( unsigned long idx,
                     uvm_status_e& status, // output
                     uvm_reg_data_t value,
                     uvm_sequence_base* parent = nullptr,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void peek( unsigned long idx,
                     uvm_status_e& status, // output
                     uvm_reg_data_t& value, // output
                     uvm_sequence_base* parent = nullptr,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  void reset( const std::string& kind = "HARD" );

  //--------------------------------------------------------------------
  // Group: Callbacks
  //--------------------------------------------------------------------

  virtual void pre_write( unsigned long idx,
                          uvm_reg_data_t& wdat,
                          uvm_path_e& path,
                          uvm_reg_map*& map );

  virtual void post_write( unsigned long idx,
                           uvm_reg_data_t wdat,
                           uvm_path_e path,
                           uvm_reg_map* map,
                           uvm_status_e& status);

  virtual void pre_read( unsigned long idx,
                         uvm_path_e& path,
                         uvm_reg_map*& map );

  virtual void post_read( unsigned long idx,
                          uvm_reg_data_t& rdat,
                          uvm_path_e path,
                          uvm_reg_map* map,
                          uvm_status_e& status);

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

 private:

  UVM_REGISTER_CB(uvm_vreg, uvm_vreg_cbs)

  virtual void set_parent( uvm_reg_block* parent );

  void m_lock_model();

  void add_field( uvm_vreg_field* field );

  void m_atomic_check( bool on );

  virtual uvm_reg_block* get_block() const;

  void do_print( const uvm_printer& printer ) const override;

  std::string convert2string() const override;

  uvm_object* clone() override;

  void do_copy( const uvm_object& rhs ) override;

  bool do_compare( const uvm_object& rhs,
                           const uvm_comparer* comparer ) const override;

  void do_pack( uvm_packer& packer ) const override;

  void do_unpack( uvm_packer& packer ) override;

  // local data members
 private:

  bool m_locked;
  uvm_reg_block* m_parent;
  unsigned int m_n_bits;
  unsigned int m_n_used_bits;

  typedef std::vector<uvm_vreg_field*> m_fields_t;
  typedef m_fields_t::iterator m_fields_itt;
  m_fields_t m_fields;   // Fields in LSB to MSB order

  uvm_mem*       m_mem;     // Where is it implemented?
  uvm_reg_addr_t m_offset;  // Start of vreg[0]
  unsigned int   m_incr;    // From start to start of next
  unsigned long  m_size;    // number of vregs
  bool           m_is_static;

  uvm_mem_region* m_region;    // Not nullptr if implemented via MAM
  sc_core::sc_mutex m_atomic;  // Semaphore - Field RMW operations must be atomic

  std::string m_fname;
  int m_lineno;
  bool m_read_in_progress;
  bool m_write_in_progress;

}; // class uvm_vreg

///////////

} // namespace uvm

#endif // UVM_VREG_H_
