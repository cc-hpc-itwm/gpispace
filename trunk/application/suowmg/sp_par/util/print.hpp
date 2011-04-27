#ifndef _PRINT_HPP
#define _PRINT_HPP 1

#include <pnetc/type/package.hpp>
#include <pnetc/type/loaded_package.hpp>
#include <pnetc/type/package_to_be_written.hpp>
#include <pnetc/type/output.hpp>
#include <pnetc/type/shot.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "util.hpp"

namespace print
{
  inline std::string output (const ::pnetc::type::output::output & o)
  {
    std::ostringstream os;

    os << "output [" << o.trace.left << ".." << o.trace.right << ")"
       << " num " << o.shot_num
       << " in slot " << o.slot
      ;

    return os.str();
  }

  inline std::string package (const ::pnetc::type::package::package & p)
  {
    std::ostringstream os;

    os << "package "
       << (p.left.extendable ? "<" : "|")
       << p.left.trace
       << ".."
       << p.right.trace
       << (p.right.extendable ? ">" : "|")
       << " (" << ::util::size (p) << ")"
      ;

    return os.str();
  }

  inline std::string interval
  (const ::pnetc::type::interval::interval & i)
  {
    std::ostringstream os;

    os << i.offset << ":" << i.size;

    return os.str();
  }

  inline std::string assigned_package
  (const ::pnetc::type::assigned_package::assigned_package & p)
  {
    std::ostringstream os;

    os << ::print::package (p.package)
       << " intervals ["
      ;

    for (::util::interval_iterator i (p); i.has_more(); ++i)
      {
        os << " " << ::print::interval (*i);
      }

    os << "]";

    return os.str();
  }

  inline std::string key
  (const ::pnetc::type::key::key & ks)
  {
    std::ostringstream os;

    os << "(sx " << ks.sx << ", sy " << ks.sy << ")";

    return os.str();
  }

  inline std::string loaded_package
  (const ::pnetc::type::loaded_package::loaded_package & p)
  {
    std::ostringstream os;

    os << ::print::assigned_package (p.assigned_package)
       << " " << ::print::key (p.key)
       ;

    return os.str();
  }

  inline std::string shot
  (const ::pnetc::type::shot::shot & s)
  {
    std::ostringstream os;

    os << "shot #" << s.num
       << " " << ::print::loaded_package (s.loaded_package);

    return os.str();
  }

  inline std::string package_to_be_written
  (const ::pnetc::type::package_to_be_written::package_to_be_written & p)
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
