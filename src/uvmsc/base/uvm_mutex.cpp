//----------------------------------------------------------------------
//   Copyright 2018 COSEDA Technologies GmbH
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

#include "uvm_mutex.h"

#include "sysc/communication/sc_host_mutex.h"

uvm::uvm_mutex::uvm_mutex() {
#if defined (__MINGW32__) || (__cplusplus < 201103L)
	m_mutex = new sc_core::sc_host_mutex;
#else
	m_mutex = std::unique_ptr<std::mutex>(new std::mutex);
#endif
}

void uvm::uvm_mutex::lock() {
#if defined (__MINGW32__) || (__cplusplus < 201103L)
	m_mutex->lock();
#else
	m_mutex->lock();
#endif
}

bool uvm::uvm_mutex::trylock() {
#if defined (__MINGW32__) || (__cplusplus < 201103L)
	return m_mutex->trylock();
#else
	return m_mutex->try_lock();
#endif
}

void uvm::uvm_mutex::unlock() {
#if defined (__MINGW32__) || (__cplusplus < 201103L)
	m_mutex->unlock();
#else
	m_mutex->unlock();
#endif
}

uvm::uvm_mutex::~uvm_mutex() {
#if defined (__MINGW32__) || (__cplusplus < 201103L)
	delete m_mutex;
#endif
}

