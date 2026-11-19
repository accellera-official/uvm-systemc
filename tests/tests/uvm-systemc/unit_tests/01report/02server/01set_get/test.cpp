//---------------------------------------------------------------------- 
//   Copyright 2017 NXP B.V.
//   Copyright 2010 Cadence Design Systems.
//   Copyright 2010 Mentor Graphics Corporation
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
#include <uvm>

using namespace uvm;

// global variable to track status
int cnt;
bool success;


class my_server : public uvm_default_report_server
{
public:

  my_server( uvm_object_name name ) : uvm_default_report_server(name)
  {}

  std::string compose_report_message( uvm_report_message* report_message, const std::string& report_object_name = "" ) const override
  {
    cnt++;
    return "MY_SERVER: " + uvm_default_report_server::compose_report_message(report_message, report_object_name);
  }

  // to make sure this is being executed, have it display a good result
  // (despite the dummy error thrown earlier)
  void report_summarize( UVM_FILE file = 0 ) const override
  {
    if(success)
    {
      std::cout << "**** UVM TEST PASSED ****" << std::endl;
      std::cout << "--- UVM Report Summary ---" << std::endl << std::endl;
      std::cout << "** Report counts by severity" << std::endl;
      std::cout << "UVM_INFO :     " << get_severity_count(UVM_INFO) << std::endl;
      std::cout << "UVM_WARNING :  " << get_severity_count(UVM_WARNING) << std::endl;
      std::cout << "UVM_ERROR :    " << get_severity_count(UVM_ERROR) << std::endl;
      std::cout << "UVM_FATAL :    " << get_severity_count(UVM_FATAL) << std::endl;
    }
    else
    {
      std::cout << "**** UVM TEST FAILED ****" << std::endl;
      uvm_default_report_server::report_summarize(file) ;
    }
  }
};

class test : public uvm_test
{
public:

  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name ) : uvm_test(name)
  {}

  void run_phase( uvm_phase& phase ) override
  {
    my_server* serv = new my_server("my_server");

    // Emit a message before setting the server to make sure counts are
    // properly copied over.
    UVM_INFO("MSG1", "Some message", UVM_LOW);
    UVM_INFO("MSG2", "Another message", UVM_LOW);

    // Use the user-defined server
    uvm_report_server::set_server(serv);

    //Emit some messages to the new server
    UVM_INFO("MSG1", "Some message again", UVM_LOW);
    UVM_INFO("MSG2", "Another message again", UVM_LOW);
  }

  void report_phase( uvm_phase& phase ) override
  {
    uvm_report_server* serv = uvm_report_server::get_server();

    if( serv->get_id_count("MSG1") == 2 &&
        serv->get_id_count("MSG2") == 2 && cnt == 2)
      success = true;

    UVM_ERROR("EXPECTED", "This is an expected error designed to test whether report_server that was set is used at end of sim.");
  }
};

int sc_main(int, char*[])
{
  cnt = 0;
  success = false;

  run_test("test");

  return 0;
}
