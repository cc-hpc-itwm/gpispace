#ifndef _PRINT_HPP
#define _PRINT_HPP 1

#include <pnetc/type/package.hpp>
#include <pnetc/type/loaded_package.hpp>
#include <pnetc/type/package_to_be_written.hpp>

#include <iostream>
#include <sstream>

namespace print
{
  inline std::string package (const ::pnetc::type::package::package & p)
  {
    std::ostringstream os;

    os << "package "
       << (p.left.extendable ? "<" : "|")
       << p.left.trace
       << ".."
       << p.right.trace
       << (p.right.extendable ? ">" : "|")
       << " (" << p.size << ")"
      ;

    return os.str();
  }

  inline std::string loaded_package
  (const ::pnetc::type::loaded_package::loaded_package & p)
  {
    std::ostringstream os;

    os << ::print::package (p.package)
       << " in slot " << p.slot
       ;

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
}

#endif
