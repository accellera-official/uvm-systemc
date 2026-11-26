//----------------------------------------------------------------------
//   Copyright 2011 Cypress Semiconductor
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2011 Cadence Design Systems, Inc.
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

#ifndef UVM_RESOURCE_OPTIONS_H_
#define UVM_RESOURCE_OPTIONS_H_

namespace uvm {

//----------------------------------------------------------------------
// Class: uvm_resource_options
//
//! Provides a namespace for managing options for the
//! resources facility. The only thing allowed in this class is static
//! local data members and static functions for manipulating and
//! retrieving the value of the data members.  The static local data
//! members represent options and settings that control the behavior of
//! the resources facility.
//----------------------------------------------------------------------

class uvm_resource_options
{
 public:
  static void turn_on_auditing();
  static void turn_off_auditing();
 static bool is_auditing();

 private:
  // disabled
  uvm_resource_options();
};

} // namespace uvm


#endif // UVM_RESOURCE_OPTIONS_H_
