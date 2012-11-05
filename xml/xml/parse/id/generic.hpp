// mirko.rahn@itwm.fhg.de

#ifndef _XML_XML_PARSE_ID_GENERIC_HPP_3ff3f82f99ee4d36a0d09bee568e892d
#define _XML_XML_PARSE_ID_GENERIC_HPP_3ff3f82f99ee4d36a0d09bee568e892d 1

#include <xml/parse/id/types.hpp>
#include <xml/parse/id/mapper.fwd.hpp>

#include <boost/optional.hpp>

#define ID_SIGNATURES(TYPE)                     \
  public:                                       \
    const id::TYPE& id() const;                 \
    id::mapper* id_mapper() const;              \
                                                \
  private:                                      \
    id::TYPE _id;                               \
    id::mapper* _id_mapper

#define ID_CONS_PARAM(TYPE)                     \
    const id::TYPE& id                          \
  , id::mapper* id_mapper

#define ID_INITIALIZE()                         \
    _id (id)                                    \
  , _id_mapper (id_mapper)


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

#endif
