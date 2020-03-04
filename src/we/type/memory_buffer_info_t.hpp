#pragma once

#include <we/expr/eval/context.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>

#include <string>

namespace we
{
  namespace type
  {
    class memory_buffer_info_t
    {
    public:
      memory_buffer_info_t();

      memory_buffer_info_t
        (std::string const& size, std::string const& alignment);

      unsigned long size (expr::eval::context const&) const;
      unsigned long alignment (expr::eval::context const&) const;

    private:
      std::string _size;
      std::string _alignment;

      friend class boost::serialization::access;

      template<typename Archive>
      void serialize (Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP (_size);
        ar & BOOST_SERIALIZATION_NVP (_alignment);
      }
    };
  }
}
