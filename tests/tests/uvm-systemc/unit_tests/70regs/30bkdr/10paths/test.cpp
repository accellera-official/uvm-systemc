//----------------------------------------------------------------------
//   Copyright 2015 NXP B.V.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2010-2011 Cadence Design Systems, Inc.
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

class r1_typ : public uvm_reg
{
 public:
  r1_typ(std::string name = "r1_typ") : uvm_reg(name, 32, UVM_NO_COVERAGE)
  {}

  virtual void build() {}
  UVM_OBJECT_UTILS(r1_typ);
};

class b1_typ : public uvm_reg_block
{
 public:
  /* rand */ r1_typ* r1;

  b1_typ(std::string name = "b1_typ")
  : uvm_reg_block(name, UVM_NO_COVERAGE)
  {}

  virtual void build()
  {
    r1 = r1_typ::type_id::create("r1");
    r1->build();
    r1->configure(this, nullptr, "r1");
  }

  UVM_OBJECT_UTILS(b1_typ);
};

class top_blk : public uvm_reg_block
{
 public:
  /* rand */ b1_typ* b1;

  top_blk(std::string name = "top_blk")
  : uvm_reg_block(name, UVM_NO_COVERAGE)
  {}

  virtual void build()
  {
    b1 = b1_typ::type_id::create("b1");
    b1->build();
    b1->configure(this,"b1");
  }

  UVM_OBJECT_UTILS(top_blk);
};


void check_roots(std::string name,
                 std::vector<std::string>& roots,
                 std::string exp[])
{
  std::cout << "Path(s) to " << name << ":" << std::endl;

  for (unsigned int i = 0; i < roots.size(); i++)
  {
    std::cout << "   " << roots[i] << std::endl;

    if (roots[i] != exp[i])
    {
      std::ostringstream str;
      str << "Root does not match '"
          << exp[i]
          << "'.";
      UVM_ERROR("ROOTS", str.str());
    }
  }
}

template <typename T, size_t N>
size_t countof( T (&array)[N] )
{
    return N;
}


void check_paths(std::string name,
    std::vector<uvm_hdl_path_concat>& paths,
    uvm_hdl_path_concat exp[],
    int exp_size)
{
  std::cout << "Path(s) to " << name << ":" << std::endl;

  if (paths.size() != exp_size)
  {
    std::ostringstream str;
    str << paths.size()
        << " paths found instead of the expected "
        << exp_size
        << ".";
    UVM_ERROR("Test", str.str());

    for (unsigned int i = 0; i < paths.size(); i++)
      std::cout << "   " << uvm_hdl_concat2string(paths[i]) << std::endl;

    std::cout << "vs." << std::endl;

    for (int i = 0; i < exp_size; i++)
    {
      std::cout << "   " << uvm_hdl_concat2string(exp[i]) << std::endl;
    }

    return;
  }

  for (unsigned int i = 0; i < paths.size(); i++)
  {
    uvm_hdl_path_concat concat;
    uvm_hdl_path_concat exp_sl;

    concat = paths[i];
    exp_sl = exp[i];

    std::cout << "   " << uvm_hdl_concat2string(concat) << std::endl;

    for (unsigned int j = 0; j < concat.slices.size(); j++)
    {
      if (concat.slices[j].path != exp_sl.slices[j].path)
      {
        std::ostringstream str;
        str << " Path '"
            << concat.slices[j].path
            << "' does not match '"
            << exp_sl.slices[j].path
            << "'.";
        UVM_ERROR("PATHS", str.str());
      }
    }
  }
}


int sc_main(int, char*[])
{
  std::vector<uvm_hdl_path_concat> paths;
  uvm_hdl_path_slice slice;
  std::vector<std::string> roots;

  top_blk model("model");
  model.build();
  model.set_hdl_path_root("$root.dut");
  model.b1->get_full_hdl_path(roots);

  std::string t_[1];
  t_[0] = "$root.dut.b1";
  check_roots("model.b1", roots, t_);

  // Repeat the test twice to make sure the paths
  // are not modified
  for (int i = 0; i < 2; i++) // repeat(2)
  {
    paths.clear(); // delete
    model.b1->r1->get_full_hdl_path(paths);

    {
      uvm_hdl_path_concat t_;
      uvm_hdl_path_concat e[1];

      t_.add_path("$root.dut.b1.r1", -1, -1);
      e[0] = t_;

      check_paths("model.b1.r1", paths, e, 1);
    }
  }

  model.b1->add_hdl_path("b1a");

  // Repeat the test twice to make sure the paths
  // are not modified
  for (int i = 0; i < 2; i++) // repeat(2)
  {
    paths.clear(); // delete
    model.b1->r1->get_full_hdl_path(paths);

    {
      uvm_hdl_path_concat e[2];

      uvm_hdl_path_concat t0;
      t0.add_path("$root.dut.b1.r1", -1, -1);
      e[0] = t0;

      uvm_hdl_path_concat t1;
      t1.add_path("$root.dut.b1a.r1");
      e[1] = t1;

      check_paths("model.b1.r1", paths, e, 2);
    }
  }

  model.b1->r1->add_hdl_path_slice("r1a", 1, 1, true);
  model.b1->r1->add_hdl_path_slice("r1a", 0, 1);
  model.b1->r1->add_hdl_path_slice("r1b", -1, -1, true);

  paths.clear(); // delete
  model.b1->r1->get_full_hdl_path(paths);

  {
    uvm_hdl_path_concat e[6];

    uvm_hdl_path_concat t0;
    t0.add_path("$root.dut.b1.r1", -1, -1);
    e[0] = t0;

    uvm_hdl_path_concat t2;
    t2.add_path("$root.dut.b1.r1a", 1, 1);
    t2.add_path("$root.dut.b1.r1a", 0, 1);
    e[2] = t2;

    uvm_hdl_path_concat t4;
    t4.add_path("$root.dut.b1.r1b", -1, -1);
    e[4] = t4;

    uvm_hdl_path_concat t1;
    t1.add_path("$root.dut.b1a.r1", -1, -1);
    e[1] = t1;

    uvm_hdl_path_concat t3;
    t3.add_path("$root.dut.b1a.r1a", 1, 1);
    t3.add_path("$root.dut.b1a.r1a", 0, 1);
    e[3] = t3;

    uvm_hdl_path_concat t5;
    t5.add_path("$root.dut.b1a.r1b", -1, -1);
    e[5] = t5;

    check_paths("model.b1.r1", paths, e, 6);
  }

  {
    uvm_coreservice_t* cs = uvm_coreservice_t::get();
    auto svr = cs->get_report_server();

    svr->report_summarize();

    if (svr->get_severity_count(UVM_FATAL) +
        svr->get_severity_count(UVM_ERROR) == 0)
      std::cout << "** UVM TEST PASSED **" << std::endl;
    else
      std::cout <<"!! UVM TEST FAILED !!" << std::endl;
  }

  return 0;
}
