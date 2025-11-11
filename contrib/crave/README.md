# CRAVE2UVM prototype

This projects contains a prototypical integration of CRAVE with UVM-SystemC to provide access to constrained randomization and coverage.
For demonstration, we ported the SystemVerilog example CandyLovers (UVM Tutorial for CandyLovers, see http://cluelogic.com/2011/07/uvm-tutorial-for-candy-lovers-overview/) to UVM-SystemC and included it in this distribution.

## Requirements

Make sure all pre-install requirements of CRAVE and UVM-SystemC are met:

* CMake (at least v2.8.9)
* GNU Make
* g++ (at least v4.7.2)
* SystemC (the environment variable SYSTEMC_HOME must be set accordingly)
* Boost (at least v1.50.0 and the environment variable BOOST_ROOT must be set accordingly, e.g. BOOST_ROOT=/usr)
* zlib development libraries (e.g. zlib1g-dev).

The version of CRAVE included in this distribution by default will build a minimal configuration (Glog and 2 solver backends: CUDD and SWORD).
The pre-compiled SWORD library does not work on some platforms.
In this case, the user can replace SWORD with STP by setting the environment variable CRAVE_USE_STP to ON.
If download is permitted, CRAVE can automatically download and build these backends.
Otherwise, these will be unpacked from pre-packaged archives.
Other configurations with additional backends (e.g. Boolector, Z3, CVC4, etc.) are also possible. 
For more detailed instructions , please refer to the CRAVE README or contact us.

## Installation

To install and run the example, use the buildscript on the top-level of this repository. It currently executes three common tasks:

1. ./buildscript install: this will install CRAVE (shipped with this distribution) into ./crave, locally compiles UVM-SystemC into ../.. and setup the Makefile for the example "CandyLovers".

2. ./buildscript compile: this will compile the example and put the executables into build/bin.

3. ./buildscript run: this will run the CandyLovers example.

## Tested OS

This distribution has been tested on the following 64-bit Linux (x86_64) systems:

* Fedora 22 with g++-5.3.1
* Ubuntu 14.04.4 LTS with g++-4.8.4
* Debian 7.0 with g++-4.7.2

## Contact
For bug report and feedback: crave@informatik.uni-bremen.de

