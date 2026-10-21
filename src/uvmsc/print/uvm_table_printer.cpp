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

#include "uvmsc/print/uvm_table_printer.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
//////////////

namespace uvm {

//------------------------------------------------------------------------------
// Class implementation: uvm_table_printer
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Constructor
//
//! Creates a new instance of #uvm_table_printer
//------------------------------------------------------------------------------

uvm_table_printer::uvm_table_printer()
{
  m_max_name = 4;
  m_max_type = 4;
  m_max_size = 4;
  m_max_value = 5;
}

//------------------------------------------------------------------------------
// member function: emit
//
//! Formats the collected information from prior calls to print_*
//! into table format.
//------------------------------------------------------------------------------

std::string uvm_table_printer::emit()
{
  std::string s;
  std::string user_format;
  std::string dash = "---------------------------------------------------------------------------------------------------";
  std::string space= "                                                                                                   ";
  std::string dashes;

  std::string linefeed = '\n'+ knobs.prefix;

  calculate_max_widths();

  if (knobs.header)
  {
    std::string header;
    user_format = format_header();
    if (user_format.empty())
    {
      std::string dash_id, dash_typ, dash_sz;
      std::string head_id, head_typ, head_sz;

      if (knobs.identifier)
      {
        dashes = dash.substr(1,m_max_name+2);
        header = "Name" + space.substr(1,m_max_name-2);
      }
      if (knobs.type_name)
      {
        dashes = dashes + dash.substr(1,m_max_type+2);
        header = header + "Type" + space.substr(1,m_max_type-2);
      }
      if (knobs.size)
      {
        dashes = dashes + dash.substr(1,m_max_size+2);
        header = header + "Size" + space.substr(1,m_max_size-2);
      }
      dashes = dashes + dash.substr(1,m_max_value) + linefeed;
      header = header + "Value" + space.substr(1,m_max_value-5) + linefeed;

      s = s + dashes + header + dashes;
    }
    else
      s = s + user_format + linefeed;
  } // header

  for( m_row_vecItT it = m_rows.begin(); it < m_rows.end(); it++ )
  {
    uvm_printer_row_info row = (*it);
    user_format = format_row(row);

    if (user_format.empty())
    {
      std::string row_str;
      if (knobs.identifier)
        row_str = space.substr(1,row.level * knobs.indent) + row.name +
                  space.substr(1,m_max_name-row.name.length()-(row.level*knobs.indent)+2);
      if (knobs.type_name)
        row_str = row_str + row.type_name + space.substr(1,m_max_type-row.type_name.length()+2);
      if (knobs.size)
        row_str = row_str + row.size + space.substr(1,m_max_size-row.size.length()+2);
      // value
      s = s + row_str + row.val + linefeed; // space.substr(1,m_max_value-row.val.length())
    }
    else
      s = s + user_format + linefeed;
  } // rows

  if (knobs.footer)
  {
    user_format = format_footer();
    if (user_format.empty())
      s = s + dashes;
    else
      s = s + user_format + linefeed;
  } // footer
  /* */
  m_rows.clear(); // flush content
  return s;
}

//------------------------------------------------------------------------------
// member function: get_default
//
//! Returns the implementation-defined default table printer.
//------------------------------------------------------------------------------

std::shared_ptr<uvm_table_printer> uvm_table_printer::get_default()
{
  return std::shared_ptr<uvm_table_printer>(uvm_coreservice_t::get()->get_uvm_default_table_printer(), [](uvm_table_printer*){});
}

//------------------------------------------------------------------------------
// member function: set_default
//
//! Sets the implementation-defined default table printer.
//------------------------------------------------------------------------------

void uvm_table_printer::set_default(std::shared_ptr<uvm_table_printer> printer)
{
  uvm_coreservice_t::get()->set_uvm_default_table_printer(printer.get());
}

//------------------------------------------------------------------------------
// member function: calculate_max_widths
//
//! Implementation defined
//------------------------------------------------------------------------------

void uvm_table_printer::calculate_max_widths()
{
  for( m_row_vecItT it = m_rows.begin(); it < m_rows.end(); it++ )
  {
    unsigned int name_len;

    uvm_printer_row_info row = (*it);

    name_len = knobs.indent * row.level + (int)row.name.length();

    if (name_len > m_max_name)
      m_max_name = name_len;

    if (row.type_name.length() > m_max_type)
      m_max_type = (int)row.type_name.length();

    if (row.size.length() > m_max_size)
      m_max_size = (int)row.size.length();

    if (row.val.length() > m_max_value)
      m_max_value = (int)row.val.length();
  }

  if (m_max_name < 4) m_max_name = 4;
  if (m_max_type < 4) m_max_type = 4;
  if (m_max_size < 4) m_max_size = 4;
  if (m_max_value< 5) m_max_value= 5;
}

//////////////

} // namespace uvm
