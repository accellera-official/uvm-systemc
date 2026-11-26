//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
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

#ifndef UVM_ANALYSIS_PORT_H_
#define UVM_ANALYSIS_PORT_H_

#include <systemc>
#include <tlm>
#include <string>

namespace uvm {

//----------------------------------------------------------------------
// CLASS: uvm_analysis_port
//
//! This class is an alias for the tlm_analysis_port
//----------------------------------------------------------------------

template <typename T>
class uvm_analysis_port : public tlm::tlm_analysis_port<T> // TODO or should be derived from e.g. uvm_port_base<tlm_analysis_if<T> > ?
{
 public:
  uvm_analysis_port() : tlm::tlm_analysis_port<T>() {}
  uvm_analysis_port( const std::string& name ) : tlm::tlm_analysis_port<T>( name.c_str() ) {}

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_analysis_port";
  }

  virtual void connect( tlm::tlm_analysis_if<T>& _if )
  {
    this->bind( _if );
  }

  // the write method comes via tlm_analysis_port

  virtual ~uvm_analysis_port(){}

}; // class uvm_analysis_port


/////////////////////////////////////////////

} /* namespace uvm */

#endif /* UVM_ANALYSIS_PORT_H_ */
