//----------------------------------------------------------------------
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
//----------------------------------------------------------------------

#include <string>

#include "uvmsc/comps/uvm_agent.h"
#include "uvmsc/base/uvm_component_name.h"
#include "uvmsc/conf/uvm_config_db.h"

namespace uvm {

//----------------------------------------------------------------------
// CLASS: uvm_agent
//
// Implementation
//----------------------------------------------------------------------

uvm_agent::uvm_agent( uvm_component_name name_ ) : uvm_component( name_ )
{
  is_active = UVM_ACTIVE;
}

std::string uvm_agent::get_type_name() const
{
	return std::string(kind());
}

//----------------------------------------------------------------------
// member function: build_phase
//
//! The member function build_phase for an uvm_agent
//! shall retrieve configuration information whether it shall be
//! an active or passive agent.
//! Note that the function is non-virtual, so an application shall not
//! override this behavior
//----------------------------------------------------------------------

void uvm_agent::build_phase( uvm_phase& phase )
{
  uvm_component::build_phase(phase);

  uvm_resource_pool* rp;
  uvm_resource_types::rsrc_q_t* rq;

  uvm_object* baseobj = dynamic_cast<uvm_object*>(this);

  rp = uvm_resource_pool::get();
  rq = rp->lookup_name(get_full_name(), "is_active", nullptr, 0);

  uvm_resource_pool::sort_by_precedence(rq);

  if( rq->size() == 0 )
  {
    std::ostringstream msg;
    msg << "Active or passive mode for agent '" << this->get_full_name()
        << "' has not been defined. Default behavior is active.";
    uvm_report_warning("ACTPASS:", msg.str(), UVM_NONE);
  }

  for (int i = 0; i < rq->size(); i++)
  {
    uvm_resource_base* rsrc = rq->get(i);
    uvm_resource<uvm_active_passive_enum>* rap;

    rap = dynamic_cast<uvm_resource<uvm_active_passive_enum>* >(rsrc);

    if ( rap != nullptr )
    {
      is_active = rap->read(baseobj);
      break;
    }
    else
    {
      uvm_resource<uvm_integral_t>* rit;
      rit = dynamic_cast<uvm_resource<uvm_integral_t>* >(rsrc);
      if (rit != nullptr)
      {
        is_active = (uvm_active_passive_enum)rit->read(baseobj).to_int();
        break;
      }
      else
      {
        uvm_resource<uvm_bitstream_t>* rbs;
        rbs = dynamic_cast<uvm_resource<uvm_bitstream_t>* >(rsrc);
        if (rbs != nullptr)
        {
          is_active = (uvm_active_passive_enum)rbs->read(baseobj).to_int();
          break;
        }
        else
        {
          uvm_resource<int>* ri;
          ri = dynamic_cast<uvm_resource<int>* >(rsrc);
          if( ri != nullptr )
          {
            is_active = (uvm_active_passive_enum)ri->read(baseobj);
            break;
          }
          else
          {
            uvm_resource<unsigned int>* riu;
            riu = dynamic_cast<uvm_resource<unsigned int>* >(rsrc);
            if (riu != nullptr )
            {
              is_active = (uvm_active_passive_enum)riu->read(baseobj);
              break;
            }
            else
            {
              uvm_resource<std::string>* rs;
              rs = dynamic_cast<uvm_resource<std::string>* >(rsrc);
              if (rs != nullptr )
              {
                rs->read(baseobj);
                // TODO assign string to enum?
                // void'(uvm_enum_wrapper#(uvm_active_passive_enum)::from_name(rs.read(this), is_active));
                break;
              }
            } // else: !if(riu != nullptr )
          } // else: !if( ri != nullptr )
        } // else: !if(rbs != nullptr)
      } // else: !if(rit != nullptr)
    } // else: !if ( rap != nullptr )
  } // for
}

//----------------------------------------------------------------------
// member function: get_is_active (virtual)
//
//! Returns #UVM_ACTIVE is the agent is acting as an active agent and
//! #UVM_PASSIVE if it is acting as a passive agent. The default implementation
//! is to just return the is_active flag, but the component developer may
//! override this behavior if a more complex algorithm is needed to determine
//! the active/passive nature of the agent.
//----------------------------------------------------------------------

uvm_active_passive_enum uvm_agent::get_is_active() const
{
  return is_active;
}

//----------------------------------------------------------------------
// member function: kind()
//
//! SystemC compatible API
//----------------------------------------------------------------------

const char* uvm_agent::kind() const
{
  return "uvm::uvm_agent";
}

} /* namespace uvm */
