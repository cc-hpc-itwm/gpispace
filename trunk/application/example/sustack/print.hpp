#ifndef _PRINT_HPP
#define _PRINT_HPP 1

#include <pnetc/type/package.hpp>
#include <pnetc/type/loaded_package.hpp>
#include <pnetc/type/package_to_be_written.hpp>

#include <iostream>

namespace print
{
  inline void package ( std::ostream & os
                      , const ::pnetc::type::package::package & p
                      )
  {
    os << "package "
       << (p.left.extendable ? "<" : "|")
       << p.left.trace
       << ".."
       << p.right.trace
       << (p.right.extendable ? ">" : "|")
       << " (" << p.size << ")"
      ;
  }

  inline void loaded_package
  ( std::ostream & os
  , const ::pnetc::type::loaded_package::loaded_package & p
  )
  {
    ::print::package (os, p.package);

    os << " in slot " << p.slot;
  }

  inline void package_to_be_written
  ( std::ostream & os
  , const ::pnetc::type::package_to_be_written::package_to_be_written & p
  )
  {
    ::print::loaded_package (os, p.loaded_package);

    std::cerr << " for offset " << p.offset;
  }
}

#endif
