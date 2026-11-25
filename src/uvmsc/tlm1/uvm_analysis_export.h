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

#ifndef UVM_ANALYSIS_EXPORT_H_
#define UVM_ANALYSIS_EXPORT_H_

#include <systemc>
#include <tlm>
#include <string>

namespace uvm {

//----------------------------------------------------------------------
// CLASS: uvm_analysis_export
//
//! The class uvm_analysis_export shall export a lower-level
//! #uvm_analysis_imp to its parent.
//----------------------------------------------------------------------

template <typename T>
class uvm_analysis_export : public sc_core::sc_export< tlm::tlm_analysis_if<T> >
{
 public:
  uvm_analysis_export() : sc_core::sc_export< tlm::tlm_analysis_if<T> >() {}
  uvm_analysis_export( const std::string& name ) : sc_core::sc_export< tlm::tlm_analysis_if<T> >( name.c_str() ) {}

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_analysis_export";
  }

  virtual void connect( tlm::tlm_analysis_if<T>& _if )
  {
    this->bind( _if );
  }

  virtual ~uvm_analysis_export(){}

}; // class uvm_analysis_export


/////////////////////////////////////////////

} /* namespace uvm */

#endif /* UVM_ANALYSIS_EXPORT_H_ */
