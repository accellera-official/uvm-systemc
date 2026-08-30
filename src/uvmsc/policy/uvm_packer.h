//------------------------------------------------------------------------------
//   Copyright 2014-2015 NXP B.V.
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
//------------------------------------------------------------------------------

#ifndef UVM_PACKER_H_
#define UVM_PACKER_H_

#include <systemc>
#include <sstream>

#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/misc/uvm_scope_stack.h"

#define UVM_PACKING_BLOCK_SIZE 4096

namespace uvm {

// forward class references
class uvm_object;

//------------------------------------------------------------------------------
// Class: uvm_packer
//
// The uvm_packer class provides a policy object for packing and unpacking
// uvm_objects. The policies determine how packing and unpacking should be done.
// Packing an object causes the object to be placed into a bit (byte or int)
// array. If the `uvm_field_* macro are used to implement pack and unpack,
// by default no metadata information is stored for the packing of dynamic
// objects (strings, arrays, class objects).
//
//-------------------------------------------------------------------------------

class uvm_packer
{
 public:
  friend class uvm_object;
  friend class uvm_default_coreservice_t;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  // Group: Packing

  virtual void pack_field( const uvm_bitstream_t& value, int size);

  virtual void pack_field_int( const uvm_integral_t& value, int size );

  virtual void pack_string( const std::string& value );

  virtual void pack_time( const sc_core::sc_time& value );

  virtual void pack_real( double value );

  virtual void pack_object( const uvm_object& value );

  // Group: Unpacking

  virtual bool is_null();

  virtual uvm_integral_t unpack_field_int( int size );

  virtual uvm_bitstream_t unpack_field( int size );

  virtual std::string unpack_string( int num_chars = -1 );

  virtual sc_core::sc_time unpack_time();

  virtual double unpack_real();

  virtual void unpack_object( uvm_object& value );

  virtual unsigned int get_packed_size() const;

  // Group: Variables

  // Variable: physical
  //
  // This bit provides a filtering mechanism for fields.
  //
  // The <abstract> and physical settings allow an object to distinguish between
  // two different classes of fields. It is up to you, in the
  // <uvm_object::do_pack> and <uvm_object::do_unpack> methods, to test the
  // setting of this field if you want to use it as a filter.

  bool physical;

  // Variable: abstract
  //
  // This bit provides a filtering mechanism for fields. 
  //
  // The abstract and physical settings allow an object to distinguish between
  // two different classes of fields. It is up to you, in the
  // <uvm_object::do_pack> and <uvm_object::do_unpack> routines, to test the
  // setting of this field if you want to use it as a filter.

  bool abstract;

  // Variable: use_metadata
  //
  // This flag indicates whether to encode metadata when packing dynamic data,
  // or to decode metadata when unpacking.  Implementations of <uvm_object::do_pack>
  // and <uvm_object::do_unpack> should regard this bit when performing their
  // respective operation. When set, metadata should be encoded as follows:
  //
  // - For strings, pack an additional null byte after the string is packed.
  //
  // - For objects, pack 4 bits prior to packing the object itself. Use 4'b0000
  //   to indicate the object being packed is null, otherwise pack 4'b0001 (the
  //   remaining 3 bits are reserved).
  //
  // - For queues, dynamic arrays, and associative arrays, pack 32 bits
  //   indicating the size of the array prior to to packing individual elements.

  bool use_metadata;

  // Variable: big_endian
  //
  // This bit determines the order that integral data is packed (using
  // <pack_field>, <pack_field_int>, <pack_time>, or <pack_real>) and how the
  // data is unpacked from the pack array (using <unpack_field>,
  // <unpack_field_int>, <unpack_time>, or <unpack_real>). When the bit is set,
  // data is associated msb to lsb; otherwise, it is associated lsb to msb. 

  bool big_endian;

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  //----------------------------------------------------------------------------
  // Operator << for data types that can be packed using this class
  //
  // These will be invoked from uvm_object's do_pack() method
  //----------------------------------------------------------------------------

  virtual uvm_packer& operator<< ( bool a );
  virtual uvm_packer& operator<< ( double a );
  virtual uvm_packer& operator<< ( float a );
  virtual uvm_packer& operator<< ( char a );
  virtual uvm_packer& operator<< ( unsigned char a );
  virtual uvm_packer& operator<< ( short a );
  virtual uvm_packer& operator<< ( unsigned short a );
  virtual uvm_packer& operator<< ( int a );
  virtual uvm_packer& operator<< ( unsigned int a );
  virtual uvm_packer& operator<< ( long a );
  virtual uvm_packer& operator<< ( unsigned long a );
  virtual uvm_packer& operator<< ( long long a );
  virtual uvm_packer& operator<< ( unsigned long long a );

  virtual uvm_packer& operator<< ( const std::string& a );
  virtual uvm_packer& operator<< ( const char* );

  virtual uvm_packer& operator<< ( const uvm_object& a );
  virtual uvm_packer& operator<< ( const sc_dt::sc_logic& a );
  virtual uvm_packer& operator<< ( const sc_dt::sc_bv_base& a );
  virtual uvm_packer& operator<< ( const sc_dt::sc_lv_base& a );
  virtual uvm_packer& operator<< ( const sc_dt::sc_int_base& a );
  virtual uvm_packer& operator<< ( const sc_dt::sc_uint_base& a );
  virtual uvm_packer& operator<< ( const sc_dt::sc_signed& a );
  virtual uvm_packer& operator<< ( const sc_dt::sc_unsigned& a );

  virtual uvm_packer& operator<< ( const sc_core::sc_time& a );

