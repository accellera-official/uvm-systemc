//----------------------------------------------------------------------
//   Copyright 2012-2015 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
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

#ifndef UVM_OBJECT_H_
#define UVM_OBJECT_H_

#include <string>
#include <iostream>
#include <vector>

#include <systemc>
#include "sysc/utils/sc_hash.h"

#include "uvmsc/base/uvm_void.h"
#include "uvm_object_globals.h"


//////////////

namespace uvm {

// forward declaration of uvm_packer used in uvm_object
class uvm_packer;
class uvm_printer;
class uvm_status_container;
class uvm_object_wrapper;
class uvm_recorder;
class uvm_comparer;

// instance count
extern int g_inst_count;

extern uvm_packer* uvm_default_packer;
extern uvm_comparer* uvm_default_comparer;


// TODO typesdefs for intstream, bytestream, etc.
//typedef vector<int> intstream;
//typedef vector<char> bytestream;
//typedef vector<bool> bitstream;

//----------------------------------------------------------------------------
// CLASS: uvm_object
//
// !Base class for data objects.
//----------------------------------------------------------------------------

class uvm_object : public uvm_void
{
  friend class uvm_packer_rep;
  template <typename T, typename CB> friend class uvm_callbacks;
  friend class uvm_callbacks_base;

public:
  //--------------------------------------------------------------------------
  // Constructors and destructor
  //--------------------------------------------------------------------------

  uvm_object();
  explicit uvm_object( uvm_object_name name );
  virtual ~uvm_object();

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Group: Seeding
  //--------------------------------------------------------------------------

  // static bool use_uvm_seeding;

  // void reseed();

  //--------------------------------------------------------------------------
  // Group: Identification
  //--------------------------------------------------------------------------

  virtual void set_name( const std::string& name );

  virtual const std::string get_name() const;

  virtual const std::string get_full_name() const;

  virtual int get_inst_id() const;

  static int get_inst_count();

  static const uvm_object_wrapper* get_type();

  virtual const uvm_object_wrapper* get_object_type() const;

  virtual const std::string get_type_name() const;

  //--------------------------------------------------------------------------
  // Group: Creation
  //--------------------------------------------------------------------------

  virtual uvm_object* create( const std::string& name = "" );

  virtual uvm_object* clone();

  //--------------------------------------------------------------------------
  // Group: Printing
  //--------------------------------------------------------------------------

  void print( uvm_printer* printer = NULL ) const;

  std::string sprint( uvm_printer* printer = NULL ) const;

  virtual void do_print( const uvm_printer& printer ) const;

  virtual std::string convert2string() const;

  //--------------------------------------------------------------------------
  // Group: Recording
  //--------------------------------------------------------------------------

  void record( uvm_recorder* recorder = NULL );

  virtual void do_record( const uvm_recorder& recorder );

  //--------------------------------------------------------------------------
  // Group: Copying
  //--------------------------------------------------------------------------

  void copy( const uvm_object& rhs );

  virtual void do_copy( const uvm_object& rhs );

  //--------------------------------------------------------------------------
  // Group: Comparing
  //--------------------------------------------------------------------------

  bool compare( const uvm_object& rhs, const uvm_comparer* comparer = NULL ) const;

  virtual bool do_compare( const uvm_object& rhs,
                           const uvm_comparer* comparer = NULL ) const;

  //--------------------------------------------------------------------------
  // Group: Packing
  //--------------------------------------------------------------------------

  int pack( std::vector<bool>& bitstream, uvm_packer* packer = NULL );

  int pack_bytes( std::vector<unsigned char>& bytestream, uvm_packer* packer = NULL );

  int pack_ints( std::vector<unsigned int>& intstream, uvm_packer* packer = NULL );

  virtual void do_pack( uvm_packer& packer ) const;

  //--------------------------------------------------------------------------
  // Group: Unpacking
  //--------------------------------------------------------------------------

  int unpack( const std::vector<bool>& bitstream, uvm_packer* packer = NULL );

  int unpack_bytes( const std::vector<unsigned char>& bytestream, uvm_packer* packer = NULL );

  int unpack_ints( const std::vector<unsigned int>& intstream, uvm_packer* packer = NULL );

  virtual void do_unpack( uvm_packer& packer );

  //--------------------------------------------------------------------------
  // Group: Configuration
  //--------------------------------------------------------------------------
/*
  virtual void set_int_local( const std::string& field_name,
                              int value,             //TODO make uvm_bitstream_t ?
                              bool recurse = true );

  virtual void set_string_local( const std::string& field_name,
                                 const std::string& value,
                                 bool recurse = true );

  virtual void set_object_local( const std::string& field_name,
                                 uvm_object* value,
                                 bool clone = true,
                                 bool recurse = true );
*/

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  friend std::ostream& operator<<( std::ostream& os, const uvm_object& obj );
  friend std::ostream& operator<<( std::ostream& os, const uvm_object* obj );

  //TODO do we need equality operators?
  /*
  friend bool operator == ( const uvm_object& a, const uvm_object& b );
  friend bool operator != ( const uvm_object& a, const uvm_object& b );
  */

  static uvm_packer* get_uvm_packer();

 private:
  void m_pack( uvm_packer*& packer );
  void m_unpack_pre( uvm_packer*& packer );
  void m_unpack_post( uvm_packer*& packer );

 protected:
  virtual bool m_register_cb();

  // data members below

 public:
  static uvm_status_container* __m_uvm_status_container;

 protected:
  std::string m_leaf_name;
  std::string m_full_name;

  int m_inst_id;
};

//////////////////////

} // namespace uvm

#endif /* UVM_OBJECT_H_ */
