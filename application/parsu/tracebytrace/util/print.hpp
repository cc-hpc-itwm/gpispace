#ifndef _PRINT_HPP
#define _PRINT_HPP 1

#include <pnetc/type/tracebytrace_package.hpp>
#include <pnetc/type/tracebytrace_loaded_package.hpp>
#include <pnetc/type/tracebytrace_package_to_be_written.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

namespace print
{
  inline std::string package (const ::pnetc::type::tracebytrace_package::tracebytrace_package & p)
  {
    std::ostringstream os;

    os << "package ["
       << p.part
       << " (size " << p.num.trace << " x " << p.size.trace
       << " = " << (p.num.trace * p.size.trace)
       << ")"
      ;

    return os.str();
  }

  inline std::string loaded_package
  (const ::pnetc::type::tracebytrace_loaded_package::tracebytrace_loaded_package & p)
  {
    std::ostringstream os;

    os << ::print::package (p.package)
       << " in slot " << p.slot
       ;

    return os.str();
  }

  inline std::string package_to_be_written
  (const ::pnetc::type::tracebytrace_package_to_be_written::tracebytrace_package_to_be_written & p)
  {
    std::ostringstream os;

    os << ::print::loaded_package (p.loaded_package)
       << " for offset " << p.offset;

    return os.str();
  }

  inline void add_line ( std::ostringstream & os
                       , const std::string & line
                       , std::string & old
                       , unsigned int & c
                       )
  {
    if (old == line)
      {
        ++c;
      }
    else
      {
        os << line;

        if (c > 1)
          {
            os << "[" << c << "]" << std::endl;
          }

        c = 0;
        old = line;
      }
  }

  inline std::string trace ( void * pos
                           , const unsigned int & size
                           )
  {
    unsigned int l (1);
    int * a ((int *)pos);

    std::ostringstream os;

    std::string old ("");
    std::string line ("");
    unsigned int c (0);

    for (unsigned int i (0); i < size / sizeof(int); ++i, ++a, ++l)
      {
        std::ostringstream f;

        f << std::setw(9) << std::hex << *a;

        if (l % 8 == 0)
          {
            f << std::endl;

            line += f.str();

            add_line (os, line, old, c);

            line.clear();
          }
        else
          {
            line += f.str();
          }
      }

    std::ostringstream f;

    f << std::endl;

    line += f.str();

    add_line (os, line, old, c);

    return os.str();
  }
}

#endif