  template <class T>
  uvm_packer& operator<< ( const std::vector<T>& a )
  {
    // first pack the size of the vector before packing its elements
    int n = (int)a.size();
    (*this) << n;
    for (int i = 0; i < n; i++) {
      (*this) << a[i];
    }
    return *this;
  }

  virtual uvm_packer& operator<< ( const std::vector<bool>& a )
  {
    // first pack the size of the vector before packing its elements
    int n = (int)a.size();
    (*this) << n;
    for (int i = 0; i < n; i++) {
      (*this) << a[i];
    }
    return *this;
  }

  //----------------------------------------------------------------------------
  // Operator >> for data types that can be unpacked using this class
  //
  // These will be invoked from uvm_object's do_unpack() method
  //----------------------------------------------------------------------------

  virtual uvm_packer& operator>> ( bool& a );
  virtual uvm_packer& operator>> ( double& a );
  virtual uvm_packer& operator>> ( float& a );
  virtual uvm_packer& operator>> ( char& a );
  virtual uvm_packer& operator>> ( unsigned char& a );
  virtual uvm_packer& operator>> ( short& a );
  virtual uvm_packer& operator>> ( unsigned short& a );
  virtual uvm_packer& operator>> ( int& a );
  virtual uvm_packer& operator>> ( unsigned int& a );
  virtual uvm_packer& operator>> ( long& a );
  virtual uvm_packer& operator>> ( unsigned long& a );
  virtual uvm_packer& operator>> ( long long& a );
  virtual uvm_packer& operator>> ( unsigned long long& a );
  virtual uvm_packer& operator>> ( std::string& a );
  virtual uvm_packer& operator>> ( uvm_object& a );
  virtual uvm_packer& operator>> ( sc_dt::sc_logic& a );
  virtual uvm_packer& operator>> ( sc_dt::sc_bv_base& a );
  virtual uvm_packer& operator>> ( sc_dt::sc_lv_base& a );
  virtual uvm_packer& operator>> ( sc_dt::sc_int_base& a );
  virtual uvm_packer& operator>> ( sc_dt::sc_uint_base& a );
  virtual uvm_packer& operator>> ( sc_dt::sc_signed& a );
  virtual uvm_packer& operator>> ( sc_dt::sc_unsigned& a );

  virtual uvm_packer& operator>> ( sc_core::sc_time& a );

  template <class T>
  uvm_packer& operator>> ( std::vector<T>& a )
  {
    a.clear();
    int n;
    // first unpack the size of the vector before unpacking its elements
    (*this) >> n;
    for (int i = 0; i < n; i++) {
      T t;
      (*this) >> t;
      a.push_back(t);
    }
    return *this;
  }

  virtual uvm_packer& operator>> ( std::vector<bool>& a )
  {
    a.clear();
    int n;
    // first unpack the size of the vector before unpacking its elements
    (*this) >> n;
    for (int i = 0; i < n; i++) {
      bool t;
      (*this) >> t;
      a.push_back(t);
    }
    return *this;
  }

  virtual ~uvm_packer();

 private:
  // Disabled
  uvm_packer();

  // variables and methods primarily for internal use

  //static std::vector<bool> bitstream;   // local bits for (un)pack_bytes
  //static std::vector<bool> fabitstream; // field automation bits for (un)pack_bytes

  unsigned int pack_index;      // used to count the number of packed bits
  unsigned int unpack_index;    // used to count the number of unpacked bits

  uvm_scope_stack scope;

  bool  reverse_order;      // flip the bit order around
  char  byte_size;          // set up bytesize for endianess
  int   word_size;          // set up worksize for endianess
  bool  nopack;             // only count packable bits

  uvm_recursion_policy_enum policy;

  sc_dt::sc_bv_base* m_bits; // TODO was uvm_pack_bitstream_t m_bits;
  int m_size;
  int m_max_size;

  virtual void get_bits( std::vector<bool>& bits ) const;
  virtual void get_bytes( std::vector<unsigned char>& bytes ) const;
  virtual void get_ints( std::vector<unsigned int>& ints ) const;

  virtual void put_bits( const std::vector<bool>& bitstream );
  virtual void put_bytes( const std::vector<unsigned char>& bytestream );
  virtual void put_ints( const std::vector<unsigned int>& intstream );

  void reset();

  bool m_enough_bits( int needed, std::string id ) const;

  void m_set_size(int nbits);

  void m_allocate( int nbits, bool copy_ = true );

  unsigned int get_remaining_unpacked_bits() const;

  void m_check_size( int nbits );

  void inc_unpack_index( int n );

  void pack_char( char a );
  void unpack_char( char& a );

  void pack_bool( bool a );
  void unpack_bool( bool& a );

  void pack_sc_logic( const sc_dt::sc_logic& a );
  void unpack_sc_logic( sc_dt::sc_logic& a );

  void pack_sc_bv_base( const sc_dt::sc_bv_base& a );
  void unpack_sc_bv_base( sc_dt::sc_bv_base& a );

  void pack_sc_lv_base( const sc_dt::sc_lv_base& a );
  void unpack_sc_lv_base( sc_dt::sc_lv_base& a );

  void pack_sc_int_base( const sc_dt::sc_int_base& a );
  void unpack_sc_int_base( sc_dt::sc_int_base& a );

  void pack_sc_uint_base( const sc_dt::sc_uint_base& a );
  void unpack_sc_uint_base( sc_dt::sc_uint_base& a );

  void pack_sc_signed( const sc_dt::sc_signed& a );
  void unpack_sc_signed( sc_dt::sc_signed& a );

  void pack_sc_unsigned( const sc_dt::sc_unsigned& a );
  void unpack_sc_unsigned( sc_dt::sc_unsigned& a );

  void set_packed_size();
};

} // namespace uvm

#endif // UVM_PACKER_H_


