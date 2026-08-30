//----------------------------------------------------------------------
//   Copyright 2013-2015 NXP B.V.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2010 Cadence Design Systems, Inc.
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

#ifndef UVM_REG_FILE_H_
#define UVM_REG_FILE_H_

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/reg/uvm_reg_model.h"
#include "uvmsc/reg/uvm_hdl_path_concat.h"
#include "uvmsc/conf/uvm_object_string_pool.h"

namespace uvm {


// forward class declarations
class uvm_reg_block;

//----------------------------------------------------------------------
// CLASS: uvm_reg_file
//
//! Register file abstraction base class
//!
//! A register file is a collection of register files and registers
//! used to create regular repeated structures.
//!
//! Register files are usually instantiated as arrays.
//----------------------------------------------------------------------

class uvm_reg_file : public uvm_object
{
 public:

  //--------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------

  //--------------------------------------------------------------------
  // Group: Initialization
  //--------------------------------------------------------------------

  explicit uvm_reg_file( const std::string& name = "" );

  void configure( uvm_reg_block* blk_parent,
                  uvm_reg_file* regfile_parent,
                  const std::string& hdl_path = "" );

  //--------------------------------------------------------------------
  // Group: Introspection
  //--------------------------------------------------------------------

  // virtual string get_name() const; // inherited from base class

  const std::string get_full_name() const override;

  virtual uvm_reg_block* get_parent() const;

  virtual uvm_reg_block* get_block() const;

  virtual uvm_reg_file* get_regfile() const;

  //--------------------------------------------------------------------
  // Group: Backdoor
  //--------------------------------------------------------------------

  void clear_hdl_path( const std::string& kind = "RTL" );

  void add_hdl_path( const std::string& path, const std::string& kind = "RTL" );

  bool has_hdl_path( const std::string& kind = "" ) const;

  void get_hdl_path( std::vector<std::string>& paths, const std::string& kind = "" ) const;

  void get_full_hdl_path( std::vector<std::string>& paths,
                          std::string kind = "",
                          const std::string& separator = "." ) const;

  void set_default_hdl_path( std::string kind );

  std::string get_default_hdl_path() const;

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  // destructor
  ~uvm_reg_file() override;

 private:

  void do_print( const uvm_printer& printer ) const override;

  std::string convert2string() const override;

  uvm_object* clone() override;

  void do_copy( const uvm_object& rhs ) override;

  bool do_compare( const uvm_object& rhs,
                           const uvm_comparer* comparer) const override;

  void do_pack( uvm_packer& packer ) const override;

  void do_unpack( uvm_packer& packer ) override;


  // local data members
 private:

  uvm_reg_block* m_parent;

  uvm_reg_file* m_rf;

  std::string m_default_hdl_path; // default set to "RTL" in constructor

  std::map<std::string, std::vector<std::string> > m_hdl_paths_pool;

}; // class uvm_reg_file


///////////

} // namespace uvm

#endif // UVM_REG_FILE_H_
