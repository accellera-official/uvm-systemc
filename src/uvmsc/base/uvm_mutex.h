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
// Defines a mutex with C++11 support, falls back to SytemC sc_host_mutex for older versions
// Removes dependency from windows.h in user code

#if defined (__MINGW32__) || (__cplusplus < 201103L)
namespace sc_core {
	class sc_host_mutex;
}
#else
#include <mutex>
#include <memory>
#endif

#ifndef UVM_UVM_MUTEX_H_
#define UVM_UVM_MUTEX_H_
// forward declaration of sc_host_mutex to avoid inclusion of windows.h

namespace uvm {

class uvm_mutex {
public:
	// the classical operations: lock(), trylock(), and unlock()

	// blocks until mutex could be locked
	void lock();

	// returns -1 if mutex could not be locked
	bool trylock();

	// returns -1 if mutex was not locked by caller
	void unlock();

	uvm_mutex();
	virtual ~uvm_mutex();
private:
	// On MinGW we always have to use sc_host_mutex as most compilers do not suppurt threading
#if defined (__MINGW32__) || (__cplusplus < 201103L)
	sc_core::sc_host_mutex* m_mutex;
#else
	std::unique_ptr<std::mutex> m_mutex;
#endif
};

}
#endif /* SRC_UVMSC_BASE_UVM_MUTEX_H_ */

