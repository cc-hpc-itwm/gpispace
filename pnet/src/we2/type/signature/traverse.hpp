// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_TRAVERSE_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_TRAVERSE_HPP

#include <we2/type/signature.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      template<typename P>
        class traverse_field_struct : public boost::static_visitor<>
      {
      public:
        traverse_field_struct (P p)
          : _p (p)
        {}

        void operator() (const std::pair<std::string, structure_type>& s) const
        {
          _p._field_struct (s);
        }
      private:
        P _p;
      };

      template<typename P>
        class traverse_field : public boost::static_visitor<>
      {
      public:
        traverse_field (P p)
          : _p (p)
        {}

        void operator() (const std::pair<std::string, std::string>& f) const
        {
          _p._field (f);
        }
        void operator() (const structured_type& s) const
        {
          boost::apply_visitor (traverse_field_struct<P> (_p), s);
        }
      private:
        P _p;
      };

      template<typename P>
        class traverse_struct : public boost::static_visitor<>
      {
      public:
        traverse_struct (P p)
          : _p (p)
        {}
        void operator() (const std::pair<std::string, structure_type>& s) const
        {
          _p._struct (s);
        }
      private:
        P _p;
      };

      template<typename P>
        void traverse (P p, const structured_type& structured)
      {
        boost::apply_visitor (traverse_struct<P> (p), structured);
      }
      template<typename P>
        void traverse (P p, const field_type& field)
      {
        boost::apply_visitor (traverse_field<P> (p), field);
      }
      template<typename P>
        void traverse (P p, const std::pair<std::string, structure_type>& s)
      {
        p._struct (s);
      }
    }
  }
}

#endif
