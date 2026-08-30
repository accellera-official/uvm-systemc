//----------------------------------------------------------------------
//   Copyright 2019 COSEDA Technologies GmbH
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2012-2021 NXP B.V.
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

#include <sstream>

#include "base/uvm_coreservice_t.h"
#include "base/uvm_default_coreservice_t.h"
#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_object.h"
#include "uvmsc/print/uvm_printer.h"
#include "uvmsc/misc/uvm_status_container.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/misc/uvm_misc.h"
#include "uvmsc/misc/uvm_scope_stack.h"

namespace uvm {

//----------------------------------------------------------------------
// Class implementation: uvm_printer
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: get_default
//
// Helper method for setting the default printer policy 
// instance via uvm_coreservice_t::set_default_printer 
//
//----------------------------------------------------------------------
std::shared_ptr<uvm_printer> uvm_printer::get_default() {
    return uvm_coreservice_t::get()->get_default_printer();
};

//----------------------------------------------------------------------
// member function: set_default
// 
// Helper method for retrieving the default printer policy 
// instance via uvm_coreservice_t::get_default_printer
//
//----------------------------------------------------------------------
void uvm_printer::set_default(std::shared_ptr<uvm_printer> printer){
    uvm_coreservice_t::get()->set_default_printer(printer);
};


//----------------------------------------------------------------------
// member function: print_field
//
//! Prints an integral field (of maximum default size of 4096 bits)
//!
//! \p name  - The name of the field.
//! \p value - The value of the field.
//! \p size  - The number of bits of the field (maximum is 4096).
//! \p radix - The radix to use for printing. The printer knob for radix is used
//!           if no radix is specified.
//! \p scope_separator - is used to find the leaf name since many printers only
//!           print the leaf name of a field.  Typical values for the separator
//!           are . (dot) or [ (open bracket).
//----------------------------------------------------------------------

void uvm_printer::print_field( const std::string& name,
                               const uvm_bitstream_t& value,
                               int size,
                               uvm_radix_enum radix,
                               const char* scope_separator,
                               const std::string& type_name ) const
{
  uvm_printer_row_info row_info;
  std::string sz_str, val_str;
  std::string loc_type_name = type_name;

  if(!name.empty())
  {
    m_scope.set_arg(name);
    //name = m_scope.get(); // not possible now to return name
  }

  if(loc_type_name.empty())
  {
    if(radix == UVM_TIME)
      loc_type_name ="time";
    else if(radix == UVM_STRING)
      loc_type_name ="string";
    else
      loc_type_name ="int";
  }

  if (size == -1)
    size = value.length();

  std::ostringstream str;
  str << size;
  sz_str = str.str(); //itoa

  if(radix == UVM_NORADIX)
    radix = knobs.default_radix;

  val_str = uvm_vector_to_string( value, size, radix,
                                  knobs.get_radix_str(radix));

  row_info.level = m_scope.depth();
  row_info.name = adjust_name(m_scope.get(), scope_separator);
  row_info.type_name = loc_type_name;
  row_info.size = sz_str;
  row_info.val = val_str;

  m_rows.push_back(row_info);
}

//----------------------------------------------------------------------
// member function: print_field_int
//
//! Prints an integral field (up to 64 bits)
//!
//! \p name  - The name of the field.
//! \p value - The value of the field.
//! \p size  - The number of bits of the field (maximum is 64).
//! \p radix - The radix to use for printing. The printer knob for radix is used
//!           if no radix is specified.
//! \p scope_separator - is used to find the leaf name since many printers only
//!           print the leaf name of a field.  Typical values for the separator
//!           are . (dot) or [ (open bracket).
//----------------------------------------------------------------------

void uvm_printer::print_field_int( const std::string& name,
                                   const uvm_integral_t& value,
                                   int size,
                                   uvm_radix_enum radix,
                                   const char* scope_separator,
                                   const std::string& type_name ) const
{
  uvm_printer_row_info row_info;
  std::string sz_str, val_str;
  std::string loc_type_name = type_name;

  if(!name.empty())
  {
    m_scope.set_arg(name);
    //name = m_scope.get(); // TODO - not possible now to return name
  }

  if(loc_type_name.empty())
  {
    if(radix == UVM_TIME)
      loc_type_name ="time";
    else if(radix == UVM_STRING)
      loc_type_name ="string";
    else
      loc_type_name ="int";
  }

  if (size == -1)
    size = value.length();

  std::ostringstream str;
  str << size;
  sz_str = str.str(); //itoa

  if(radix == UVM_NORADIX)
    radix = knobs.default_radix;

  val_str = uvm_vector_to_string( value, size, radix,
                                  knobs.get_radix_str(radix));

  row_info.level = m_scope.depth();
  row_info.name = adjust_name(m_scope.get(), scope_separator);
  row_info.type_name = loc_type_name;
  row_info.size = sz_str;
  row_info.val = val_str;

  m_rows.push_back(row_info);
}


//----------------------------------------------------------------------
// member function: print_real
//
//! Prints a real-value field.
//----------------------------------------------------------------------

void uvm_printer::print_real( const std::string& name,
                              double value,
                              const char* scope_separator ) const
{
  uvm_printer_row_info row_info;

  if (!name.empty() && name != "...")
  {
    m_scope.set_arg(name);
    //name = m_scope.get(); // no need to return changed name
  }

  std::ostringstream str_size;
  str_size << sizeof(value)*CHAR_BIT;

  std::ostringstream str_val;
  str_val << value;

  row_info.level = m_scope.depth();
  row_info.name = adjust_name(m_scope.get(), scope_separator);
  row_info.type_name = "double";
  row_info.size = str_size.str();
  row_info.val = str_val.str();

  m_rows.push_back(row_info);
}

//----------------------------------------------------------------------
// member function: print_double
//
//! Prints a real-value field.
//----------------------------------------------------------------------

void uvm_printer::print_double( const std::string& name,
                              double value,
                              const char* scope_separator ) const

{
  print_real(name, value, scope_separator);
}

//----------------------------------------------------------------------
// member function: print_object
//
//! Prints the object
//----------------------------------------------------------------------

void uvm_printer::print_object( const std::string& name,
                                const uvm_object& obj,
                                const char* scope_separator ) const
{
  const uvm_component* comp = nullptr;
  const uvm_component* child_comp = nullptr;

  const uvm_object* objp = &obj;

  print_object_header(name, obj, scope_separator);

  if( (knobs.depth == -1 || (knobs.depth > m_scope.depth())) &&
        (objp->get_status_container()->cycle_check.find(objp) == objp->get_status_container()->cycle_check.end()) // not exists
    )
  {
    objp->get_status_container()->cycle_check[objp] = true;
    if(name.empty() && objp != nullptr)
      m_scope.down(objp->get_name());
    else
      m_scope.down(name);

    // Handle children of the comp
    comp = dynamic_cast<const uvm_component*>(objp);
    if(comp != nullptr)
    {
      std::string lname;
      if (comp->get_first_child(lname))
        do
        {
          child_comp = comp->get_child(lname);
          if(child_comp->print_enabled)
            this->print_object("", *child_comp);
        }
        while (comp->get_next_child(lname));
    }

    // print members of object
    objp->sprint(const_cast<uvm_printer*>(this));

    if( !name.empty() && name[0] == '[' )
      m_scope.up("[");
    else
      m_scope.up(".");

    objp->get_status_container()->cycle_check.erase( objp->get_status_container()->cycle_check.find(objp) );
  }
}

//----------------------------------------------------------------------
// member function: print_object_header
//
//! Print the object header
//----------------------------------------------------------------------

void uvm_printer::print_object_header( const std::string& name,
                                       const uvm_object& obj,
                                       const char* scope_separator ) const
{
  uvm_printer_row_info row_info;
  const uvm_component* comp;
  std::string lname = name;

  const uvm_object* objp = &obj;

  if(lname.empty())
  {
    if(objp != nullptr )
    {
      comp = dynamic_cast<const uvm_component*>(objp);
      if( (m_scope.depth() == 0) && (comp != nullptr) )
        lname = comp->get_full_name();
      else
        lname = objp->get_name();
    }
  }

  if(lname.empty())
    lname = "<unnamed>";

  m_scope.set_arg(lname);
  row_info.level = m_scope.depth();

  if(row_info.level == 0 && knobs.show_root == true)
    row_info.name = objp->get_full_name();
  else
    row_info.name = adjust_name(m_scope.get(), scope_separator);

  row_info.type_name = (objp != nullptr) ?  objp->get_type_name() : "object";
  row_info.size = "-";
  row_info.val = knobs.reference ? uvm_object_value_str(objp) : "-";

  m_rows.push_back(row_info);
}

//----------------------------------------------------------------------
// member function: print_string
//
//! Prints a string field.
//----------------------------------------------------------------------

void uvm_printer::print_string( const std::string& name,
                                const std::string& value,
                                const char* scope_separator ) const
{
  uvm_printer_row_info row_info;
  std::string lname = name;

  if(!name.empty())
    m_scope.set_arg(name);

  row_info.level = m_scope.depth();
  row_info.name = adjust_name(m_scope.get(),scope_separator);
  row_info.type_name = "string";

  std::ostringstream str;
  str << value.length();

  row_info.size = str.str();
  row_info.val = (value.empty() ? "\"\"" : value);

  m_rows.push_back(row_info);
}

//----------------------------------------------------------------------
// member function: print_time
//
//! Prints a time value. name is the name of the field, and value is the
//! value to print.
//----------------------------------------------------------------------

void uvm_printer::print_time( const std::string& name,
                              const sc_core::sc_time& value,
                              const char* scope_separator ) const
{
  double t = value.to_seconds();
  print_real(name, t, scope_separator); // TODO print as string??
}

//----------------------------------------------------------------------
// member function: print_generic
//
//! Prints a field having the given \p name, \p type_name, \p size, and \p value.
//----------------------------------------------------------------------

void uvm_printer::print_generic( const std::string& name,
                                 const std::string& type_name,
                                 int size,
                                 const std::string& value,
                                 const char* scope_separator ) const
{
  uvm_printer_row_info row_info;
  std::string lname = name;

  if (!name.empty() && name != "...")
  {
    m_scope.set_arg(name);
    lname = m_scope.get();
  }

  std::ostringstream str;
  str << size;
  row_info.level = m_scope.depth();
  row_info.name = adjust_name(lname, scope_separator);
  row_info.type_name = type_name;
  row_info.size = (size == -2 ? "..." : str.str());
  row_info.val = (value.empty() ? "\"\"" : value);

  m_rows.push_back(row_info);
}

//----------------------------------------------------------------------
// member function: emit
//
//! Emits a string representing the contents of an object
//! in a format defined by an extension of this object.
//----------------------------------------------------------------------

std::string uvm_printer::emit()
{
  uvm_report_error("NO_OVERRIDE","emit() method not overridden in printer subtype");
  return "";
}

//----------------------------------------------------------------------
// member function: format_row
//
//! Hook for producing custom output of a single field (row).
//----------------------------------------------------------------------

std::string uvm_printer::format_row( const uvm_printer_row_info& row )
{
  return "";
}

//----------------------------------------------------------------------
// member function: format_header
//
//! Hook to override base header with a custom header.
//----------------------------------------------------------------------

std::string uvm_printer::format_header()
{
  return "";
}

//----------------------------------------------------------------------
// member function: format_footer
//
//! Hook to override base footer with a custom footer.
//----------------------------------------------------------------------

std::string uvm_printer::format_footer()
{
  return "";
}

//----------------------------------------------------------------------
// member function: adjust_name
//
//! Prints a field's name, or \p id, which is the full instance name.
//! The intent of the separator is to mark where the leaf name starts if the
//! printer if configured to print only the leaf name of the identifier.
//----------------------------------------------------------------------

std::string uvm_printer::adjust_name( const std::string& id,
                                 const char* scope_separator ) const
{
  if ( (knobs.show_root && (m_scope.depth() == 0 )) || knobs.full_name || id == "...")
    return id;
  return uvm_leaf_scope(id, scope_separator);
}

//----------------------------------------------------------------------
// member function: print_array_header
//
//! Prints the header of an array. This function is called before each
//! individual element is printed. print_array_footer is called to mark the
//! completion of array printing.
//----------------------------------------------------------------------

void uvm_printer::print_array_header( const std::string& name,
                                      int    size,
                                      const std::string& arraytype,
                                      const char* scope_separator ) const
{
  uvm_printer_row_info row_info;

  if(!name.empty())
    m_scope.set_arg(name);

  row_info.level = m_scope.depth();
  row_info.name = adjust_name(m_scope.get(),scope_separator);
  row_info.type_name = arraytype;

  std::ostringstream str;
  str << size;

  row_info.size = str.str();
  row_info.val = "-";

  m_rows.push_back(row_info);

  m_scope.down(name);
  const_cast<uvm_printer *>(this)->m_array_stack.push_back(true);
}

//----------------------------------------------------------------------
// member function: print_array_range
//
//! Prints a range using ellipses for values. This method is used when honoring
//! the array knobs for partial printing of large arrays,
//! uvm_printer_knobs::begin_elements and uvm_printer_knobs::end_elements.
//!
//! This function should be called after #begin_elements have been printed
//! and before #end_elements have been printed.
//----------------------------------------------------------------------

void uvm_printer::print_array_range( int min, int max ) const
{
  std::string tmpstr;
  if(min == -1 && max == -1)
     return;
  if(min == -1)
     min = max;
  if(max == -1)
     max = min;
  if(max < min)
     return;
  print_generic(std::string("..."), std::string("..."), -2, std::string("..."));
}

//----------------------------------------------------------------------
// member function: print_array_footer
//
//! Prints the header of a footer. This function marks the end of an array
//! print. Generally, there is no output associated with the array footer, but
//! this method lets the printer know that the array printing is complete.
//----------------------------------------------------------------------

void uvm_printer::print_array_footer( int size ) const
{
  if(m_array_stack.size())
  {
    m_scope.up();
    const_cast<uvm_printer *>(this)->m_array_stack.pop_front();
  }
}


////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

uvm_printer::uvm_printer()
{
  uvm_scope_stack m_scope;
}

//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------

uvm_printer::~uvm_printer()
{
}

//----------------------------------------------------------------------
// Utility methods - Implementation defined
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// member function: istop
//----------------------------------------------------------------------

bool uvm_printer::istop() const
{
  return (m_scope.depth() == 0);
}

} /* namespace uvm */
