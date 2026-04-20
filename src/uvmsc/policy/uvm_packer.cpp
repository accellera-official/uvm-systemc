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

#include <systemc>
#include <sstream>

#include "uvmsc/misc/uvm_status_container.h"
#include "uvmsc/base/uvm_object.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/policy/uvm_packer.h"
#include "uvmsc/misc/uvm_misc.h"
#include "uvmsc/macros/uvm_message_defines.h"

//using namespace sc_dt;

namespace uvm {

uvm_packer::uvm_packer()
{
  physical = true;
  byte_size = 8;  //set up bytesize for endianess
  word_size = 16; //set up worksize for endianess
  big_endian = false;
  use_metadata = false;
  abstract = false;
  policy = UVM_DEFAULT_POLICY;

  pack_index = 0;
  unpack_index = 0;

  m_bits = nullptr;
  m_size = 0;
  m_max_size = 0;
}

uvm_packer::~uvm_packer()
{
  if (m_bits != nullptr)
  {
    delete m_bits;
    m_bits = nullptr;
  }
}

//------------------------------------------------------------------------------
// Member function: pack_field (virtual)
//
//! Packs an integral value (less than or equal to 4096 bits) into the
//! packed array. Argument \p size is the number of bits of argument \p value
//! to pack.
//!
//! NOTE- max size limited to BITSTREAM bits parameter (default: 64)
//------------------------------------------------------------------------------

void uvm_packer::pack_field( const uvm_bitstream_t& value, int size )
{
  // TODO should be changed to 4096 size
  sc_dt::sc_bv<64> tmp = value;
  m_check_size(size);

  if(big_endian)
    for (int i = 0; i < size; i++)
      tmp[i] = value[size-1-i].to_bool();

  (*m_bits)(pack_index + size - 1, pack_index) = tmp.range(size-1, 0); // make sure we only copy the size
  pack_index += size;

  std::ostringstream str;
  str << "uvm_packer::pack_field_int 0x" << std::hex << tmp.to_uint64() << ", length = " << size << std::endl;
  str << "pack_index = " << pack_index << std::endl;
  UVM_INFO("PCKFLD", str.str(), uvm::UVM_DEBUG);
}

//------------------------------------------------------------------------------
// member function: pack_field_int (virtual)
//
//! Packs the integral value (less than or equal to 64 bits) into the
//! pack array.  The argument \p size is the number of bits to pack, usually obtained by
//! $bits. This optimized version of #pack_field is useful for sizes up
//! to 64 bits.
//------------------------------------------------------------------------------

void uvm_packer::pack_field_int( const uvm_integral_t& value, int size )
{
  sc_dt::sc_bv<64> tmp = value;
  m_check_size(size);

  if(big_endian)
    for (int i = 0; i < size; i++)
      tmp[i] = value[size-1-i].to_bool();

  (*m_bits)(pack_index + size - 1, pack_index) = tmp.range(size-1, 0); // make sure we only copy the size
  pack_index += size;

  std::ostringstream str;
  str << "uvm_packer::pack_field_int 0x" << std::hex << tmp.to_uint64() << ", length = " << size << std::endl;
  str << "pack_index = " << pack_index << std::endl;
  UVM_INFO("PCKFLDINT", str.str(), uvm::UVM_DEBUG);
}


//------------------------------------------------------------------------------
// Member function: pack_string
//
//! Packs a string value into the pack array.
//!
//! When the metadata flag is set, the packed string is terminated by a null
//! character to mark the end of the string.
//! This is useful for mixed language communication where unpacking may occur
//! outside of SystemVerilog UVM.
//------------------------------------------------------------------------------

void uvm_packer::pack_string( const std::string& value )
{
  int nchars = (int)value.length();

  for (int i = 0; i < nchars; i++)
  {
    pack_char(value[i]);
  }

  if(use_metadata)
    pack_char(0); // nullptr character

  std::ostringstream str;
  str << "uvm_packer::pack_string '" << value << "'" << std::endl;
  str << "pack_index = " << pack_index << std::endl;
  if(use_metadata) str << "use_metadata = true" << std::endl;
  UVM_INFO("PCKSTR", str.str(), uvm::UVM_DEBUG);
}

//------------------------------------------------------------------------------
// Member function: pack_time (virtual)
//
//! Packs a time value as 64 bits into the pack array.
//------------------------------------------------------------------------------

void uvm_packer::pack_time( const sc_core::sc_time& value )
{
  double v = value.to_seconds();
  pack_field_int(uvm_realtobits(v), 64);

  std::ostringstream str;
  str << "uvm_packer::pack_time" << v << std::endl;
  str << "pack_index = " << pack_index << std::endl;
  UVM_INFO("PCKTIME", str.str(), uvm::UVM_DEBUG);
}


//------------------------------------------------------------------------------
// Member function: pack_real (virtual)
//
//! Packs a real \p value as 64 bits into the pack array.
//!
//! The real \p value is converted to a 6-bit scalar value using the function
//! uvm_realtobits before it is packed into the array.
//------------------------------------------------------------------------------

void uvm_packer::pack_real( double value )
{
  pack_field_int(uvm_realtobits(value), 64);
}


//------------------------------------------------------------------------------
// Member function: pack_object (virtual)
//
//! Packs an object value into the pack array.
//
//! A 4-bit header is inserted ahead of the string to indicate the number of
//! bits that was packed. If a null object was packed, then this header will
//! be 0.
//!
//! This is useful for mixed-language communication where unpacking may occur
//! outside of SystemVerilog UVM.
//------------------------------------------------------------------------------

void uvm_packer::pack_object( const uvm_object& value )
{
  const uvm_object* val = &value;

  if( val->get_status_container()->cycle_check.find(val)
      != val->get_status_container()->cycle_check.end()) //exists
  {
    std::ostringstream str;
    str << "Cycle detected for object "
        << val->get_inst_id()
        << " during pack";
    UVM_WARNING("CYCFND", str.str());
    return;
  }

  val->get_status_container()->cycle_check[val] = true;

  if((policy != UVM_REFERENCE) && (val != nullptr) )
  {
    if(use_metadata)
    {
      m_check_size(4);
      m_bits->range(pack_index+3, pack_index) = 1;
      pack_index += 4; // to better debug when display packed bits in hexidecimal
    }
    scope.down(val->get_name());
    //value.__m_uvm_field_automation(null, UVM_PACK,""); // TODO do we need this?
    val->do_pack(*this);
    scope.up();
  }
  else
  {
    if(use_metadata)
    {
      m_check_size(4);
      m_bits->range(pack_index+3, pack_index) = 0; // nullptr object gets metadata 0
      pack_index += 4;
    }
  }
  val->get_status_container()->cycle_check.erase(val);
}

//------------------------------------------------------------------------------
// Member function: is_null
//
//! This member function is used during unpack operations to peek at the next 4-bit
//! chunk of the pack data and determine if it is 0.
//!
//! If the next four bits are all 0, then the return value is a 1; otherwise
//! it is 0.
//!
//! This is useful when unpacking objects, to decide whether a new object
//! needs to be allocated or not.
//------------------------------------------------------------------------------

bool uvm_packer::is_null()
{
  return (m_bits->range(unpack_index+3, unpack_index) == 0);
}


//------------------------------------------------------------------------------
// Member function: unpack_field_int (virtual)
//
//! Unpacks bits from the pack array and returns the bit-stream that was
//! unpacked.
//! Argument \p size is the number of bits to unpack; the maximum is 64 bits.
//! This is a more efficient variant than #unpack_field when unpacking into
//! smaller vectors.
//------------------------------------------------------------------------------

uvm_integral_t uvm_packer::unpack_field_int( int size )
{
  sc_dt::sc_bv<64> a, tmp;

  if (m_enough_bits(size, "integral"))
  {
    a = (*m_bits)(unpack_index + size - 1, unpack_index);

    inc_unpack_index(size);
    if(big_endian)
    {
      for (int i = 0; i < size; i++)
        tmp[i] = a[size-1-i];
      a = tmp; // content swapped
    }

    std::ostringstream str;
    str << "uvm_packer::unpack_field_int 0x" << std::hex << a.to_uint64() << ", length = " << size << std::endl;
    str << "unpack_index = " << unpack_index << std::endl;
    UVM_INFO("UNPCKFLDINT", str.str(), uvm::UVM_DEBUG);

    return a;
  }
  else
  {
    UVM_ERROR("UNPCKERR","Not enough bits in packed structure.");
    return 0;
  }
}


//------------------------------------------------------------------------------
// Member function: unpack_field (virtual)
//
//! Unpacks bits from the pack array and returns the bit-stream that was
//! unpacked. \p size is the number of bits to unpack; the maximum is 4096 bits.
//------------------------------------------------------------------------------

uvm_bitstream_t uvm_packer::unpack_field( int size )
{
  sc_dt::sc_bv<64> a, tmp; // TODO maximum to unpack is 4096

  if (m_enough_bits(size,"integral"))
  {
    a = (*m_bits)(unpack_index + size - 1, unpack_index);
    inc_unpack_index(size);
    if(big_endian)
    {
      for (int i = 0; i < size; i++)
        tmp[i] = a[size-1-i];
      a = tmp; // content swapped
    }

    std::ostringstream str;
    str << "uvm_packer::unpack_field 0x" << std::hex << a.to_uint64() << ", length = " << size << std::endl;
    str << "unpack_index = " << unpack_index << std::endl;
    UVM_INFO("UNPCKFLD", str.str(), uvm::UVM_DEBUG);

    return a;
  }
  else
  {
    UVM_ERROR("UNPCKERR","Not enough bits in packed structure.");
    return 0;
  }
}

//------------------------------------------------------------------------------
// Member function: unpack_string (virtual)
//
//! Unpacks a string.
//!
//! num_chars bytes are unpacked into a string. If num_chars is -1 then
//! unpacking stops on at the first null character that is encountered.
//------------------------------------------------------------------------------

// If num_chars is not -1, then the user only wants to unpack a
// specific number of bytes into the string.

std::string uvm_packer::unpack_string( int num_chars )
{
  sc_dt::sc_bv<8> b;
  bool is_null_term; // Assumes a NULL terminated string
  int i = 0;

  std::string s;

  if(num_chars == -1)
    is_null_term = true;
  else
    is_null_term = false;

  while( m_enough_bits(8, "string") &&
        (( (*m_bits).range(unpack_index+7, unpack_index) != 0 ) || (is_null_term == false)) &&
        ((i < num_chars) || (is_null_term == true)) )
  {
    s += " ";
    if(!big_endian)
      s[i] = (char)(*m_bits)(unpack_index+7, unpack_index).to_int();
    else
    {
      for(int j = 0; j < 8; ++j)
        b[7-j] = (*m_bits)[unpack_index+j].to_bool();
      s[i] = b.to_int();
    }

    inc_unpack_index(8);

    std::ostringstream str;
    str << "uvm_packer::unpack_char -> " << s[i] << std::endl;
    str << "unpack_index = " << unpack_index << std::endl;
    UVM_INFO("UNPCKSTR", str.str(), uvm::UVM_DEBUG);

    ++i;
  }

  if(use_metadata) // metadata adds null string at the end
  {
    if(m_enough_bits(8, "string"))
      inc_unpack_index(8);
  }

  std::ostringstream str;
  str << "uvm_packer::unpack_string '" << s << "'" << std::endl;
  str << "unpack_index = " << unpack_index << std::endl;
  UVM_INFO("UNPCKSTR", str.str(), uvm::UVM_DEBUG);

  return s;
}


//------------------------------------------------------------------------------
// Member function: unpack_time (virtual)
//
//! Unpacks the next 64 bits of the pack array and places them into a
//! time variable.
//! In case the time cannot be unpacked, return SC_ZERO_TIME
//------------------------------------------------------------------------------

sc_core::sc_time uvm_packer::unpack_time()
{
  sc_core::sc_time sct = sc_core::SC_ZERO_TIME;

  if (m_enough_bits(64, "time"))
  {
    double t = unpack_real();
    sct = sc_core::sc_time(t, sc_core::SC_SEC);
  }
  else
    uvm_report_error("UNPCKERR","Not able to unpack time.", UVM_NONE);

  std::ostringstream str;
  str << "uvm_packer::unpack_time" << sct << std::endl;
  str << "pack_index = " << pack_index << std::endl;
  UVM_INFO("UNPCKTIME", str.str(), uvm::UVM_DEBUG);

  return sct;
}


//------------------------------------------------------------------------------
// Member function: unpack_real (virtual)
//
//! Unpacks the next 64 bits of the pack array and places them into a
//! real variable.
//!
//! The 64 bits of packed data are converted to a real using the $bits2real
//! system function.
//------------------------------------------------------------------------------

double uvm_packer::unpack_real()
{
  if (m_enough_bits(64,"real"))
    return uvm_bitstoreal(unpack_field_int(64));

  UVM_ERROR("UNPCKERR","Not able to unpack real value.");
  return 0.0;
}


//------------------------------------------------------------------------------
// Member function: unpack_object (virtual)
//
//! Unpacks an object and stores the result into \p value.
//!
//! \p value must be an allocated object that has enough space for the data
//! being unpacked. The first four bits of packed data are used to determine
//! if a null object was packed into the array.
//!
//! The #is_null function can be used to peek at the next four bits in
//! the pack array before calling this method.
//------------------------------------------------------------------------------

void uvm_packer::unpack_object( uvm_object& value )
{
  int is_non_null = 1;
  uvm_object* val = &value;

  if( val->get_status_container()->cycle_check.find(val)
      != val->get_status_container()->cycle_check.end()) //exists
  {
    std::ostringstream str;
    str << "Cycle detected for object "
        << val->get_inst_id()
        << " during unpack.";
    UVM_WARNING("CYCFND", str.str());
    return;
  }
  val->get_status_container()->cycle_check[val] = true;

  if(use_metadata)
  {
    is_non_null = (*m_bits).range(unpack_index+3, unpack_index).to_int();
    inc_unpack_index(4);
  }

  // NOTE- policy is a ~pack~ policy, not unpack policy;
  //       and you can't pack an object by REFERENCE

  if (val != nullptr)
  {
    if (is_non_null > 0)
    {
      scope.down(val->get_name());
      //value.__m_uvm_field_automation(null, UVM_UNPACK,"");
      val->do_unpack(*this);
      scope.up();
    }
    else
    {
      // TODO: help do_unpack know whether unpacked result would be null
      //       to avoid new'ing unnecessarily;
      //       this does not nullify argument; need to pass obj by ref
    }
  }
  else
    if ((is_non_null != 0) && (val == nullptr))
      UVM_ERROR("UNPCKERR","Can not unpack into nullptr object.");

  val->get_status_container()->cycle_check.erase(val);
}


//------------------------------------------------------------------------------
// Member function: get_packed_size (virtual)
//
//! Returns the number of bits that were packed.
//------------------------------------------------------------------------------

unsigned int uvm_packer::get_packed_size() const
{
  return pack_index;
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
///      Implementation-defined member functions start here           //
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

uvm_packer& uvm_packer::operator<< ( bool a )
{
  pack_bool(a);
  return *this;
}

uvm_packer& uvm_packer::operator>> ( bool& a )
{
  unpack_bool(a);
  return *this;
}

uvm_packer& uvm_packer::operator<< ( double a )
{
  pack_real(a);
  return *this;
}

uvm_packer& uvm_packer::operator>> ( double& a )
{
  a = unpack_real();
  return *this;
}

uvm_packer& uvm_packer::operator<< ( float a )
{
  pack_real(a);
  return *this;
}

uvm_packer& uvm_packer::operator>> ( float& a )
{
  a = unpack_real();
  return *this;
}

uvm_packer& uvm_packer::operator<< ( const sc_core::sc_time& a )
{
  pack_time(a);
  return *this;
}

uvm_packer& uvm_packer::operator>> ( sc_core::sc_time& a )
{
  a = unpack_time();
  return *this;
}

#define UVM_PACK_UNPACK_OPERATORS(T) \
  uvm_packer& uvm_packer::operator<< ( T a ) \
  { \
    sc_dt::sc_bv<64> a_ = a; \
    pack_field_int(a_, 8*sizeof(T));\
    return *this;\
  } \
  uvm_packer& uvm_packer::operator>> ( T& a ) \
  { \
    a = (T)(unpack_field_int(8*sizeof(T))); \
    return *this; \
  }

UVM_PACK_UNPACK_OPERATORS( char )
UVM_PACK_UNPACK_OPERATORS( unsigned char )
UVM_PACK_UNPACK_OPERATORS( short )
UVM_PACK_UNPACK_OPERATORS( unsigned short  )
UVM_PACK_UNPACK_OPERATORS( int )
UVM_PACK_UNPACK_OPERATORS( unsigned int  )
UVM_PACK_UNPACK_OPERATORS( long )
UVM_PACK_UNPACK_OPERATORS( unsigned long )
UVM_PACK_UNPACK_OPERATORS( long long )
UVM_PACK_UNPACK_OPERATORS( unsigned long long )

uvm_packer& uvm_packer::operator<< ( const char* a )
{
  if (!a ) {
    pack_string("");
  } else {
    pack_string(std::string(a));
  }
  return *this;
}

uvm_packer& uvm_packer::operator<< ( const std::string& a )
{
  pack_string(a);
  return *this;
}

uvm_packer& uvm_packer::operator>> ( std::string& a )
{
  a = unpack_string();
  return *this;
}

uvm_packer& uvm_packer::operator<< ( const uvm_object& a )
{
  pack_object(a);
  return *this;
}

uvm_packer& uvm_packer::operator>> ( uvm_object& a )
{
  unpack_object(a);
  return *this;
}


#define UVM_PACK_UNPACK_SC_OPERATORS(T1, T2) \
  uvm_packer& uvm_packer::operator<< ( const T1& a ) \
  { \
    pack_##T2(a);\
    return *this;\
  } \
  uvm_packer& uvm_packer::operator>> ( T1& a ) \
  {  \
    unpack_##T2(a); \
    return *this; \
  }

UVM_PACK_UNPACK_SC_OPERATORS( sc_dt::sc_logic, sc_logic )
UVM_PACK_UNPACK_SC_OPERATORS( sc_dt::sc_bv_base, sc_bv_base )
UVM_PACK_UNPACK_SC_OPERATORS( sc_dt::sc_lv_base, sc_lv_base )
UVM_PACK_UNPACK_SC_OPERATORS( sc_dt::sc_int_base, sc_int_base )
UVM_PACK_UNPACK_SC_OPERATORS( sc_dt::sc_uint_base, sc_uint_base )
UVM_PACK_UNPACK_SC_OPERATORS( sc_dt::sc_signed, sc_signed )
UVM_PACK_UNPACK_SC_OPERATORS( sc_dt::sc_unsigned, sc_unsigned )


//------------------------------------------------------------------------------
// member function: get_bits
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::get_bits( std::vector<bool>& bits ) const
{
  unsigned int n = get_remaining_unpacked_bits();
  for (unsigned int i = 0; i < n; i++)
  {
    bool b = (*m_bits)[i].to_bool();
    bits.push_back(b);
  }
}




//------------------------------------------------------------------------------
// member function: get_bytes
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::get_bytes( std::vector<unsigned char>& bytes ) const
{
  unsigned int n = get_remaining_unpacked_bits();
  unsigned int nbytes = 1 + (n-1)/8;

  for (unsigned int i = 0; i < nbytes; i++)
  {
    char b = (*m_bits).range(8*i+7,8*i).to_int();

    if(big_endian)
    {
      sc_dt::sc_bv<8> tmp, tmp2;
      tmp = b;
      for(int j = 0; j < 8; ++j) tmp2[j] = tmp[7-j];
      b = tmp2.to_int();
    }

    bytes.push_back(b);
  }
}


//------------------------------------------------------------------------------
// member function: get_ints
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::get_ints( std::vector<unsigned int>& ints ) const
{
  unsigned int n = get_remaining_unpacked_bits();
  unsigned int nints = 1 + (n-1)/32;

  for (unsigned int i = 0; i < nints; i++)
  {
    int b = (*m_bits).range(32*i+31, 32*i).to_int();

    if(big_endian)
    {
      sc_dt::sc_bv<32> tmp, tmp2;
      tmp = b;
      for(int j = 0; j < 32; ++j) tmp2[j] = tmp[31-j];
      b = tmp2.to_int();
    }

    ints.push_back(b);
  }
}

//------------------------------------------------------------------------------
// member function: put_bits
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::put_bits( const std::vector<bool>& bitstream )
{
  reset();
  int bit_size = (int)bitstream.size();
  m_set_size(bit_size);

  if(!big_endian)
    for (int i = 0; i < bit_size; i++)
    {
      (*m_bits)[i] = bitstream[i];
    }
  else
    for (int i = bit_size-1; i >= 0; i--)
    {
      (*m_bits)[i] = bitstream[i];
    }

  pack_index = bit_size;
}


//------------------------------------------------------------------------------
// member function: put_bytes
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::put_bytes( const std::vector<unsigned char>& bytestream )
{
  reset();
  int byte_size = (int)bytestream.size();
  m_set_size(byte_size*8);

  for (int i = 0; i < byte_size; i++)
  {
    sc_dt::sc_bv<8> b = bytestream[i];
    if(big_endian)
    {
      sc_dt::sc_bv<8> tmp = b;
      for(int j = 0; j < 8; ++j)
        b[j] = tmp[7-j]; // swap char first
    }
    (*m_bits).range(8*i+7,8*i) = b;
  }
  pack_index = 8*byte_size;
}


//------------------------------------------------------------------------------
// member function: put_ints
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::put_ints( const std::vector<unsigned int>& intstream )
{
  reset();
  int int_size = (int)intstream.size();
  m_set_size(int_size*32);

  for (int i = 0; i < int_size; i++)
  {
    sc_dt::sc_bv<32> v = intstream[i];
    if(big_endian)
    {
      sc_dt::sc_bv<32> tmp = v;
      for(int j = 0; j < 32; ++j)
        v[j] = tmp[31-j]; // swap int first
    }

    (*m_bits).range(32*i+31, 32*i) = v;
  }
  pack_index = 32*int_size;
}


//------------------------------------------------------------------------------
// Member function: reset
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::reset()
{
  pack_index = 0;
  unpack_index = 0;
  m_size = 0;
}


//------------------------------------------------------------------------------
// Member function: enough_bits
//
//! Implementation defined
//------------------------------------------------------------------------------

bool uvm_packer::m_enough_bits( int needed, std::string id ) const
{
  if ((int)(pack_index - unpack_index) < needed)
  {
   std::ostringstream str;
   str << needed
       << " bits needed to unpack "
       << id
       << ", yet only "
       << pack_index - unpack_index //m_packed_size - count
       << " available.";
    UVM_ERROR("PCKSZERR", str.str());
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------
// Member function: m_set_size
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::m_set_size( int nbits )
{
  // check that we are in a "fresh" packer, e.g. one that has been reset
#ifdef _NCSC_DEBUG
  assert(size == 0);
  assert(pack_index == 0);
#endif
  // no need to copy over from old sc_bv_base
  m_allocate(nbits, false);
}


//------------------------------------------------------------------------------
// Member function: m_allocate
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::m_allocate( int nbits, bool copy_ )
{
  // allocate new sc_bv_base with new size, and copy over contents from
  // old sc_bv_base

  int total_bits = pack_index + nbits;
  int new_size =
      ((total_bits - 1) / UVM_PACKING_BLOCK_SIZE + 1) * UVM_PACKING_BLOCK_SIZE;
#ifdef _NCSC_DEBUG
  assert((new_size % UVM_PACKING_BLOCK_SIZE) == 0);
#endif
  m_size = new_size;

  if (m_size > m_max_size)
  { // need to allocate
    m_max_size = m_size;
    sc_dt::sc_bv_base* new_bv = new sc_dt::sc_bv_base(m_size);
    if (m_bits)
    {
      if (copy_)
      {
        *new_bv = *m_bits;
      }
      delete m_bits;
    }
    m_bits = new_bv;
  }
}


//------------------------------------------------------------------------------
// Member function: get_remaining_unpacked_bits
//
//! Implementation defined
//------------------------------------------------------------------------------

unsigned int uvm_packer::get_remaining_unpacked_bits() const
{
  return pack_index - unpack_index;
}


//------------------------------------------------------------------------------
// Member function: m_check_size
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::m_check_size( int nbits )
{
  int capacity = m_size - pack_index;
  if (nbits >= capacity)
  {
    m_allocate(nbits);
  }
}

//------------------------------------------------------------------------------
// Member function: inc_unpack_index
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::inc_unpack_index( int n )
{
  unpack_index += n;
  if (unpack_index > pack_index)
  {
    std::ostringstream msg;
    msg << "unpack_index > pack_index. "
        << "unpack_index = " << unpack_index
        << " pack_index = " << pack_index;
    UVM_ERROR("UNPCKERR", msg.str());
  }
}

//------------------------------------------------------------------------------
// Member function: pack_char
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::pack_char( char a )
{
  int nbits = 8;
  m_check_size(nbits);
  sc_dt::sc_bv<8> b, c;
  b = a;

  if (big_endian)
  {
    for (int i = 0; i < 8; i++)
      c[i] = b[7-i];
  }
  else
    c = b;

  (*m_bits)(pack_index + nbits - 1, pack_index) = c;
  pack_index += nbits;

  std::string s;
  if (c.to_int())
    s = (char)c.to_int();
  else
    s = "\0";

  std::ostringstream str;
  str << "uvm_packer::pack_char -> " << s << std::endl;
  str << "pack_index = " << pack_index << std::endl;
  UVM_INFO("PCKCHR", str.str(), uvm::UVM_DEBUG);
}


//------------------------------------------------------------------------------
// Member function: unpack_char
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::unpack_char( char& a )
{
  unsigned nbits = 8;
  sc_dt::sc_bv<8> b, c;
  b = (*m_bits)(unpack_index + nbits - 1, unpack_index);

  inc_unpack_index(nbits);

  if (big_endian)
  {
    for (int i = 0; i < 8; i++)
      c[i] = b[7-i];
  }
  else c = b;

  a = c.to_int();

  std::ostringstream str;
  str << "uvm_packer::unpack_char " << c.to_int() << std::endl;
  str << "unpack_index = " << unpack_index << std::endl;
  UVM_INFO("UNPCKCHR", str.str(), uvm::UVM_DEBUG);
}


void uvm_packer::pack_bool( bool a )
{
  m_check_size(1);

  (*m_bits)[pack_index] = a ? 1 : 0;
  pack_index += 1;

  std::ostringstream str;
  str << "uvm_packer::pack_bool: " << (a ? 1 : 0) << std::endl;
  str << "pack_index = " << pack_index << std::endl;
  UVM_INFO("PCKBOOL", str.str(), uvm::UVM_DEBUG);
}



void uvm_packer::unpack_bool( bool& a )
{
  a = (*m_bits)[unpack_index] == 1;

  inc_unpack_index(1);

  std::ostringstream str;
  str << "uvm_packer::unpack_char " << (a ? 1 : 0) << std::endl;
  str << "unpack_index = " << unpack_index << std::endl;
  UVM_INFO("UNPCKBOOL", str.str(), uvm::UVM_DEBUG);

}

void uvm_packer::pack_sc_logic( const sc_dt::sc_logic& a )
{
  m_check_size(1);
  (*m_bits).set_bit(pack_index, a.value());
  pack_index++;
}

void uvm_packer::unpack_sc_logic( sc_dt::sc_logic& a )
{
  sc_dt::sc_logic_value_t val = sc_dt::sc_logic_value_t((*m_bits).get_bit(unpack_index));
  a = val;
  inc_unpack_index(1);
}

void uvm_packer::pack_sc_bv_base( const sc_dt::sc_bv_base& a )
{
  int nbits = a.length();
  m_check_size(nbits);
  (*m_bits)(pack_index + nbits - 1, pack_index) = a;
  pack_index += nbits;
}

void uvm_packer::unpack_sc_bv_base( sc_dt::sc_bv_base& a )
{
  int nbits = a.length();
  a = (*m_bits)(unpack_index + nbits - 1, unpack_index);
  inc_unpack_index(nbits);
}

void uvm_packer::pack_sc_lv_base( const sc_dt::sc_lv_base& a )
{
  int n = a.length();
  m_check_size(n);
  for (int i = 0; i < n; i++)
  {
    sc_dt::sc_logic_value_t val = a.get_bit(i);
    (*m_bits).set_bit(pack_index, val);
    pack_index++;
  }
}

void uvm_packer::unpack_sc_lv_base( sc_dt::sc_lv_base& a )
{
  int n = a.length();
  for (int i = 0; i < n; i++)
  {
    sc_dt::sc_logic_value_t val = sc_dt::sc_logic_value_t((*m_bits).get_bit(unpack_index));
    a.set_bit(i, val);
    unpack_index++;
  }
}

void uvm_packer::pack_sc_int_base( const sc_dt::sc_int_base& a )
{
  int n = a.length();
  m_check_size(n);
  for (int i = 0; i < n; i++)
  {
    bool val = a.test(i);
    if (val) {
      (*m_bits).set_bit(pack_index, sc_dt::Log_1);
    } else {
      (*m_bits).set_bit(pack_index, sc_dt::Log_0);
    }
    pack_index++;
  }
}

void uvm_packer::unpack_sc_int_base( sc_dt::sc_int_base& a )
{
  int n = a.length();
  for (int i = 0; i < n; i++)
  {
    sc_dt::sc_logic_value_t val = sc_dt::sc_logic_value_t((*m_bits).get_bit(unpack_index));
    if (val == sc_dt::Log_1)
    {
      a.set(i, true);
    } else {
      a.set(i, false);
    }
    unpack_index++;
  }
}

void uvm_packer::pack_sc_uint_base( const sc_dt::sc_uint_base& a )
{
  int n = a.length();
  m_check_size(n);
  for (int i = 0; i < n; i++)
  {
    bool val = a.test(i);
    if (val)
    {
      (*m_bits).set_bit(pack_index, sc_dt::Log_1);
    } else {
      (*m_bits).set_bit(pack_index, sc_dt::Log_0);
    }
    pack_index++;
  }
}

void uvm_packer::unpack_sc_uint_base( sc_dt::sc_uint_base& a )
{
  int n = a.length();
  for (int i = 0; i < n; i++)
  {
    sc_dt::sc_logic_value_t val = sc_dt::sc_logic_value_t((*m_bits).get_bit(unpack_index));
    if (val == sc_dt::Log_1)
    {
      a.set(i, true);
    } else {
      a.set(i, false);
    }
    unpack_index++;
  }
}

void uvm_packer::pack_sc_signed( const sc_dt::sc_signed& a )
{
  int n = a.length();
  m_check_size(n);
  for (int i = 0; i < n; i++) {
    bool val = a.test(i);
    if (val) {
      (*m_bits).set_bit(pack_index, sc_dt::Log_1);
    } else {
      (*m_bits).set_bit(pack_index, sc_dt::Log_0);
    }
    pack_index++;
  }
}

void uvm_packer::unpack_sc_signed( sc_dt::sc_signed& a )
{
  int n = a.length();
  for (int i = 0; i < n; i++)
  {
    sc_dt::sc_logic_value_t val = sc_dt::sc_logic_value_t((*m_bits).get_bit(unpack_index));
    if (val == sc_dt::Log_1)
    {
      a.set(i, true);
    } else {
      a.set(i, false);
    }
    unpack_index++;
  }
}

void uvm_packer::pack_sc_unsigned( const sc_dt::sc_unsigned& a )
{
  int n = a.length();
  m_check_size(n);
  for (int i = 0; i < n; i++)
  {
    bool val = a.test(i);
    if (val)
    {
      (*m_bits).set_bit(pack_index, sc_dt::Log_1);
    } else {
      (*m_bits).set_bit(pack_index, sc_dt::Log_0);
    }
    pack_index++;
  }
}

void uvm_packer::unpack_sc_unsigned( sc_dt::sc_unsigned& a )
{
  int n = a.length();
  for (int i = 0; i < n; i++)
  {
    sc_dt::sc_logic_value_t val = sc_dt::sc_logic_value_t((*m_bits).get_bit(unpack_index));
    if (val == sc_dt::Log_1)
    {
      a.set(i, true);
    } else {
      a.set(i, false);
    }
    unpack_index++;
  }
}


//------------------------------------------------------------------------------
// Member function: set_packed_size
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_packer::set_packed_size()
{
  // TODO remove?
  //packed_index = count;
  //pack_index = 0;
}


////////////////

} // namespace uvm
