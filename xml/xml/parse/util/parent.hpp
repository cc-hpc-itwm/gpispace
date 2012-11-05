// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <xml/parse/id/types.hpp>

#include <boost/optional.hpp>

#define PARENT_SIGNATURES(PARENT)                           \
  public:                                                   \
    bool has_parent() const;                                \
    boost::optional<const PARENT ## _type&> parent() const; \
    boost::optional<PARENT ## _type&> parent();             \
                                                            \
  private:                                                  \
    boost::optional<id::PARENT> _parent

#define PARENT_CONS_PARAM(PARENT)               \
  const id::PARENT& parent

#define PARENT_INITIALIZE()                     \
  _parent (parent)
