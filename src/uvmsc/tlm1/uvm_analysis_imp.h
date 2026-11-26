//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
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

#ifndef UVM_ANALYSIS_IMP_H_
#define UVM_ANALYSIS_IMP_H_

#include <systemc>
#include <tlm>
#include <string>

namespace uvm {

//----------------------------------------------------------------------
// Class: uvm_analysis_imp
//
//! Receives all transactions broadcasted by a #uvm_analysis_port. It serves as
//! the termination point of an analysis port/export/imp connection. The component
//! attached to the imp class --called a subscriber-- implements the analysis
//! interface.
//!
//! Will invoke the write() method in the parent component.
//! The implementation of the write() method must not modify
//! the value passed to it.
//----------------------------------------------------------------------

template <typename T = int, typename IMP = int>
class uvm_analysis_imp : public tlm::tlm_analysis_port<T>
{
 public:
  explicit uvm_analysis_imp( const std::string& name, IMP* imp )
  : tlm::tlm_analysis_port<T>( name.c_str() ),
    m_imp(imp)
  {}

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_analysis_imp";
  }

  virtual void connect( tlm::tlm_analysis_if<T>& _if )
  {
    this->bind( _if );
  }

  void write( const T &t )
  {
    m_imp->write(t);
  }

  virtual ~uvm_analysis_imp(){}

 private:
  IMP* m_imp;

}; // class uvm_analysis_imp


/////////////////////////////////////////////

} /* namespace uvm */

#endif /* UVM_ANALYSIS_IMP_H_ */
