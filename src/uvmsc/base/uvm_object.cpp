//----------------------------------------------------------------------------
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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
//----------------------------------------------------------------------------

#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/base/uvm_object.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/factory/uvm_factory.h"
#include "uvmsc/misc/uvm_status_container.h"
#include "uvmsc/print/uvm_printer.h"
#include "uvmsc/print/uvm_table_printer.h"
#include "uvmsc/print/uvm_tree_printer.h"
#include "uvmsc/print/uvm_printer_globals.h"
#include "uvmsc/policy/uvm_packer.h"
#include "uvmsc/policy/uvm_recorder.h"
#include "uvmsc/report/uvm_report_object.h"

namespace uvm {


// global for instance count
int g_inst_count = 0;

//----------------------------------------------------------------------------
// Class implementation: uvm_object
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// initialization of static data members
//----------------------------------------------------------------------------

uvm_status_container* uvm_object::__m_uvm_status_container = nullptr;

//----------------------------------------------------------------------------
// initialization of external members
//----------------------------------------------------------------------------

uvm_packer* uvm_default_packer = uvm_object::get_uvm_packer();

//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

uvm_object::uvm_object()
{
  m_leaf_name = sc_core::sc_gen_unique_name( "object" );
  m_full_name = m_leaf_name; // initial value
  m_inst_id = g_inst_count++;

  // make only one status container
  if (uvm_object::__m_uvm_status_container == nullptr)
    __m_uvm_status_container = new uvm_status_container();

  // register callbacks
  m_register_cb();
}

uvm_object::uvm_object( uvm_object_name name)
{
  m_leaf_name = name;
  m_inst_id = g_inst_count++;

  // make only one status container
  if (uvm_object::__m_uvm_status_container == nullptr)
    __m_uvm_status_container = new uvm_status_container();

  // register callbacks
  m_register_cb();
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------

uvm_object::~uvm_object()
{}

//----------------------------------------------------------------------------
// member function: set name (virtual)
//
//! Sets the instance name of this object, overwriting any previously
//! given name.
//----------------------------------------------------------------------------

void uvm_object::set_name(const std::string& name)
{
  m_leaf_name = name;
}


//----------------------------------------------------------------------------
// member function: get_name (virtual)
//
//! Returns the name of the object, as provided by the ~name~ argument in the
//! constructor or <set_name> method.
//----------------------------------------------------------------------------

const std::string uvm_object::get_name() const
{
  return m_leaf_name;
}

//----------------------------------------------------------------------------
// member function: get_full_name (virtual)
//
//! Returns the full hierarchical name of this object.
//----------------------------------------------------------------------------

const std::string uvm_object::get_full_name() const
{
  return get_name();  // TODO not the full name..
}

//----------------------------------------------------------------------------
// member function: get_inst_id (virtual)
//
//! Returns the object's unique, numeric instance identifier.
//----------------------------------------------------------------------------

int uvm_object::get_inst_id() const
{
  return m_inst_id;
}

//----------------------------------------------------------------------------
// member function: get_inst_count (static)
//
//! Returns the current value of the instance counter, which represents the
//! total number of uvm_object-based objects that have been allocated in
//! simulation. The instance counter is used to form a unique numeric instance
//! identifier.
//----------------------------------------------------------------------------

int uvm_object::get_inst_count()
{
  return g_inst_count;
}

//----------------------------------------------------------------------------
// member function: get_type (static)
//
//! Returns the type-proxy (wrapper) for this object. The <uvm_factory>'s
//! type-based override and creation methods take arguments of
//! <uvm_object_wrapper>. This method, if implemented, can be used as convenient
//! means of supplying those arguments.
//!
//! The default implementation of this method produces an error and returns
//! nullptr. To enable use of this method, a user's subtype must implement a
//! version that returns the subtype's wrapper.
//----------------------------------------------------------------------------

const uvm_object_wrapper* uvm_object::get_type()
{
  uvm_report_error("NOTYPID", "Member function get_type() not implemented in derived class.", UVM_NONE);
  return nullptr;
}

//----------------------------------------------------------------------------
// member function: get_object_type (virtual)
//
//! Returns the type-proxy (wrapper) for this object. The uvm_factory
//! type-based override and creation methods take arguments of
//! #uvm_object_wrapper. This method, if implemented, can be used as convenient
//! means of supplying those arguments. This method is the same as the static
//! #get_type method, but uses an already allocated object to determine
//! the type-proxy to access (instead of using the static object).
//!
//! The default implementation of this method does a factory lookup of the
//! proxy using the return value from #get_type_name. If the type returned
//! by #get_type_name is not registered with the factory, then a null
//! handle is returned.
//----------------------------------------------------------------------------

const uvm_object_wrapper* uvm_object::get_object_type() const
{
  if( get_type_name() == "<unknown>" )
    return nullptr;

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_factory* factory = cs->get_factory();

  return factory->find_wrapper_by_name(get_type_name());
}

//----------------------------------------------------------------------------
// member function: get_type_name (virtual)
//
//! This function returns the type name of the object, which is typically the
//! type identifier enclosed in quotes. It is used for various debugging
//! functions in the library, and it is used by the factory for creating
//! objects.
//----------------------------------------------------------------------------

const std::string uvm_object::get_type_name() const
{
  return "<unknown>";
}

//----------------------------------------------------------------------------
// Group: Creation
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: create (virtual)
//
//! The create method allocates a new object of the same type as this object
//! and returns it via a base uvm_object handle. Every class deriving from
//! uvm_object, directly or indirectly, must implement the create method.
//! This member function should be overloaded by the application.
//----------------------------------------------------------------------------

uvm_object* uvm_object::create( const std::string& name )
{
  std::ostringstream str;
  str << "Member function create() of object '"
      << get_name() << "' not overloaded by the application.";
  uvm_report_error("NO_OVERRIDE", str.str(), UVM_NONE);
  return nullptr;
}

//----------------------------------------------------------------------------
// member function: clone
//
//! The clone method creates and returns an exact copy of this object.
//! The default implementation calls #create followed by #copy. As clone is
//! virtual, derived classes may override this implementation if desired.
//----------------------------------------------------------------------------

uvm_object* uvm_object::clone()
{
  uvm_object* obj = this->create(get_name());
  if(obj == nullptr)
  {
    std::ostringstream str;
    str <<  "The member function create failed for object '" << get_name() << "', object cannot be cloned.";
    uvm_report_warning("CRFLD", str.str(), UVM_NONE);
  }
  else
    obj->copy(*this);
  return(obj);
}

//----------------------------------------------------------------------------
// Group: Printing
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// member function: print
//
//! The print method deep-prints this object's properties in a format and
//! manner governed by the given argument \p printer; if the argument \p printer
//! is not provided, the global <uvm_default_printer> is used.
//----------------------------------------------------------------------------

void uvm_object::print( uvm_printer* printer ) const
{
  if (printer == nullptr)
    printer = uvm_default_printer;

  if (printer == nullptr)
    uvm_report_error("nullptrPRINTER","uvm_default_printer is nullptr");

  //$fwrite(printer.knobs.mcd,); // TODO printer knobs?
  std::string s = sprint(printer);
  std::cout << s;
}

//----------------------------------------------------------------------------
// member function: sprint
//
//! The sprint method works just like the <print> method, except the output
//! is returned in a string rather than displayed.
//----------------------------------------------------------------------------

std::string uvm_object::sprint( uvm_printer* printer ) const
{
  if( printer == nullptr )
    printer = uvm_default_printer;

  // not at top-level, must be recursing into sub-object
  if(!printer->istop())
  {
    __m_uvm_status_container->printer = printer;
    //__m_uvm_field_automation(null, UVM_PRINT, ""); // TODO do we need this?
    do_print( *printer );
    return "";
  }

  printer->print_object(get_name(), *this);
  // backward compat with sprint knob: if used,
  //    print that, do not call emit()
  if (!printer->m_string.empty())
    return printer->m_string;

  return printer->emit();
}

//----------------------------------------------------------------------------
// member function: do_print (virtual)
//
//! The do_print method is the user-definable hook called by #print and
//! #sprint that allows users to customize what gets printed or sprinted
//! beyond the field information provided by the `uvm_field_* macros,
//! <Utility and Field Macros for Components and Objects>.
//----------------------------------------------------------------------------

void uvm_object::do_print( const uvm_printer& printer ) const
{
  uvm_report_warning("DOPRNT", "member function do_print not implemented in " + get_type_name(), UVM_HIGH);
  return;
}

//----------------------------------------------------------------------------
// member function: convert2string (virtual)
//
//! This virtual function is a user-definable hook, called directly by the
//! user, that allows users to provide object information in the form of
//! a string. Unlike <sprint>, there is no requirement to use an <uvm_printer>
//! policy object. As such, the format and content of the output is fully
//! customizable, which may be suitable for applications not requiring the
//! consistent formatting offered by the <print>/<sprint>/<do_print>
//! API.
//----------------------------------------------------------------------------

std::string uvm_object::convert2string() const
{
  return "";
}

//----------------------------------------------------------------------------
// Group: Recording
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: record
//
//! The record method deep-records this object's properties according to an
//! optional ~recorder~ policy. The method is not virtual and must not be
//! overloaded. To include additional fields in the record operation, derived
//! classes should override the member function #do_record.
//!
//! The optional ~recorder~ argument specifies the recording policy, which
//! governs how recording takes place. If a recorder policy is not provided
//!explicitly, then the global #uvm_default_recorder policy is used. See
//! uvm_recorder for information.
//!
//! A simulator's recording mechanism is vendor-specific. By providing access
//! via a common interface, the uvm_recorder policy provides vendor-independent
//! access to a simulator's recording capabilities.
//----------------------------------------------------------------------------

void uvm_object::record( uvm_recorder* recorder )
{
  if(recorder == nullptr)
    recorder = new uvm_recorder();

  if(!recorder->tr_handle)
    return;

  __m_uvm_status_container->recorder = recorder;

  recorder->recording_depth++;
  //__m_uvm_field_automation(null, UVM_RECORD, ""); // TODO field automation
  do_record(*recorder);

  recorder->recording_depth--;

  if(recorder->recording_depth == 0)
  {
    recorder->tr_handle = 0;
  }
}

//----------------------------------------------------------------------------
// member function: do_record (virtual)
//
//! The do_record method is the user-definable hook called by the <record>
//! method. A derived class should override this method to include its fields
//! in a record operation.
//!
//! The ~recorder~ argument is policy object for recording this object. A
//! do_record implementation should call the appropriate recorder methods for
//! each of its fields. Vendor-specific recording implementations are
//! encapsulated in the \p recorder policy, thereby insulating user-code from
//! vendor-specific behavior. See #uvm_recorder for more information.
//----------------------------------------------------------------------------

void uvm_object::do_record( const uvm_recorder& recorder )
{
  return;
}

//----------------------------------------------------------------------------
// Group: Copying
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: copy
//
//! The copy makes this object a copy of the specified object.
//!
//! The copy method is not virtual and should not be overloaded in derived
//! classes. To copy the fields of a derived class, that class should override
//! the #do_copy method.
//----------------------------------------------------------------------------

void uvm_object::copy( const uvm_object& rhs )
{
  do_copy(rhs);
}

//----------------------------------------------------------------------------
// member function: do_copy (virtual)
//
//! The do_copy method is the user-definable hook called by the copy method.
//! A derived class should override this method to include its fields in a copy
//! operation.
//----------------------------------------------------------------------------

void uvm_object::do_copy( const uvm_object& rhs )
{
  uvm_report_warning("DOCPY", "member function do_copy not implemented in " + get_type_name(), UVM_HIGH);
}

//----------------------------------------------------------------------------
// Group: Comparing
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: compare
//
//! Deep compares members of this data object with those of the object provided
//! in the ~rhs~ (right-hand side) argument, returning 1 on a match, 0 othewise.
//!
//! The compare method is not virtual and should not be overloaded in derived
//! classes. To compare the fields of a derived class, that class should
//! override the <do_compare> method.
//----------------------------------------------------------------------------

bool uvm_object::compare( const uvm_object& rhs,
                          const uvm_comparer* comparer ) const
{
  return do_compare(rhs, (comparer==nullptr)?::uvm::uvm_default_comparer:comparer);
}


//----------------------------------------------------------------------------
// member function: do_compare (virtual)
//
//! The do_compare method is the user-definable hook called by the <compare>
//! method. A derived class should override this method to include its fields
//! in a compare operation. It should return 1 if the comparison succeeds, 0
//! otherwise.
//----------------------------------------------------------------------------

bool uvm_object::do_compare( const uvm_object& rhs) const
{
  uvm_report_warning("DOCOMP", "member function do_compare not implemented in " + get_type_name(), UVM_HIGH);
  return true;
}

bool uvm_object::do_compare( const uvm_object& rhs,
                             const uvm_comparer* comparer ) const
{
  uvm_report_warning("DOCOMP", "member function do_compare with comparer argument not implemented in " + get_type_name(), UVM_HIGH);
  return true;
}

//----------------------------------------------------------------------------
// Group: Packing
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: m_pack
//
//! internal member function
//----------------------------------------------------------------------------

void uvm_object::m_pack( uvm_packer*& packer )
{
  if( packer != nullptr)
    __m_uvm_status_container->packer = packer;
  else
    __m_uvm_status_container->packer = uvm_default_packer;

  packer = __m_uvm_status_container->packer;

  packer->reset();
  packer->scope.down(get_name());

  //   __m_uvm_field_automation(null, UVM_PACK, ""); // TODO do we need this?
  do_pack(*packer);
  packer->set_packed_size();

  packer->scope.up();

}

//----------------------------------------------------------------------------
// member function: pack
//
//! internal member function
//----------------------------------------------------------------------------

int uvm_object::pack( std::vector<bool>& bitstream, uvm_packer* packer )
{
  m_pack(packer);
  packer->get_bits(bitstream);
  return packer->get_packed_size();
}

//----------------------------------------------------------------------------
// member function: pack_bytes
//
//! internal member function
//----------------------------------------------------------------------------

int uvm_object::pack_bytes( std::vector<unsigned char>& bytestream, uvm_packer* packer )
{
  m_pack(packer);
  packer->get_bytes(bytestream);
  return packer->get_packed_size();
}

//----------------------------------------------------------------------------
// member function: pack_ints
//
//! The pack methods bitwise-concatenate this object's properties into an array
//! of bits, bytes, or ints. The methods are not virtual and must not be
//! overloaded. To include additional fields in the pack operation, derived
//! classes shall override the member function #do_pack.
//----------------------------------------------------------------------------

int uvm_object::pack_ints( std::vector<unsigned int>& intstream, uvm_packer* packer )
{
  m_pack(packer);
  packer->get_ints(intstream);
  return packer->get_packed_size();
}

//----------------------------------------------------------------------------
// member function: do_pack (virtual)
//
//! The do_pack method is the user-definable hook called by the pack methods.
//! A derived class shall override this method to include its fields in a pack
//! operation.
//----------------------------------------------------------------------------

void uvm_object::do_pack( uvm_packer& packer ) const
{
  uvm_report_warning("DOPCK", "member function do_pack not implemented in " + get_type_name(), UVM_HIGH);
}

//----------------------------------------------------------------------------
// Group: Unpacking
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: m_unpack_pre
//
//! Internal member function
//! Initialize packer
//----------------------------------------------------------------------------

void uvm_object::m_unpack_pre( uvm_packer*& packer )
{
  if( packer != nullptr)
    __m_uvm_status_container->packer = packer;
  else
    __m_uvm_status_container->packer = uvm_default_packer;

  packer = __m_uvm_status_container->packer;

  packer->reset();
}


//----------------------------------------------------------------------------
// member function: m_unpack_post
//
//! Internal member function
//! The member function unpack shall extract property values from an array
//----------------------------------------------------------------------------

void uvm_object::m_unpack_post( uvm_packer*& packer )
{
  unsigned int provided_size;

  provided_size = packer->get_packed_size();

  //Put this object into the hierarchy
  packer->scope.down(get_name());

  // TODO do we need field automation??
  //__m_uvm_field_automation(null, UVM_UNPACK, "");

  do_unpack(*packer);

  //Scope back up before leaving
  packer->scope.up();

  if(packer->get_packed_size() != provided_size)
  {
    std::ostringstream str;
    str << "Unpack operation unsuccessful: unpacked "
        << packer->get_packed_size()
        << " bits from a total of "
        << provided_size
        << " bits.";
    uvm_report_warning("BDUNPK", str.str(), UVM_NONE);
  }
}

//----------------------------------------------------------------------------
// member function: unpack
//
//! The member function unpack_bits shall extract property values from an array
//! of bits. The method of unpacking ~must~ exactly correspond to the method of
//! packing. This is assured if (a) the same ~packer~ policy is used to pack
//! and unpack, and (b) the order of unpacking is the same as the order of
//! packing used to create the input array.
//----------------------------------------------------------------------------

int uvm_object::unpack( const std::vector<bool>& bitstream, uvm_packer* packer )
{
  m_unpack_pre(packer);
  packer->put_bits(bitstream);
  m_unpack_post(packer);
  packer->set_packed_size();
  return packer->get_packed_size();
}

//----------------------------------------------------------------------------
// member function: unpack_bytes
//
//! The member function unpack_bytes shall extract property values from an array
//! of bytes. The method of unpacking ~must~ exactly correspond to the method of
//! packing. This is assured if (a) the same ~packer~ policy is used to pack
//! and unpack, and (b) the order of unpacking is the same as the order of
//! packing used to create the input array.
//----------------------------------------------------------------------------

int uvm_object::unpack_bytes( const std::vector<unsigned char>& bytestream, uvm_packer* packer )
{
  m_unpack_pre(packer);
  packer->put_bytes(bytestream);
  m_unpack_post(packer);
  packer->set_packed_size();
  return packer->get_packed_size();}

//----------------------------------------------------------------------------
// member function: unpack_ints
//
//! The member function unpack_ints shall extract property values from an array
//! of ints. The method of unpacking ~must~ exactly correspond to the method of
//! packing. This is assured if (a) the same ~packer~ policy is used to pack
//! and unpack, and (b) the order of unpacking is the same as the order of
//! packing used to create the input array.
//----------------------------------------------------------------------------

int uvm_object::unpack_ints( const std::vector<unsigned int>& intstream, uvm_packer* packer )
{
  m_unpack_pre(packer);
  packer->put_ints(intstream);
  m_unpack_post(packer);
  packer->set_packed_size();
  return packer->get_packed_size();
}

//----------------------------------------------------------------------------
// member function: do_unpack (virtual)
//
//! The do_unpack method is the user-definable hook called by the <unpack>
//! method. A derived class should override this method to include its fields
//! in an unpack operation.
//----------------------------------------------------------------------------

void uvm_object::do_unpack( uvm_packer& packer )
{
  uvm_report_warning("DOUNPCK", "member function do_unpack not implemented in " + get_type_name(), UVM_HIGH);
}

//----------------------------------------------------------------------------
// Group: Configuration
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: set_int_local
//
//! The member function set_int_local shall provide write access to
//! integer properties indexed by a ~field_name~ string. The object
//! designer choose which, if any, properties will be accessible, and overrides
//! the appropriate methods depending on the properties' types.
//----------------------------------------------------------------------------

/* TODO
void uvm_object::set_int_local( const std::string& field_name,
                                int value,             //TODO make uvm_bitstream_t ?
                                bool recurse )
{}
*/

//----------------------------------------------------------------------------
// member function: set_string_local
//
//! The member function set_string_local shall provide write access to
//! string properties indexed by a ~field_name~ string. The object
//! designer choose which, if any, properties will be accessible, and overrides
//! the appropriate methods depending on the properties' types.
//----------------------------------------------------------------------------

/* TODO
void uvm_object::set_string_local( const std::string& field_name,
                                   const std::string& value,
                                   bool recurse )
{}
*/

//----------------------------------------------------------------------------
// member function: set_object_local
//
//! The member function set_object_local shall provide write access to an
//! uvm_object-based properties indexed by a ~field_name~ string. The object
//! designer choose which, if any, properties will be accessible, and overrides
//! the appropriate methods depending on the properties' types. For objects,
//! the optional ~clone~ argument specifies whether to clone the ~value~
//! argument before assignment.
//----------------------------------------------------------------------------

/* TODO
void uvm_object::set_object_local( const std::string& field_name,
                                   uvm_object* value,
                                   bool clone,
                                   bool recurse )
{
}
*/

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
///      Implementation-defined member functions start here           //
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const uvm_object& obj ) {
  uvm_object* mobj = const_cast<uvm_object*>(&obj);
  uvm_printer* prt = uvm_default_tree_printer;
  os << mobj->sprint(prt);
  return os;
}

std::ostream& operator<<( std::ostream& os, const uvm_object* obj )
{
  uvm_object* mobj = const_cast<uvm_object*>(obj);
  uvm_printer* prt = uvm_default_tree_printer;
  os << mobj->sprint(prt);
  return os;
}

/* TODO object comparison operators
bool operator==( const uvm_object& a, const uvm_object& b )
{
  return ( ( a.m_leaf_name == b.m_leaf_name ) &&
           ( a.m_inst_id == b.m_inst_id ) );
}

bool operator!=( const uvm_object& a, const uvm_object& b )
{
  return ( ( a.m_leaf_name != b.m_leaf_name ) ||
           ( a.m_inst_id != b.m_inst_id ) );
}
*/

//----------------------------------------------------------------------------
// member function: m_register_cb
//
//! Implementation defined
//! Hook to call the UVM_REGISTER_CB macro to register the callback.
//! This member function will be overloaded by the actual component
//! implementation
//----------------------------------------------------------------------------

bool uvm_object::m_register_cb()
{
  return false;
}

//----------------------------------------------------------------------------
// member function: get_uvm_packer
//
//! Implementation defined
//! Helper method to create a packer singleton as part of the uvm_object
//----------------------------------------------------------------------------

uvm_packer* uvm_object::get_uvm_packer()
{
  static uvm_packer* p = new uvm_packer;
  return p;
}


///////////

} // namespace uvm
