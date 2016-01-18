// mirko.rahn@itwm.fhg.de

#pragma once

#include <xml/parse/id/types.hpp>
#include <xml/parse/id/mapper.fwd.hpp>

#include <boost/optional.hpp>

#define ID_SIGNATURES(TYPE)                     \
  public:                                       \
    typedef id::TYPE id_type;                   \
    const id::TYPE& id() const;                 \
    id::ref::TYPE make_reference_id() const;    \
                                                \
  private:                                      \
    id::mapper* id_mapper() const;              \
    id::TYPE _id;                               \
    id::mapper* _id_mapper

#define ID_CONS_PARAM(TYPE)                     \
    const id::TYPE& id                          \
  , id::mapper* id_mapper

#define ID_INITIALIZE()                         \
    _id (id)                                    \
  , _id_mapper (id_mapper)
