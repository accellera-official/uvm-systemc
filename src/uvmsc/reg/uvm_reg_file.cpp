//----------------------------------------------------------------------
//   Copyright 2013-2015 NXP B.V.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2010 Cadence Design Systems, Inc.
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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

#include "uvmsc/reg/uvm_reg_file.h"
#include "uvmsc/reg/uvm_reg_block.h"

namespace uvm {

//----------------------------------------------------------------------
// Group: Initialization
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Constructor
//
//! Create a new instance
//!
//! Creates an instance of a register file abstraction class
//! with the specified name.
//----------------------------------------------------------------------

uvm_reg_file::uvm_reg_file( const std::string& name ) : uvm_object(name)
{
  m_parent = nullptr;
  m_rf = nullptr;
  m_default_hdl_path = "RTL";
  m_hdl_paths_pool.clear();
}

//----------------------------------------------------------------------
// member function: configure
//
//! Configure a register file instance
//!
//! Specify the parent block and register file of the register file
//! instance.
//! If the register file is instantiated in a block,
//! \p regfile_parent is specified as nullptr.
//! If the register file is instantiated in a register file,
//! \p blk_parent must be the block parent of that register file and
//! \p regfile_parent is specified as that register file.
//!
//! If the register file corresponds to a hierarchical RTL structure,
//! it's contribution to the HDL path is specified as the \p hdl_path.
//! Otherwise, the register file does not correspond to a hierarchical RTL
//! structure (e.g. it is physically flattened) and does not contribute
//! to the hierarchical HDL path of any contained registers.
//----------------------------------------------------------------------

void uvm_reg_file::configure( uvm_reg_block* blk_parent,
                              uvm_reg_file* regfile_parent,
                              const std::string& hdl_path )
{
   m_parent = blk_parent;
   m_rf = regfile_parent;
   add_hdl_path(hdl_path);
}

//----------------------------------------------------------------------
// Group: Introspection
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: get_name
//
//! Get the simple name
//!
//! Return the simple object name of this register file.
//----------------------------------------------------------------------

// inherit from base class

//----------------------------------------------------------------------
// member function: get_full_name
//
//! Get the hierarchical name
//!
//! Return the hierarchical name of this register file.
//! The base of the hierarchical name is the root block.
//----------------------------------------------------------------------

std::string uvm_reg_file::get_full_name() const
{
  uvm_reg_block* blk;
  std::string name;

  name = get_name();

  // Do not include top-level name in full name
  if (m_rf != nullptr)
    return m_rf->get_full_name() + "." + name;

  // Do not include top-level name in full name
  blk = get_block();

  if (blk == nullptr)
    return name;

  if (blk->get_parent() == nullptr)
    return name;

  name = m_parent->get_full_name() + "." + name;

  return name;
}

//----------------------------------------------------------------------
// member function: get_parent
//
//! Get the parent block
//----------------------------------------------------------------------

uvm_reg_block* uvm_reg_file::get_parent() const
{
  return get_block();
}

//----------------------------------------------------------------------
// member function: get_regfile
//
//! Get the parent register file
//
//! Returns nullptr if this register file is instantiated in a block.
//----------------------------------------------------------------------

uvm_reg_file* uvm_reg_file::get_regfile() const
{
  return m_rf;
}

//----------------------------------------------------------------------
// Group: Backdoor
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function:  clear_hdl_path
//
//! Delete HDL paths
//!
//! Remove any previously specified HDL path to the register file instance
//! for the specified design abstraction.
//----------------------------------------------------------------------

void uvm_reg_file::clear_hdl_path( const std::string& kind )
{
  std::string lkind = kind;

  if (lkind == "ALL")
  {
    m_hdl_paths_pool.clear();
    return;
  }

  if (lkind.empty())
  {
    if (m_rf != nullptr)
      lkind = m_rf->get_default_hdl_path();
    else
      lkind = m_parent->get_default_hdl_path();
  }

  if (m_hdl_paths_pool.find(lkind) == m_hdl_paths_pool.end())
  {
    UVM_WARNING("RegModel", "Unknown HDL Abstraction '" + lkind + "'");
    return;
  }

  m_hdl_paths_pool.erase(lkind);
}

//----------------------------------------------------------------------
// member function: add_hdl_path
//
//! Add an HDL path
//!
//! Add the specified HDL path to the register file instance for the specified
//! design abstraction. This method may be called more than once for the
//! same design abstraction if the register file is physically duplicated
//! in the design abstraction
//----------------------------------------------------------------------

void uvm_reg_file::add_hdl_path( const std::string& path, const std::string& kind )
{
  std::vector<std::string> paths;

  if (m_hdl_paths_pool.find(kind) != m_hdl_paths_pool.end())
    paths = m_hdl_paths_pool.find(kind)->second;

  paths.push_back(path);
  m_hdl_paths_pool[kind] = paths;
}

//----------------------------------------------------------------------
// member function:   has_hdl_path
//
// Check if a HDL path is specified
//
//! Returns TRUE if the register file instance has a HDL path defined for the
//! specified design abstraction. If no design abstraction is specified,
//! uses the default design abstraction specified for the nearest
//! enclosing register file or block
//!
//! If no design abstraction is specified, the default design abstraction
//! for this register file is used.
//----------------------------------------------------------------------

bool uvm_reg_file::has_hdl_path( const std::string& kind ) const
{
  std::string lkind = kind;

  if (lkind.empty())
  {
    if (m_rf != nullptr)
      lkind = m_rf->get_default_hdl_path();
    else
      lkind = m_parent->get_default_hdl_path();
  }

  return (m_hdl_paths_pool.find(lkind) != m_hdl_paths_pool.end() );
}

//----------------------------------------------------------------------
// member function:  get_hdl_path
//
//! Get the incremental HDL path(s)
//!
//! Returns the HDL path(s) defined for the specified design abstraction
//! in the register file instance. If no design abstraction is specified, uses
//! the default design abstraction specified for the nearest enclosing
//! register file or block.
//! Returns only the component of the HDL paths that corresponds to
//! the register file, not a full hierarchical path
//!
//! If no design abstraction is specified, the default design abstraction
//! for this register file is used.
//----------------------------------------------------------------------

void uvm_reg_file::get_hdl_path( std::vector<std::string>& paths, const std::string& kind ) const
{
  std::string lkind = kind;

  if (lkind.empty())
  {
    if (m_rf != nullptr)
       lkind = m_rf->get_default_hdl_path();
    else
       lkind = m_parent->get_default_hdl_path();
  }

  if (!has_hdl_path(lkind))
  {
    UVM_ERROR("RegModel",
      "Register does not have HDL path defined for abstraction '" + lkind + "'");
    return;
  }

  if ( m_hdl_paths_pool.find(lkind) != m_hdl_paths_pool.end())
    paths = m_hdl_paths_pool.find(lkind)->second;
}

//----------------------------------------------------------------------
// member function:  get_full_hdl_path
//
//! Get the full hierarchical HDL path(s)
//!
//! Returns the full hierarchical HDL path(s) defined for the specified
//! design abstraction in the register file instance. If no design abstraction
//! is specified, uses the default design abstraction specified for the
//! nearest enclosing register file or block.
//! There may be more than one path returned even
//! if only one path was defined for the register file instance, if any of the
//! parent components have more than one path defined for the same design
//! abstraction
//!
//! If no design abstraction is specified, the default design abstraction
//! for each ancestor register file or block is used to get each
//! incremental path.
//----------------------------------------------------------------------

void uvm_reg_file::get_full_hdl_path( std::vector<std::string>& paths,
                                      std::string kind,
                                      const std::string& separator ) const
{
  if (kind.empty())
    kind = get_default_hdl_path();

  if (!has_hdl_path(kind))
  {
    UVM_ERROR("RegModel",
      "Register file does not have hdl path defined for abstraction '" + kind + "'");
    return;
  }

  std::vector<std::string> hdl_paths;

  if (m_hdl_paths_pool.find(kind) != m_hdl_paths_pool.end())
    hdl_paths = m_hdl_paths_pool.find(kind)->second;

  std::vector<std::string> parent_paths;

  if (m_rf != nullptr)
    m_rf->get_full_hdl_path(parent_paths, kind, separator);
  else
    if (m_parent != nullptr)
      m_parent->get_full_hdl_path(parent_paths, kind, separator);

  for ( unsigned int i = 0; i < hdl_paths.size(); i++ )
  {
    std::string hdl_path = hdl_paths[i];

    if (parent_paths.size() == 0)
    {
      if (!hdl_path.empty())
        paths.push_back(hdl_path);
      continue;
    }

    for( unsigned int j = 0; j < parent_paths.size(); j++ )
    {
      if (hdl_path.empty())
        paths.push_back(parent_paths[j]);
      else
        paths.push_back( parent_paths[j] + separator + hdl_path );
    }
  }
}

//----------------------------------------------------------------------
// member function: set_default_hdl_path
//
//! Set the default design abstraction
//!
//! Set the default design abstraction for this register file instance.
//----------------------------------------------------------------------

void uvm_reg_file::set_default_hdl_path(std::string kind)
{
  if (kind.empty())
  {
    if (m_rf != nullptr)
      kind = m_rf->get_default_hdl_path();
    else
      if (m_parent != nullptr)
        kind = m_parent->get_default_hdl_path();
      else
      {
        UVM_ERROR("RegModel", "Register file has no parent. Must specify a valid HDL abstraction (kind)");
        return;
      }
  }
  m_default_hdl_path = kind;
}

//----------------------------------------------------------------------
// member function:  get_default_hdl_path
//
//! Get the default design abstraction
//!
//! Returns the default design abstraction for this register file instance.
//! If a default design abstraction has not been explicitly set for this
//! register file instance, returns the default design abstraction for the
//! nearest register file or block ancestor.
//! Returns an empty string if no default design abstraction has been specified.
//----------------------------------------------------------------------


std::string uvm_reg_file::get_default_hdl_path() const
{
  if (m_default_hdl_path.empty())
  {
    if (m_rf != nullptr)
      return m_rf->get_default_hdl_path();
    else
      return m_parent->get_default_hdl_path();
  }
  return m_default_hdl_path;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// Destructor
//
// Implementation defined
// Clean-up allocated memory
//----------------------------------------------------------------------

uvm_reg_file::~uvm_reg_file()
{
  m_hdl_paths_pool.clear();
}

//----------------------------------------------------------------------
// member function: get_block
//
// Implementation defined
//----------------------------------------------------------------------

uvm_reg_block* uvm_reg_file::get_block() const
{
  return m_parent;
}


//----------------------------------------------------------------------
// member function: do_print
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_file::do_print( const uvm_printer& printer ) const
{
  uvm_object::do_print(printer);
}

//----------------------------------------------------------------------
// member function: convert2string
//
// Implementation defined
//----------------------------------------------------------------------

std::string uvm_reg_file::convert2string() const
{
  UVM_FATAL("RegModel","RegModel register files cannot be converted to strings");
  return "";
}

//----------------------------------------------------------------------
// member function: clone
//
// Implementation defined
//----------------------------------------------------------------------

uvm_object* uvm_reg_file::clone()
{
  UVM_FATAL("RegModel","RegModel register files cannot be cloned");
  return nullptr;
}

//----------------------------------------------------------------------
// member function: do_copy
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_file::do_copy( const uvm_object& rhs )
{
  UVM_FATAL("RegModel","RegModel register files cannot be copied");
  // TODO this should be managed by a disabled copy constructor
}

//----------------------------------------------------------------------
// member function: do_compare
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_reg_file::do_compare( const uvm_object& rhs,
                               const uvm_comparer* comparer ) const
{
  UVM_WARNING("RegModel","RegModel register files cannot be compared");
  return false;
}

//----------------------------------------------------------------------
// member function: do_pack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_file::do_pack( uvm_packer& packer ) const
{
  UVM_WARNING("RegModel","RegModel register files cannot be packed");
}


//----------------------------------------------------------------------
// member function: do_unpack
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_file::do_unpack( uvm_packer& packer )
{
  UVM_WARNING("RegModel","RegModel register files cannot be unpacked");
}



/////////////

} // namespace uvm
