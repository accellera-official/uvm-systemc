//----------------------------------------------------------------------
//   Copyright 2013-2015 NXP B.V.
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

#ifndef UVM_REG_FIELD_H_
#define UVM_REG_FIELD_H_

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/macros/uvm_object_defines.h"
#include "uvmsc/factory/uvm_object_registry.h"
#include "uvmsc/reg/uvm_reg.h"

//////////////

namespace uvm {


//-----------------------------------------------------------------
// CLASS: uvm_reg_field
//
//! Field abstraction class
//!
//! A field represents a set of bits that behave consistently
//! as a single entity.
//!
//! A field is contained within a single register, but may
//! have different access policies depending on the address map
//! use the access the register (thus the field).
//-----------------------------------------------------------------

class uvm_reg_field : public uvm_object
{
 public:
  friend class uvm_reg_fifo;
  template <typename T, typename CB>  friend class uvm_callbacks;

  UVM_OBJECT_UTILS(uvm_reg_field)

  //---------------------------------------------------------------
  // UVM Standard LRM API below
  //---------------------------------------------------------------

  //---------------------------------------------------------------
  // Group: Initialization
  //---------------------------------------------------------------

  uvm_reg_field( const std::string& name = "uvm_reg_field" );

  void configure( uvm_reg* parent,
                  unsigned int size,
                  unsigned int lsb_pos,
                  const std::string& access,
                  bool is_volatile, // changed icm UVM-SV
                  uvm_reg_data_t reset,
                  bool has_reset,
                  bool is_rand,
                  bool individually_accessible );

  //---------------------------------------------------------------
  // Group: Introspection
  //---------------------------------------------------------------

  // virtual string get_name(); // inherit from base class

  const std::string get_full_name() const override;

  virtual uvm_reg* get_parent() const;

  virtual unsigned int get_lsb_pos() const;

  virtual unsigned int get_n_bits() const;

  static unsigned int get_max_size();

  virtual std::string set_access( const std::string& mode );

  static bool define_access( std::string name );

  virtual std::string get_access( uvm_reg_map* map = nullptr ) const;

  virtual bool is_known_access( uvm_reg_map* map = nullptr ) const;

  virtual void set_volatility( bool is_volatile ); // changed argument icm UVM-SV

  virtual bool is_volatile() const;

  //---------------------------------------------------------------
  // Group: Access
  //---------------------------------------------------------------

  virtual void set( uvm_reg_data_t value,
		  	  	  	const std::string& fname = "",
                    int lineno = 0);

  virtual uvm_reg_data_t get( const std::string& fname = "",
                              int lineno = 0 ) const;

  virtual uvm_reg_data_t get_mirrored_value( const std::string& fname = "",
                                             int lineno = 0 ) const;

  virtual void reset( const std::string& kind = "HARD" );

  virtual uvm_reg_data_t get_reset( const std::string& kind = "HARD" ) const;

  virtual bool has_reset( const std::string& kind = "HARD",
                          bool do_delete = 0 );

  virtual void set_reset( uvm_reg_data_t value,
		  	  	  	  	  const std::string& kind = "HARD" );

  virtual bool needs_update() const;

  virtual void write( uvm_status_e& status,
                      uvm_reg_data_t value,
                      uvm_path_e path = UVM_DEFAULT_PATH,
                      uvm_reg_map* map = nullptr,
                      uvm_sequence_base* parent = nullptr,
                      int prior = -1,
                      uvm_object* extension = nullptr,
                      const std::string& fname = "",
                      int lineno = 0 );

