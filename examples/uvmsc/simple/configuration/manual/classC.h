//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2009 Cadence Design Systems, Inc.
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

#ifndef CLASSC_H_
#define CLASSC_H_

#include <uvm>

class C : public uvm::uvm_component
{
  int v;
  int s;
  std::map<std::string, std::string> myaa;
  typedef std::map<std::string, std::string>::const_iterator myaa_itt;

public:
  C(uvm::uvm_component_name name ) : uvm::uvm_component(name) , v(0), s(0)
  {}

  void build_phase(uvm::uvm_phase& phase) override
  {
    std::string str;
    uvm_component::build_phase(phase);

    uvm::uvm_config_db<int>::get(this, "", "v", v);
    uvm::uvm_config_db<int>::get(this, "", "s", s);
    if( uvm::uvm_config_db<std::string>::get(this, "", "myaa[foo]", str) ) myaa["foo"] = str;
    if( uvm::uvm_config_db<std::string>::get(this, "", "myaa[bar]", str) ) myaa["bar"] = str;
    if( uvm::uvm_config_db<std::string>::get(this, "", "myaa[foobar]", str) ) myaa["foobar"] = str;
  }
   
  void do_print(const uvm::uvm_printer& printer) const override
  {
    printer.print_field_int("v", v, sizeof(v)*CHAR_BIT);
    printer.print_field_int("s", s, sizeof(s)*CHAR_BIT);

    printer.print_array_header("myaa", myaa.size(), "aa_string_string");

    for( myaa_itt it = myaa.begin(); it != myaa.end(); it++)
      printer.print_string("myaa["+(*it).first+"]", (*it).second);

    printer.print_array_footer();
  }

  UVM_COMPONENT_UTILS(C);
};

#endif /* CLASSC_H_ */
