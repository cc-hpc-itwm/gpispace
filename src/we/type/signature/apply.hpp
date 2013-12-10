// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_APPLY_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_APPLY_HPP

#include <we/type/signature.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      template<typename P>
        class apply_field_struct : public boost::static_visitor<>
      {
      public:
        apply_field_struct (P p)
          : _p (p)
        {}

        void operator() (std::pair<std::string, structure_type>& s) const
        {
          _p._field_struct (s);
        }
      private:
        P _p;
      };

      template<typename P>
        class apply_field : public boost::static_visitor<>
      {
      public:
        apply_field (P p)
          : _p (p)
        {}

        void operator() (std::pair<std::string, std::string>& f) const
        {
          _p._field (f);
        }
        void operator() (structured_type& s) const
        {
          boost::apply_visitor (apply_field_struct<P> (_p), s);
        }
      private:
        P _p;
      };

      template<typename P>
        class apply_struct : public boost::static_visitor<>
      {
      public:
        apply_struct (P p)
          : _p (p)
        {}
        void operator() (std::pair<std::string, structure_type>& s) const
        {
          _p._struct (s);
        }
      private:
        P _p;
      };

      template<typename P>
        void apply (P p, structured_type& structured)
      {
        boost::apply_visitor (apply_struct<P> (p), structured);
      }
      template<typename P>
        void apply (P p, field_type& field)
      {
        boost::apply_visitor (apply_field<P> (p), field);
      }
      template<typename P>
        void apply (P p, std::pair<std::string, structure_type>& s)
      {
        p._struct (s);
      }
    }
  }
}

#endif
