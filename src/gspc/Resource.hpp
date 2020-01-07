#pragma once

#include <gspc/Tree.hpp>

#include <util-generic/hard_integral_typedef.hpp>

#include <iosfwd>

namespace gspc
{
  namespace resource
  {
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (ID, std::uint64_t);

    std::ostream& operator<< (std::ostream&, ID const&);

    struct Type;
  }

  //! \todo Replace MaybeError by a state machine for Resources!? (and
  //! _its_ monadic bind)
  struct Resource
  {
    resource::Type const& type() const;
  };

  using Resources = Forest<Resource>;
}