  virtual void read( uvm_status_e& status,
                     uvm_reg_data_t& value,
                     uvm_path_e path = UVM_DEFAULT_PATH,
                     uvm_reg_map* map = nullptr,
                     uvm_sequence_base* parent = nullptr,
                     int prior = -1,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void poke( uvm_status_e& status,
                     uvm_reg_data_t value,
                     const std::string& kind = "",
                     uvm_sequence_base* parent = nullptr,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void peek( uvm_status_e& status,
                     uvm_reg_data_t& value,
                     const std::string& kind = "",
                     uvm_sequence_base* parent = nullptr,
                     uvm_object* extension = nullptr,
                     const std::string& fname = "",
                     int lineno = 0 );

  virtual void mirror( uvm_status_e& status,
                       uvm_check_e check = UVM_NO_CHECK,
                       uvm_path_e path = UVM_DEFAULT_PATH,
                       uvm_reg_map* map = nullptr,
                       uvm_sequence_base* parent = nullptr,
                       int prior = -1,
                       uvm_object* extension = nullptr,
                       const std::string& fname = "",
                       int lineno = 0 );

  void set_compare( uvm_check_e check = UVM_CHECK );

  uvm_check_e get_compare() const;

  bool is_indv_accessible( uvm_path_e path,
                           uvm_reg_map* local_map) const;

  bool predict( uvm_reg_data_t value,
                uvm_reg_byte_en_t be = -1,
                uvm_predict_e kind = UVM_PREDICT_DIRECT,
                uvm_path_e path = UVM_FRONTDOOR,
                uvm_reg_map* map = nullptr,
                const std::string& fname = "",
                int lineno = 0 );

  //---------------------------------------------------------------
  // Group: Callbacks
  //---------------------------------------------------------------

  virtual void pre_write( uvm_reg_item* rw );

  virtual void post_write( uvm_reg_item* rw );

  virtual void pre_read( uvm_reg_item* rw);

  virtual void post_read( uvm_reg_item* rw);

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  virtual uvm_reg* get_register() const;

  static bool m_predefine_policies();

  virtual uvm_reg_data_t m_predict( uvm_reg_data_t cur_val,
                                    uvm_reg_data_t wr_val,
                                    uvm_reg_map* map );

  virtual uvm_reg_data_t m_update();

  bool m_check_access( uvm_reg_item* rw,
                       uvm_reg_map_info*& map_info,
                       const std::string& caller );

  virtual void do_write( uvm_reg_item* rw );

  virtual void do_read( uvm_reg_item* rw );

  virtual void do_predict( uvm_reg_item* rw,
                           uvm_predict_e kind = UVM_PREDICT_DIRECT,
                           uvm_reg_byte_en_t be = -1 );

  void pre_randomize();
  void post_randomize();

  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  // Implementation defined - member functions for UVM object
  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////

  void do_print( const uvm_printer& printer ) const override;
  std::string convert2string() const override;
  uvm_object* clone() override;
  void do_copy( const uvm_object& rhs ) override;
  bool do_compare( const uvm_object& rhs,
                           const uvm_comparer* comparer ) const override;
  void do_pack( uvm_packer& packer ) const override;
  void do_unpack( uvm_packer& packer ) override;

 private:

  static std::map<std::string, bool>& m_policy_names();

  UVM_REGISTER_CB( uvm_reg_field, uvm_reg_cbs )

  // data members

  static bool m_predefined;
  static bool m_predefined_policies;

  /*rand*/ uvm_reg_data_t m_value; // Mirrored after randomize() // TODO randomize

  uvm_reg_data_t m_mirrored; // What we think is in the HW
  uvm_reg_data_t m_desired;  // Mirrored after set()
  std::string  m_access;
  uvm_reg* m_parent;
  unsigned int m_lsb;
  unsigned int m_size;
  bool m_volatile;
  std::map <std::string, uvm_reg_data_t> m_reset;
  bool m_written;
  bool m_read_in_progress;
  bool m_write_in_progress;

  mutable std::string m_fname;
  mutable int m_lineno;

  int m_cover_on;
  bool  m_individually_accessible;
  uvm_check_e m_check;

  static unsigned int m_max_size;


  /* TODO constraints...
       constraint uvm_reg_field_valid {
         if (`UVM_REG_DATA_WIDTH > m_size) {
           value < (`UVM_REG_DATA_WIDTH'h1 << m_size);
         }
       }
  */

}; // class uvm_reg_field

} // namespace uvm


#endif /* UVM_REG_FIELD_H_ */

