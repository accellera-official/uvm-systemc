//----------------------------------------------------------------------
//   Copyright 2011 Cypress Semiconductor
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2011 Cadence Design Systems, Inc.
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2014 Université Pierre et Marie Curie, Paris
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

#ifndef UVM_RESOURCE_TYPES_H_
#define UVM_RESOURCE_TYPES_H_

#include "uvmsc/conf/uvm_queue.h"

// forward declarations
//class uvm_resource_base;

namespace uvm {

//----------------------------------------------------------------------
// Class: uvm_resource_types
//
//! Provides typedefs and enums used throughout the resources facility.
//! It's used in lieu of package-scope types.  When needed, other classes
//! can use these types by prefixing their usage with uvm_resource_types::
//----------------------------------------------------------------------

class uvm_resource_types
{
 public:
  // types uses for setting overrides
  typedef int override_t; // TODO change to bit-vector?

  typedef enum { TYPE_OVERRIDE = 1, //"0b01",
                 NAME_OVERRIDE = 2, //"0b10"
                 BOTH_OVERRIDE = 3  //"0b11"
               } override_e;

   // general purpose queue of resource
  typedef uvm_queue<uvm_resource_base *> rsrc_q_t;

  // enum for setting resource search priority
  typedef enum { PRI_HIGH, PRI_LOW } priority_e;

  // access record for resources.  A set of these is stored for each
  // resource by accessing object.  It's updated for each read/write.

  struct access_t
  {
    sc_core::sc_time read_time;
    sc_core::sc_time write_time;
    unsigned int read_count;
    unsigned int write_count;

    // Default constructor to initialize read_count and write_count;
    access_t() : read_time(), write_time(), read_count(0), write_count(0) {}
  };

}; // class uvm_resource_types


//----------------------------------------------------------------------
// Class: get_t
//
//! Instances of get_t are stored in the history list as a record of each
//! get.  Failed gets are indicated with rsrc set to NULL. This is part
//! of the audit trail facility for resources.
//----------------------------------------------------------------------

class get_t
{
 public:
  std::string name;
  std::string scope;
  uvm_resource_base* rsrc;
  sc_core::sc_time t;
};

} // namespace uvm

#endif // UVM_RESOURCE_TYPES_H_
