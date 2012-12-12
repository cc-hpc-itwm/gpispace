#ifndef WE_TYPE_REQUIREMENT_HPP
#define WE_TYPE_REQUIREMENT_HPP 1

#include <boost/functional/hash.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>

namespace we
{
  namespace type
  {
    struct requirement_t
    {
      typedef std::string value_type;
      typedef value_type argument_type;

      requirement_t ()
        : value_()
        , mandatory_(false)
      {}

      explicit
      requirement_t (value_type arg, const bool _mandatory = false)
        : value_(arg)
        , mandatory_(_mandatory)
      {}

      requirement_t (requirement_t const &other)
        : value_(other.value_)
        , mandatory_(other.mandatory_)
      {}

      requirement_t& operator=(requirement_t const & rhs)
      {
        this->value_ = rhs.value_;
        this->mandatory_ = rhs.mandatory_;
        return *this;
      }

      ~requirement_t () {}

      bool is_mandatory (void) const
      {
        return mandatory_;
      }

      const value_type & value(void) const
      {
        return value_;
      }

      void value(const value_type & val)
      {
        value_ = val;
      }
    private:
      friend class boost::serialization::access;
      template <typename Archive>
      void serialize(Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP(value_);
        ar & BOOST_SERIALIZATION_NVP(mandatory_);
      }

      value_type value_;
      bool mandatory_;
    };

    inline requirement_t make_mandatory (const std::string& val)
    {
      return requirement_t (val, true);
    }

    inline requirement_t make_optional (const std::string& val)
    {
      return requirement_t (val, false);
    }

    inline bool operator==( const requirement_t& a
                          , const requirement_t& b
                          )
    {
      return (a.value() == b.value())
        &&   (a.is_mandatory() == b.is_mandatory())
        ;
    }

    inline std::size_t hash_value(requirement_t const & r)
    {
      boost::hash<std::string> hasher;
      std::size_t h (hasher(r.value()));
      boost::hash_combine (h, r.is_mandatory());
      return h;
    }
  }
}

#endif
