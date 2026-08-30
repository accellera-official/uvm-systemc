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

#ifndef UVM_VREG_FIELD_H_
#define UVM_VREG_FIELD_H_

#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/reg/uvm_reg_cbs_types.h"
#include "uvmsc/macros/uvm_object_defines.h"
#include "uvmsc/factory/uvm_object_registry.h"

namespace uvm {

// forward class declarations
class uvm_sequence_base;
class uvm_vreg;
class uvm_reg_map;

//----------------------------------------------------------------------
// Class: uvm_vreg_field
//
//! Virtual field abstraction class
//!
//! A virtual field represents a set of adjacent bits that are
//! logically implemented in consecutive memory locations.
//----------------------------------------------------------------------

class uvm_vreg_field : public uvm_object
{
 public:
  friend class uvm_vreg;

  UVM_OBJECT_UTILS(uvm_vreg_field);

  //--------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------

  //--------------------------------------------------------------------
  // Group: Initialization
  //--------------------------------------------------------------------

  explicit uvm_vreg_field( const std::string& name = "uvm_vreg_field" );

  void configure( uvm_vreg* parent,
                  unsigned int size,
                  unsigned int lsb_pos );

  //--------------------------------------------------------------------
  // Group: Introspection
  //--------------------------------------------------------------------

  // virtual get_name() const;

  const std::string get_full_name() const override;

  virtual uvm_vreg* get_parent() const;

  virtual unsigned int get_lsb_pos_in_register() const;

  virtual unsigned int get_n_bits() const;

  virtual std::string get_access( uvm_reg_map* map = nullptr ) const;

  //--------------------------------------------------------------------
  // Group: HDL Access
  //--------------------------------------------------------------------

  virtual void write( unsigned long idx,
                      uvm_status_e&  status,
                      uvm_reg_data_t value,
                      uvm_path_e path = UVM_DEFAULT_PATH,
                      uvm_reg_map* map = nullptr,
                      uvm_sequence_base*  parent = nullptr,
                      uvm_object*         extension = nullptr,
                      const std::string& fname = "",
                      int lineno = 0 );

  virtual void read( unsigned long idx,
                     uvm_status_e& status,
                     uvm_reg_data_t& value,
                     uvm_path_e path = UVM_DEFAULT_PATH,
                     uvm_reg_map* map = nullptr,
                     uvm_sequence_base* parent = nullptr,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void poke( unsigned long idx,
                     uvm_status_e& status,
                     uvm_reg_data_t value,
                     uvm_sequence_base* parent = nullptr,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void peek( unsigned long idx,
                     uvm_status_e& status,
                     uvm_reg_data_t& value,
                     uvm_sequence_base* parent = nullptr,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  //--------------------------------------------------------------------
  // Group: Callbacks
  //--------------------------------------------------------------------

  virtual void pre_write( unsigned long idx,
                          uvm_reg_data_t& wdat,
                          uvm_path_e& path,
                          uvm_reg_map*& map);

  virtual void post_write( unsigned long idx,
                           uvm_reg_data_t wdat,
                           uvm_path_e path,
                           uvm_reg_map* map,
                           uvm_status_e& status );

  virtual void pre_read( unsigned long idx,
                         uvm_path_e& path,
                         uvm_reg_map*& map);

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

  UVM_REGISTER_CB(uvm_vreg_field, uvm_vreg_field_cbs)

  virtual uvm_vreg* get_register() const;

  //--------------------------------------------------------------------
  // Implementation defined: UVM object methods
  //--------------------------------------------------------------------

  void do_print( const uvm_printer& printer ) const override;
  std::string convert2string() const override;
  uvm_object* clone() override;
  void do_copy( const uvm_object& rhs ) override;
  bool do_compare( const uvm_object& rhs,
                           const uvm_comparer* comparer ) const override;
  void do_pack( uvm_packer& packer ) const override;
  void do_unpack( uvm_packer& packer ) override;

private:
  uvm_vreg* m_parent;
  unsigned int m_lsb;
  unsigned int m_size;
  std::string m_fname;
  int m_lineno;
  bool m_read_in_progress;
  bool m_write_in_progress;

}; // class uvm_vreg_field

} // namespace uvm

#endif // UVM_VREG_FIELD_H_
