//------------------------------------------------------------------------------
//   Copyright 2010 Mentor Graphics Corporation
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
//------------------------------------------------------------------------------
//
// Top level sequence which runs four sequences with different priority and time
// offsets. It also sets up the sequencer arbitration algorithm

#ifndef ARB_EXAMPLE_SEQ_H_
#define ARB_EXAMPLE_SEQ_H_

#include <uvm>
#include <systemc>
#include "sysc/kernel/sc_dynamic_processes.h"

#include "seq_arb_item.h"
#include "arb_seq.h"
#include "lock_seq.h"
#include "grab_seq.h"

class arb_example_seq : public uvm::uvm_sequence<seq_arb_item>
{
 public:
  UVM_OBJECT_UTILS(arb_example_seq);

  arb_seq* seq_1;
  arb_seq* seq_2;
  arb_seq* seq_3;
  arb_seq* seq_4;
  grab_seq* seq_5g;
  lock_seq* seq_5l;

  uvm::SEQ_ARB_TYPE arb_type;

  arb_example_seq( const std::string name = "arb_example_seq" )
    : uvm::uvm_sequence<seq_arb_item>(name)
  {}

  void start_thread1()
  {
    for(int i=0;i<10;i++) // repeat(10)
    {
      sc_core::wait(1, sc_core::SC_MS); // Offset by 1
      seq_1->start(m_sequencer, this, 500); // Highest priority
    }
  }

  void start_thread2()
  {
    for(int i=0;i<10;i++) // repeat(10)
    {
      sc_core::wait(2, sc_core::SC_MS); // Offset by 2
      seq_2->start(m_sequencer, this, 500); // Highest priority
    }
  }

  void start_thread3()
  {
    for(int i=0;i<10;i++) // repeat(10)
    {
      sc_core::wait(3, sc_core::SC_MS); // Offset by 3
      seq_3->start(m_sequencer, this, 300); // Medium priority
    }
  }

  void start_thread4()
  {
    SC_FORK
      sc_core::sc_spawn(sc_bind(&arb_example_seq::start_thread4a, this)),
      sc_core::sc_spawn(sc_bind(&arb_example_seq::start_thread4b, this))
    SC_JOIN

    sc_core::wait(4, sc_core::SC_MS);
    seq_4->start(m_sequencer, this, 200);

    SC_FORK
      sc_core::sc_spawn(sc_bind(&arb_example_seq::start_thread4c, this)),
      sc_core::sc_spawn(sc_bind(&arb_example_seq::start_thread4d, this))
    SC_JOIN
  }

  void start_thread4a()
  {
    for(int i=0;i<2;i++) // repeat(2)
    {
      sc_core::wait(4, sc_core::SC_MS); // Offset by 4
      seq_4->start(m_sequencer, this, 200); // Lowest priority
    }
  }

  void start_thread4b()
  {
    sc_core::wait(10, sc_core::SC_MS);
    seq_5g->start(m_sequencer, this, 50);
  }

  void start_thread4c()
  {
    seq_5l->start(m_sequencer, this, 200);
  }

  void start_thread4d()
  {
    sc_core::wait(20, sc_core::SC_MS);
    seq_5g->start(m_sequencer, this, 50);
  }

  void body() override
  {
    seq_1 = arb_seq::type_id::create("seq_1");
    seq_1->seq_no = 1;
    seq_2 = arb_seq::type_id::create("seq_2");
    seq_2->seq_no = 2;
    seq_3 = arb_seq::type_id::create("seq_3");
    seq_3->seq_no = 3;
    seq_4 = arb_seq::type_id::create("seq_4");
    seq_4->seq_no = 4;
    seq_5g = grab_seq::type_id::create("seq_5g");
    seq_5l = lock_seq::type_id::create("seq_5l");


    m_sequencer->set_arbitration(arb_type);

    // start sequences in parallel
    SC_FORK
      sc_core::sc_spawn(sc_bind(&arb_example_seq::start_thread1, this)),
      sc_core::sc_spawn(sc_bind(&arb_example_seq::start_thread2, this)),
      sc_core::sc_spawn(sc_bind(&arb_example_seq::start_thread3, this)),
      sc_core::sc_spawn(sc_bind(&arb_example_seq::start_thread4, this))
    SC_JOIN

    arb_seq::type_id::destroy(seq_1);
    arb_seq::type_id::destroy(seq_2);
    arb_seq::type_id::destroy(seq_3);
    arb_seq::type_id::destroy(seq_4);
    grab_seq::type_id::destroy(seq_5g);
    lock_seq::type_id::destroy(seq_5l);
  } // body

}; // class arb_example_seq

#endif // ARB_EXAMPLE_SEQ_H_
