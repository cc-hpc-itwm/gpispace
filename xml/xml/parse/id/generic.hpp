// mirko.rahn@itwm.fhg.de

#ifndef XML_PARSE_ID_GENERIC_HPP
#define XML_PARSE_ID_GENERIC_HPP

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


#define PARENT_SIGNATURES(PARENT)                           \
  public:                                                   \
    typedef id::PARENT parent_id_type;                      \
    bool has_parent() const;                                \
    boost::optional<const PARENT ## _type&> parent() const; \
    boost::optional<PARENT ## _type&> parent();             \
    void unparent();                                        \
    void parent (const id::PARENT& parent);                 \
                                                            \
  private:                                                  \
    boost::optional<id::PARENT> _parent

#define PARENT_CONS_PARAM(PARENT)               \
  const boost::optional<id::PARENT>& parent

#define PARENT_INITIALIZE()                     \
  _parent (parent)

#endif
