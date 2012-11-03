// mirko.rahn@itwm.fhg.de

#pragma once

#include <xml/parse/id/types.hpp>
#include <xml/parse/id/mapper.fwd.hpp>

#define ID_SIGNATURES(TYPE)                     \
  public:                                       \
    const id::TYPE& id() const;                 \
    id::mapper* id_mapper() const;              \
                                                \
  private:                                      \
    id::TYPE _id;                               \
    id::mapper* _id_mapper;

#define ID_CONS_PARAM(TYPE)                     \
    const id::TYPE& id                          \
  , id::mapper* id_mapper

#define ID_INITIALIZE()                         \
    _id (id)                                    \
  , _id_mapper (id_mapper)
