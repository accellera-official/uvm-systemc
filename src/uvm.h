//----------------------------------------------------------------------
//   Copyright 2014 NXP B.V.
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

#ifndef UVM_H_
#define UVM_H_

#include "uvmsc/macros/uvm_defines.h"

#if !UVM_DEPRECATED_API_ENABLED
#error "'#include <uvm.h>' is deprecated in favour of '#include <uvm>' and has been removed by default. Define ALLOW_UVM_SC_DEPRECATED (or ALLOW_UVM_SC_DEPRECATED_SILENT) to temporarily enable deprecated API."
#elif !defined(ALLOW_UVM_SC_DEPRECATED_SILENT)
#warning '#include <uvm.h>' is deprecated in favour of '#include <uvm>' and will be removed in a future release.
#endif

// include some standard namespace (std) definitions

#include <cassert>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#if defined(__sun) || defined(__sun__)
#   include <inttypes.h>
#elif !defined(WIN32) && !defined(_WIN32)
#   include <stdint.h>
#endif

#if defined(__EDG__)
#   include <strstream>
#endif // defined(__EDG__)

#include <typeinfo>
#include <utility>
#include <vector>

// I/O steams

#if !(defined( _MSC_VER ) && _MSC_VER < 1300) && !defined( __BORLANDC__ )

#if defined(__GNUC__) && (__GNUC__ == 2) // Maintain compatibility for 2.95.x
   using std::strstream;
   using std::strstreambuf;
   using std::istrstream;
   using std::ostrstream;
#endif

   using std::ios;
   using std::streambuf;
   using std::streampos;
   using std::streamsize;
   using std::iostream;
   using std::istream;
   using std::ostream;
   using std::cin;
   using std::cout;
   using std::cerr;
   using std::endl;
   using std::flush;
   using std::dec;
   using std::hex;
   using std::oct;

#if defined(linux)
   using std::noshowbase;
   using std::showbase;
#endif // defined(linux)

   using std::fstream;
   using std::ifstream;
   using std::ofstream;

//  from <cstdio>:

#if !defined( _MSC_VER )
   using std::size_t;
   using std::FILE;
   using std::fpos_t;
   using std::fclose;
   using std::clearerr;
#endif // defined(_MSC_VER)

   using std::remove;
   using std::rename;
   using std::tmpfile;
   using std::tmpnam;
   using std::fflush;
   using std::fopen;
   using std::freopen;
   using std::setbuf;
   using std::setvbuf;
   using std::fprintf;
   using std::fscanf;
   using std::printf;
   using std::scanf;
   using std::sprintf;
   using std::sscanf;
   using std::vfprintf;
   using std::vprintf;
   using std::vsprintf;
   using std::fgetc;
   using std::fgets;
   using std::fputc;
   using std::fputs;
   using std::getc;
   using std::getchar;
   using std::putc;
   using std::putchar;
   using std::puts;
   using std::ungetc;
   using std::fread;
   using std::fwrite;
   using std::fgetpos;
   using std::fseek;
   using std::fsetpos;
   using std::ftell;
   using std::rewind;
   using std::feof;
   using std::ferror;
   using std::perror;

//  from <cstdlib>:

   using std::div_t;
   using std::ldiv_t;

   using std::atof;
   using std::atoi;
   using std::atol;
   using std::strtod;
   using std::strtol;
   using std::strtoul;
   using std::rand;
   using std::srand;
   using std::calloc;
   using std::free;
   using std::malloc;
   using std::realloc;
   using std::abort;
   using std::atexit;
   using std::exit;
   using std::getenv;
   using std::system;
   using std::bsearch;
   using std::qsort;
   using std::abs;
   using std::div;
   using std::labs;
   using std::ldiv;
   using std::mblen;
   using std::mbtowc;
   using std::mbstowcs;

#if !defined(__CYGWIN__) && !defined(__CYGWIN32)
   using std::wctomb;
   using std::wcstombs;
#endif // !defined(__CYGWIN__) && !defined(__CYGWIN32)

// from <cstring>:

   using std::memcpy;
   using std::memmove;
   using std::strcpy;
   using std::strncpy;
   using std::strcat;
   using std::strncat;
   using std::memcmp;
   using std::strcmp;
   using std::strcoll;
   using std::strncmp;
   using std::strxfrm;
   using std::memchr;
   using std::strchr;
   using std::strcspn;
   using std::strpbrk;
   using std::strrchr;
   using std::strspn;
   using std::strstr;
   using std::strtok;
   using std::memset;
   using std::strerror;
   using std::strlen;

#else

#if defined( _MSC_VER )
    // VC++6 with standard library

   using std::ios;
   using std::streambuf;
   using std::streampos;
   using std::streamsize;
   using std::iostream;
   using std::istream;
   using std::ostream;
   using std::strstream;
   using std::strstreambuf;
   using std::istrstream;
   using std::ostrstream;
   using std::cin;
   using std::cout;
   using std::cerr;
   using std::endl;
   using std::flush;
   using std::dec;
   using std::hex;
   using std::oct;

   using std::fstream;
   using std::ifstream;
   using std::ofstream;

#endif // defined( _MSC_VER )

#endif // !(defined( _MSC_VER ) && _MSC_VER < 1300) && !defined( __BORLANDC__ )

// include UVM definitions

#include "uvm"

// include elements from uvm namespace
using namespace uvm;

// TODO should we explicitly add individual elements only?

#endif /* UVM_H_ */
