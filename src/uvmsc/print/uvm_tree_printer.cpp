//----------------------------------------------------------------------
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2012-2014 NXP B.V.
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

#include <systemc>

#include "uvmsc/print/uvm_tree_printer.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"

//////////////

namespace uvm {

//------------------------------------------------------------------------------
// Class implementation: uvm_tree_printer
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Constructor
//
//! Creates a new instance of #uvm_tree_printer
//------------------------------------------------------------------------------

uvm_tree_printer::uvm_tree_printer()
{
  knobs.size = 0;
  knobs.type_name = 0;
  knobs.header = 0;
  knobs.footer = 0;
  newline = "\n";
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------

uvm_tree_printer::~uvm_tree_printer()
{
}

//------------------------------------------------------------------------------
// member function: emit
//
//! Formats the collected information from prior calls to print_*
//! into table format.
//------------------------------------------------------------------------------

std::string uvm_tree_printer::emit()
{
  std::string s = knobs.prefix;
  std::string space= "                                                                                                   ";
  std::string user_format;

  std::string linefeed = (newline.empty() || newline == " ") ? newline : (newline + knobs.prefix);

  std::string separator_start;
  std::string separator_end;

  // make sure we access the separator characters only for a valid string length
  if (knobs.separator.size() > 1)
  {
    separator_start = knobs.separator[0];
    separator_end = knobs.separator[1];
  }

  // Header
  if (knobs.header)
  {
    user_format = format_header();
    if (!user_format.empty())
      s = s + user_format + linefeed;
  }

  for( unsigned int i = 0; i < m_rows.size(); i++ )
  {
    uvm_printer_row_info row = m_rows[i];
    user_format = format_row(row);
    if (user_format.empty())
    {
      std::string indent_str;
      indent_str = space.substr(1,row.level * knobs.indent);

      // Name (id)
      if (knobs.identifier)
      {
        s = s + indent_str + row.name;
        if (!row.name.empty() && row.name != "...")
          s = s + ": ";
      }

      // Type Name
      if (row.val[0] == '@') // is an object w/ knobs.reference on
        s = s + "(" + row.type_name + row.val + ") ";
      else
        if (knobs.type_name &&
             (!row.type_name.empty() ||
              row.type_name != "-" ||
              row.type_name != "..."))
          s = s + "(" + row.type_name + ") ";

      // Size
      if (knobs.size)
      {
        if (!row.size.empty() || row.size != "-")
            s = s + "(" + row.size + ") ";
      }

      if ( i < m_rows.size()-1 )
      {
        if (m_rows[i+1].level > row.level)
        {
          s = s + separator_start + linefeed;
          continue;
        }
      }

      // Value (unconditional)
      s = s + row.val + " " + linefeed;

      // Scope handling...
      if (i <= m_rows.size()-1)
      {
        int end_level;
        if (i == m_rows.size()-1)
          end_level = 0;
        else
          end_level = m_rows[i+1].level;

        if (end_level < row.level)
        {
          std::string indent_str;
          for (int l = row.level-1; l >= end_level; l--)
          {
            indent_str = space.substr(1,l * knobs.indent);
            s = s + indent_str + separator_end + linefeed;
          }
        }
      }

    }
    else
      s = s + user_format;
  }

  // Footer
  if (knobs.footer)
  {
    user_format = format_footer();
    if (!user_format.empty())
      s = s + user_format + linefeed;
  }

  if (newline.empty() || newline == " ")
    s = s + '\n';

  m_rows.clear();
  return s;
}


//////////////

//------------------------------------------------------------------------------
// member function: get_default
//
//! Returns the implementation-defined default tree printer.
//------------------------------------------------------------------------------

std::shared_ptr<uvm_tree_printer> uvm_tree_printer::get_default()
{
  return std::shared_ptr<uvm_tree_printer>(uvm_coreservice_t::get()->get_uvm_default_tree_printer(), [](uvm_tree_printer*){});
}

//------------------------------------------------------------------------------
// member function: set_default
//
//! Sets the implementation-defined default tree printer.
//------------------------------------------------------------------------------

void uvm_tree_printer::set_default(std::shared_ptr<uvm_tree_printer> printer)
{
  uvm_coreservice_t::get()->set_uvm_default_tree_printer(printer.get());
}

} // namespace uvm
